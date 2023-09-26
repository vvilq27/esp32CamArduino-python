#include "esp_camera.h"
#include <SPI.h>
#include <WiFi.h>
//#include "driver/adc.h"
#include "soc/rtc_cntl_reg.h"

#define DATA_BYTES 30
#define ROW_LENGTH 200
#define CAMERA_MODEL_AI_THINKER // Has PSRAM

//#include "ESP32Camera.h"
#include "camera_pins.h"

const char* ssid = "ESP";
const char* password = "1234";

String serverName = "192.168.4.1";   // REPLACE WITH YOUR Raspberry Pi IP ADDRESS
String serverPath = "/upload";     // The default serverPath should be upload.php
const int serverPort = 80;

WiFiClient client;

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
//ESP32Camera camera;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
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
  config.pixel_format = PIXFORMAT_JPEG; //PIXFORMAT_JPEG   ,   PIXFORMAT_GRAYSCALE

  config.frame_size = FRAMESIZE_QQVGA;
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

  imgIdx =0;
  rowValidFlag = false;

  Serial.println("start WiFi setup");
  connectWifiServer();
  Serial.println("WiFi setup complete");
  Serial.println("");

} 


void loop() {
//  long start = millis();

//  rowId = 0;

  serverPhotoUpload();
  
//  sendImg(imgSize);

//  resendDataUntilImageValid();
  
//  imgIdx++;
  delay(1000);

}// end main loop

void serverPhotoUpload(){
  fb = esp_camera_fb_get();

  Serial.println("Connecting to server: " + serverName);
  

  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");    
    String head = "--espCamServerUpload\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"espPic.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--espCamServerUpload--\r\n";

    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;
  
    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=espCamServerUpload");
    client.println();
    client.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0; n<fbLen; n=n+1024) {
      if (n+1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        client.write(fbBuf, remainder);
      }
    }   
    client.print(tail);
    
    esp_camera_fb_return(fb);
    client.stop();
  } else {
    Serial.println("Connection to " + serverName +  " failed.");
  }
}

void sendImg(uint32_t imgSize){
    while(imgSize){
      printImageIndexes();

      if(imgSize%2000==0)
        delay(30);
    
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
//        rowValidFlag = true;// limits changes to once per pic
      } else {
        data = (char *)fb->buf;
        uint16_t intRowNumber = command.toInt();
        data += intRowNumber * ROW_LENGTH;
        
        for(uint8_t i = 0; i <ROW_LENGTH; i++){
          Serial.write(*data++);
        }

        Serial.println();
      }
    }//while serial in buff
    
  }// while flag
  Serial.println("Img accepted");
  
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
    esp_camera_deinit();
//    camera.begin(FRAMESIZE_QVGA, PIXFORMAT_JPEG);

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
    config.pixel_format = PIXFORMAT_JPEG; //
  
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 30;
    config.fb_count = 1;
  
    // camera init
    esp_err_t err = esp_camera_init(&config);
    
    Serial.println("Started camera in QQVGA JPEG format");
  } else if(command ==  "format_grayscale"){
    esp_camera_deinit();
    
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
  
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 30;
    config.fb_count = 1;
  
    // camera init
    esp_err_t err = esp_camera_init(&config);
    
    Serial.println("Started camera in QQVGA GRAY format");
  }

  else if(command == "bright+"){
    if(brightness < 2)
      s->set_brightness(s, ++brightness);
  }
  
  else{
      Serial.println("Command unknown");
   }
}

void connectWifiServer(){
   WiFi.mode(WIFI_STA);
    Serial.println();
    Serial.print("Connecting to WIFI: ");
    Serial.println(ssid);
    WiFi.begin(ssid);  
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(1100);
      ESP.restart();
  }
  Serial.println("\n connection success");
}

//void disableWiFi(){
//    adc_power_off();
//    WiFi.disconnect(true);  // Disconnect from the network
//    WiFi.mode(WIFI_OFF);    // Switch WiFi off
//}
