/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-save-microsd-card
  Github repository at https://github.com/RuiSantosdotme/ESP32-CAM-Arduino-IDE

  IMPORTANT!!!
   - Select Board "AI Thinker ESP32-CAM"
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/


#include "esp_camera.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32


// Pin definition for CAMERA_MODEL_AI_THINKER
// Change pin definition if you're using another ESP32 with camera module
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

RTC_DATA_ATTR uint16_t pictureNumber = 0;    //number of picture taken, set to 0 on reset

camera_config_t config;                      //stores the camera configuration parameters

bool configInitCamera();
bool initMicroSDCard();
uint16_t takeSavePhoto(uint8_t num, uint32_t gapmS);
void LEDflash(uint16_t flashes, uint16_t ondelaymS, uint16_t offdelaymS);

bool configInitCamera()
{
  Serial.print("Initialising the camera module ");

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
  config.pixel_format = PIXFORMAT_JPEG; //YUV422,GRAYSCALE,RGB565,JPEG

  //Select lower framesize if the camera doesn't support PSRAM
  if (psramFound())
  {
    Serial.println("PSRAM found");
    config.frame_size = FRAMESIZE_UXGA; //FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;           //10-63 lower number means higher quality
    config.fb_count = 2;
  } else
  {
    Serial.println("No PSRAM");
    config.frame_size = FRAMESIZE_XGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);   //Initialize the Camera
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    Serial.println();
    return false;
  }

  Serial.println(" OK");

  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
  return true;
}


bool initMicroSDCard()
{
  Serial.print("Starting SD Card ");

  if(!SD_MMC.begin("/sdcard",true))               //use this line for 1 bit mode
  {
    Serial.println(" Card Mount Failed");
    LEDflash(2, 1, 199);                          //indicate an SD card error
    delay(1000);
    return false;
  }
  Serial.println("OK");

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD Card attached");
    LEDflash(2, 1, 199);                          //indicate an SD card error
    delay(1000);
    return false;
  }
  return true;
}


uint16_t takeSavePhoto(uint8_t num, uint32_t gapmS)
{
  uint8_t index;

  for (index = 1; index <= num; index++)
  {
    pictureNumber++;
    String path = "/picture" + String(pictureNumber) + ".jpg";
    Serial.printf("Picture file name: %s\n", path.c_str());

    camera_fb_t  * fb = esp_camera_fb_get();

    if (!fb)
    {
      Serial.println("Camera capture failed");
      LEDflash(3, 1, 199);                          //indicate a camera error
      delay(1000);
      pictureNumber--;                              //restore picture number
      return 0;                                     //a return of 0 means no picture
    }

    Serial.println("Camera capture success");

    fs::FS &fs = SD_MMC;                            //save picture to microSD card
    File file = fs.open(path.c_str(), FILE_WRITE);
    if (!file)
    {
      Serial.println("Failed to open file in writing mode");
      LEDflash(2, 1, 199);                          //indicate an SD card error
      delay(1000);
      pictureNumber--;                              //restore picture number
      return 0;                                     //a return of 0 means no picture
    }
    else
    {
      file.write(fb->buf, fb->len); // payload (image), payload length
      Serial.printf("Saved file to path: %s\n", path.c_str());
      LEDflash(1, 1, 199);                          //single flash - picture taken and saved
      delay(1000);
    }
    file.close();

    esp_camera_fb_return(fb);                       //return the frame buffer back to the driver for reuse

    //note the picture number will increase even if the SD card has failed.
    //if there are gaps in the saved picture numbers on SD then the card is intermittent
    delay(gapmS);
  }
  return pictureNumber;
}


void LEDflash(uint16_t flashes, uint16_t ondelaymS, uint16_t offdelaymS)
{
  uint16_t index;

  pinMode(4, OUTPUT);                     //setup white LED pin as output

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(4, HIGH);
    delay(ondelaymS);
    digitalWrite(4, LOW);
    delay(offdelaymS);
  }
}
