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
#define EEPROM_SIZE 410

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
uint8_t brightness;
uint32_t oldImageState[100] = {0};
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
  

  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;
} 


void loop() {
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  if(motionDetection(fb)){
    // Path where new picture will be saved in SD Card
    String path = "/pic" +String(pictureNumber) +".bmp";
    saveImage(fb, path.c_str());
    pictureNumber++;
  }

  esp_camera_fb_return(fb);
  
//  Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4 to limit current
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);

  esp_sleep_enable_timer_wakeup(2000000);
  esp_light_sleep_start();

  rtc_gpio_hold_dis(GPIO_NUM_4);

}// end main loop

//
//    MOTION ALGO
//

//basically divide image to 100 (12x16) boxes, sum each box pix values and compare with previous state
uint8_t motionDetection(camera_fb_t *fb){
  char * data = (char *) fb->buf;
  uint8_t imageChangesCnt = 0;

  for(uint8_t box = 0; box < 100; box++){
    uint32_t boxPixSum = 0;

    for(uint8_t row = 0; row < 12; row++){
      for(uint8_t pixel = 0; pixel< 16; pixel++){
        boxPixSum += data[160*12*(box/10) + box%10*16+row*160+pixel];
      }
    }

    float floatBoxStateDiff = abs((float) boxPixSum/oldImageState[box] - 1);
    uint8_t boxChanged = floatBoxStateDiff > 0.15 ? 1 : 0;

    imageChangesCnt += boxChanged;
    oldImageState[box] = boxPixSum;

  }

    return imageChangesCnt > 3 ? 1 : 0;
}

void writeIntIntoEEPROM(int address, int number)
{ 
  byte byte1 = number >> 8;
  byte byte2 = number & 0xFF;
  EEPROM.write(address, byte1);
  EEPROM.write(address + 1, byte2);
}

int readIntFromEEPROM(int address)
{
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}

//
//    GRAYSCALE BMP IMG SD SAVE
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
}


//void disableWiFi(){
//    adc_power_off();
//    WiFi.disconnect(true);  // Disconnect from the network
//    WiFi.mode(WIFI_OFF);    // Switch WiFi off
//}
