#include "esp_camera.h"
#include <SPI.h>
#include <nRF24L01.h>       //Download here: https://electronoobs.com/eng_arduino_NRF24_lib.php
#include <RF24.h>

#include <WiFi.h>
#include <esp_wifi.h>
#include "driver/adc.h"

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#define DATA_BYTES 30

#include "camera_pins.h"

/*    pinout for SPI nrf24 connection
 *     io2 - CSN
 *     io14 - SCK
 *     io15 - CE
 *     io13 - MOSI
 *     io12 - MISO
 */
 
void startCameraServer();

SPIClass SPI2(HSPI);
RF24 radio(15,2);
const uint64_t address = 0xE8E8F0F0E1LL;

uint8_t dataPacket[32];
char *data;
uint32_t imgSize;
uint8_t packetCount;
uint8_t imgIdx;
camera_fb_t *fb;

void setup() {
  disableWiFi();
  Serial.begin(1000000);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif


  if (!radio.begin(&SPI2, 15,2)) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {} // hold in infinite loop
  }
  
  radio.setAutoAck(true);
  radio.setChannel(100);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(address);

  fb = esp_camera_fb_get();

  data = (char *)fb->buf;
  imgSize = fb->len;

  for(int i =0 ; i <imgSize; i++){
    if(*data<16)
      Serial.print(0);
      
    Serial.print(*data++, HEX);

    if(i%DATA_BYTES == 0)
      Serial.println();
  }

  Serial.println();
  Serial.println(imgSize);

  esp_camera_fb_return(fb);

  imgIdx =0;
} 

void loop() {
  long start = millis();
  isRfStuck();
  fb = esp_camera_fb_get();

  data = (char *)fb->buf;
  imgSize = fb->len;
  packetCount = 0;
  
  sendPacketWithImgSize(imgSize);
  
  // go thru data[] and pick every DATA_BYTES bytes into packet
  for(int packetCnt = 0; packetCnt < imgSize/DATA_BYTES + 1; packetCnt++){
    
    //case for last packet, to not exceed data[] index
    if(packetCnt == imgSize/DATA_BYTES){
      int remainingBytes = imgSize - packetCnt*DATA_BYTES;

      zeroDataPacket();

      //2 is offset from img index and packet index
      for(int j = 2; j < 32; j++){
        if(j <= remainingBytes)
          dataPacket[j] = *data++;
        else
          dataPacket[j] = 0;
      }
    } 

    //not last packet, populate packet with all DATA_BYTES bytes
    else {
      for(int j = 2; j <32; j++){          
        dataPacket[j] = *data++;
      }
    }
   
   dataPacket[1] = packetCnt;
   dataPacket[0] = imgIdx;

//print packet
//   for( int i =0; i<32;i++){        
//      Serial.write(dataPacket[i]);
//   }

   radio.write(&dataPacket, sizeof(dataPacket));
//   delay(1);
  //var just to print it after for loop finished
   packetCount = packetCnt;
//   long rfDelay = millis() - start;
//   
//   if(isRfStuck(rfDelay))
//      break;
  }//end for loop, all packets sent

  imgIdx++;

  Serial.print("total packets: ");
  Serial.println(packetCount);

  delay(1000);

  esp_camera_fb_return(fb);

  uint16_t radioDelay = millis() - start;
  
  Serial.print("pic in ");
  Serial.println(radioDelay);
}// end main loop

boolean isRfStuck(){
    if (!radio.begin(&SPI2, 15,2)) {
      Serial.println(F("radio hardware is not responding!!"));
      while (1) {} // hold in infinite loop
    }

    radio.setAutoAck(true);
    radio.setChannel(100);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_LOW);
    radio.flush_tx();
    radio.openWritingPipe(address);

    return true;    
}

void zeroDataPacket(){
  for(int j = 0; j <32; j++)
        dataPacket[j] = 0;
}

void sendPacketWithImgSize(int imgSize){
  int size = imgSize;
  int sizeTable[4];

  zeroDataPacket();

  uint8_t idx = 0;

  while (size > 0) {
    sizeTable[idx] = size%0x100;
    size = size >> 8;

    if(size>0)
      idx++;
  }
  dataPacket[0] = 's';
  dataPacket[1] = 'i';
  dataPacket[2] = 'z';
  dataPacket[3] = 'e';
  dataPacket[4] = ':';

  for(uint8_t i = 5; i <= idx; i++){
    dataPacket[i] = sizeTable[idx-i];
  }

  radio.write(&dataPacket, sizeof(dataPacket));
}

void disableWiFi(){
    adc_power_off();
    WiFi.disconnect(true);  // Disconnect from the network
    WiFi.mode(WIFI_OFF);    // Switch WiFi off
}
