#include "esp_camera.h"
#include <SPI.h>
//#include <WiFi.h>
#include "driver/adc.h"

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#define DATA_BYTES 30
#define ROW_LENGTH 200

#include "camera_pins.h"

/*    pinout for SPI nrf24 connection
 *     io2 - CSN
 *     io14 - SCK
 *     io15 - CE
 *     io13 - MOSI
 *     io12 - MISO
 */

char *data;
uint32_t imgSize;
uint8_t packetCount;
uint8_t imgIdx;
uint16_t rowId ;
camera_fb_t *fb;
bool rowValidFlag;
sensor_t * s;
camera_config_t config;
uint8_t brightness;

void setup() {
//  disableWiFi();
  Serial.setTimeout(10);
  Serial.begin(1000000);
  Serial.setDebugOutput(true);
  Serial.println();

  
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
  config.pixel_format = PIXFORMAT_GRAYSCALE; //PIXFORMAT_JPEG

  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 30;
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  s = esp_camera_sensor_get();
  brightness = 0;
  
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 2); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  
  // drop down frame size for higher initial frame rate
//  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
  s->set_brightness(s, 0);
#endif

  
//  fb = esp_camera_fb_get();
//
//  data = (char *)fb->buf;
//  imgSize = fb->len;
//
//  for(int i =0 ; i <imgSize; i++){
//    if(*data<16)
//      Serial.print(0);
//      
//    Serial.print(*data++, HEX);
//
//    if(i%DATA_BYTES == 0)
//      Serial.println();
//  }
//
//  Serial.println("init image size: ");
//  Serial.println(imgSize);
//
//  esp_camera_fb_return(fb);

  imgIdx =0;
  rowValidFlag = false;
} 


void loop() {
  long start = millis();

  fb = esp_camera_fb_get();

  data = (char *)fb->buf;
  imgSize = fb->len;

  Serial.print("newimg:");
  Serial.println(imgSize);

  rowId = 0;
  
  sendImg(imgSize);

  resendDataUntilImageValid();
  
  imgIdx++;
  Serial.print("Image in: ");
  Serial.println(millis() - start);

  esp_camera_fb_return(fb);
}// end main loop

void sendImg(uint32_t imgSize){
    while(imgSize){
      printImageIndexes();
    
      for(uint8_t i = 0; i <ROW_LENGTH; i++){
        char pixel = *data++;
        if(pixel == 10 || pixel == 13)
          pixel = 11;
        Serial.write(pixel);
        imgSize--;
      }
      
      Serial.println();
  }
}

void printImageIndexes(){
  //remove imgIdx
  if(imgIdx < 10)
      Serial.print("00");
    if(imgIdx > 9 && imgIdx < 100)
      Serial.print("0");
    Serial.print(imgIdx);
    Serial.print(',');

    if(rowId < 10)
      Serial.print("000");
    else if (rowId > 9 && rowId < 100)
      Serial.print("00");
    else if (rowId > 99 && rowId < 1000)
      Serial.print("0");
    Serial.print(rowId++);
    
    Serial.print("|");
}

void resendDataUntilImageValid(){
  while(!rowValidFlag){
    while (Serial.available() > 0) {
      String command = Serial.readStringUntil('\n');
      command.trim();

      if(command == "ok"){
        rowValidFlag = true;
      } else if ( command.length() > 5){
        configureCam(command);
      } else {
        data = (char *)fb->buf;
        uint16_t intRowNumber = command.toInt();
        data += intRowNumber * ROW_LENGTH;
        
        for(uint8_t i = 0; i <ROW_LENGTH; i++){
          Serial.write(*data++);
        }

        Serial.println();
      }
    }
    
  }
  
  rowValidFlag = false;
}

void configureCam(String command){
  if(command == "res_qqvga"){
      s->set_framesize(s, FRAMESIZE_QQVGA);
      Serial.println("changing resolution to QQVGA[120x160]");
  } else  if(command == "res_qvga"){
      esp_camera_fb_return(fb);
      s->set_framesize(s, FRAMESIZE_QVGA);
      

      Serial.println("changing resolution to QVGA[240x320]");
  } else if(command == "res_hvga"){
      config.frame_size = FRAMESIZE_HVGA;
      esp_err_t err = esp_camera_init(&config);
  } else if(command == "res_vga"){
      s->set_framesize(s, FRAMESIZE_VGA);
  }

  else if(command == "format_jpeg"){
     s->set_pixformat(s, PIXFORMAT_JPEG);
  } else if(command ==  "format_grayscale"){
      s->set_pixformat(s, PIXFORMAT_GRAYSCALE);
  }

  else if(command == "bright+"){
    if(brightness < 2)
      s->set_brightness(s, ++brightness);
  }


      
   else{
      Serial.println("Command unknown");
   }
}

//void disableWiFi(){
//    adc_power_off();
//    WiFi.disconnect(true);  // Disconnect from the network
//    WiFi.mode(WIFI_OFF);    // Switch WiFi off
//}
