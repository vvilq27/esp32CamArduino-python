#include "esp_camera.h"
#include <SPI.h>
#include "driver/adc.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include <EEPROM.h>            // read and write from flash memory
#include "driver/rtc_io.h"

#define DATA_BYTES 30
#define ROW_LENGTH 200
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#define EEPROM_SIZE 1

#define IMAGE_HEIGHT (unsigned int)120
#define IMAGE_WIDTH (unsigned int)160
#define IMAGE_MODE CAMERA_GRAYSCALE
#define BITS_PER_PIXEL (unsigned int)8
#define PALETTE_COLORS_AMOUNT (unsigned int) 256 
#define PALETTE_SIZE  (unsigned int)(PALETTE_COLORS_AMOUNT * 4) // 4 bytes = 32bit per color (3 bytes RGB and 1 byte 0x00)
#define IMAGE_PATH "/image.bmp"

// Headers info
#define BITMAP_FILE_HEADER_SIZE (unsigned int)14 // For storing general information about the bitmap image file
#define DIB_HEADER_SIZE (unsigned int)40 // For storing information about the image and define the pixel format
#define HEADER_SIZE (BITMAP_FILE_HEADER_SIZE + DIB_HEADER_SIZE)

//#include "ESP32Camera.h"
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
camera_fb_t *fb;
sensor_t * s;
camera_config_t config;
int pictureNumber = 0;

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
  
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 2); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }

  #if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
    s->set_brightness(s, 0);
  #endif

  camera_fb_t * fb = NULL;
  
  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;

  // Path where new picture will be saved in SD Card
  String path = "/pic" +String(pictureNumber) +".bmp";

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_dis(GPIO_NUM_4);
 
  saveImage(fb, path.c_str());
  
  esp_camera_fb_return(fb);

  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4 to limit current
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);

  esp_sleep_enable_timer_wakeup(5000000);
  esp_deep_sleep_start();

  Serial.println("this should never print");
} 


void loop() {
  /*
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
  */
}// end main loop
/*
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
*/

//
//
//    GRAYSCALE IMG SD SAVE
//
//

// Set the headers data
void setFileHeaders(unsigned char *bitmapFileHeader, unsigned char *bitmapDIBHeader, int fileSize){
    // Set the headers to 0
    memset(bitmapFileHeader, (unsigned char)(0), BITMAP_FILE_HEADER_SIZE);
    memset(bitmapDIBHeader, (unsigned char)(0), DIB_HEADER_SIZE);

    // File header
    bitmapFileHeader[0] = 'B';
    bitmapFileHeader[1] = 'M';
    bitmapFileHeader[2] = (unsigned char)(fileSize);
    bitmapFileHeader[3] = (unsigned char)(fileSize >> 8);
    bitmapFileHeader[4] = (unsigned char)(fileSize >> 16);
    bitmapFileHeader[5] = (unsigned char)(fileSize >> 24);
    bitmapFileHeader[10] = (unsigned char)HEADER_SIZE + PALETTE_SIZE;

    // Info header
    bitmapDIBHeader[0] = (unsigned char)(DIB_HEADER_SIZE);
    bitmapDIBHeader[4] = (unsigned char)(IMAGE_WIDTH);
    bitmapDIBHeader[5] = (unsigned char)(IMAGE_WIDTH >> 8);
    bitmapDIBHeader[8] = (unsigned char)(IMAGE_HEIGHT);
    bitmapDIBHeader[9] = (unsigned char)(IMAGE_HEIGHT >> 8);
    bitmapDIBHeader[14] = (unsigned char)(BITS_PER_PIXEL);
}

void setColorMap(unsigned char *colorMap){
    //Init the palette with zeroes
    memset(colorMap, (unsigned char)(0), PALETTE_SIZE);
    
    // Gray scale color palette, 4 bytes per color (R, G, B, 0x00)
    for (int i = 0; i < PALETTE_COLORS_AMOUNT; i++) {
        colorMap[i * 4] = i;
        colorMap[i * 4 + 1] = i;
        colorMap[i * 4 + 2] = i;
    }
}

// Save the headers and the image data into the .bmp file
void saveImage(camera_fb_t *fb, const char* imagePath){
    int fileSize = BITMAP_FILE_HEADER_SIZE + DIB_HEADER_SIZE + IMAGE_WIDTH * IMAGE_HEIGHT;
    
    // Bitmap structure (Head + DIB Head + ColorMap + binary image)
    unsigned char bitmapFileHeader[BITMAP_FILE_HEADER_SIZE];
    unsigned char bitmapDIBHeader[DIB_HEADER_SIZE];
    unsigned char colorMap[PALETTE_SIZE]; // Needed for <= 8bpp grayscale bitmaps    

    setFileHeaders(bitmapFileHeader, bitmapDIBHeader, fileSize);
    setColorMap(colorMap);

    //Serial.println("Starting SD Card");
    if(!SD_MMC.begin()){
      Serial.println("SD Card Mount Failed");
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE){
      Serial.println("No SD Card attached");
    }
  
    fs::FS &fs = SD_MMC; 
  
    File file = fs.open(imagePath, FILE_WRITE);
    
    if(!file){
      Serial.println("Failed to open file in writing mode");
    } 
    else {
      file.write(bitmapFileHeader, BITMAP_FILE_HEADER_SIZE);
      file.write(bitmapDIBHeader, DIB_HEADER_SIZE);
      file.write(colorMap, PALETTE_SIZE);
      file.write(fb->buf, 19200); // payload (image), payload length
      Serial.printf("Saved file to path: %s\n", imagePath);
      EEPROM.write(0, pictureNumber);
      EEPROM.commit();
    }
    
    file.close();
    //release sd card
    SD_MMC.end();

//    FILE *file = fopen(imagePath, "w");
//
//    // Write the bitmap file
//    fwrite(bitmapFileHeader, 1, BITMAP_FILE_HEADER_SIZE, file);
//    fwrite(bitmapDIBHeader, 1, DIB_HEADER_SIZE, file);
//    fwrite(colorMap, 1, PALETTE_SIZE, file);
//    fwrite(imageData, 1, IMAGE_HEIGHT * IMAGE_WIDTH, file);
//
//    // Close the file stream
//    fclose(file);
}


//void disableWiFi(){
//    adc_power_off();
//    WiFi.disconnect(true);  // Disconnect from the network
//    WiFi.mode(WIFI_OFF);    // Switch WiFi off
//}
