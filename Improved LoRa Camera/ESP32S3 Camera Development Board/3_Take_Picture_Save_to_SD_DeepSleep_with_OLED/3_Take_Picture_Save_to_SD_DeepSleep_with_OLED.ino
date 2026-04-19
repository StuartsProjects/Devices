/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 19/4/26

  This programs is supplied as is, it is up to the user of the program to decide if the programs are
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  This program is for the ESP32S3 Camera Development board. The program starts, initialises the
  camera and SD then takes pictures which are saved to SD. The camera is then put into standby mode by
  a register write that will only work on OV3660 and OV5640 cameras, an OV2640 camera will not go into
  standby. 

  In the Arduino IDE set the board type to be 'ESP32S3 DEV Module'. You need to enable OPI PSRAM in
  the IDE settings.

  At startup if there is an error accessing the OLED display then the RGB LED will flash red
  continuously. If there is a PSRAM error at startup the RGB LED will flash green continuously. 
  
  At the end of the picture taking sequence a total of the errors found is printed to the Serial monitor.
  If the OLED is connected then there will be status messages of pictures taken and errors shown on the 
  display. If there are errors the neopixel LED will stay red during the sleep. 

  The ESP32S3 is then put into sleep for a number of seconds.
  
  Note that the camera functions require that a recent copy of the ESP32 core is used, this program was
  tested with core 3.3.1 and IDE 2.3.8. There can be compilation errors if the  earlier version 2 core
  is used. You need to enable OPI PSRAM in the IDE settings. 

 
*******************************************************************************************************/
#include "ESP32S3_Camera_Development_Board.h"

#include <Wire.h>
#include "esp_camera.h"
sensor_t *sensor;         //referance for camera
camera_config_t config;

#include <SPI.h>
#include <SD_MMC.h>       //for SD card
File file;

#include <U8g2lib.h>      //get library here > https://github.com/olikraus/U8g2_Arduino
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C disp(U8G2_R0, U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C disp(U8G2_R0, U8X8_PIN_NONE);

#define DEFAULTFONT u8x8_font_chroma48medium8_r  //font for U8X8 Library

bool SDFOUND = false;      //set true is SD OK on intialisation
bool CAMERAFOUND = false;  //set to true if Camera initially detected OK
bool PSRAMFOUND;           //set to true if PSRAM detected OK
bool SDSAVEOK = false;     //set to true if image save to SD OK
bool OLEDFOUND = false;

#define uS_TO_S_FACTOR 1000000ULL       //conversion factor for micro seconds to seconds
RTC_DATA_ATTR uint16_t bootcount = 0;   //number of boots\wakeups
RTC_DATA_ATTR uint16_t imageCount = 1;  //number of images processed
RTC_DATA_ATTR uint16_t ERROR01 = 0;     //SD card init errors
RTC_DATA_ATTR uint16_t ERROR02 = 0;     //PSRAM init errors
RTC_DATA_ATTR uint16_t ERROR03 = 0;     //Camera init errors
RTC_DATA_ATTR uint16_t ERROR04 = 0;     //Image errors
RTC_DATA_ATTR uint16_t ERROR05 = 0;     //File write errors
RTC_DATA_ATTR uint16_t ERROR06 = 0;     //Buffer errors

char filename[16];                      //filename for saving to SD
uint32_t available_PSRAM;               //amount of PSRAM found
uint32_t filesize;                      //size of image file saved
uint16_t filecrc;                       //CRC of file saved
uint16_t arrayCRC;                      //CRC or image in PSRAM
uint8_t *PSRAMptr;                      //create a global pointer to the array to send, so all functions have accessuint32_t available_PSRAM;
uint32_t fbsize;                        //size of frame buffer = picture size
uint32_t allocated_PSRAM;               //after PSRAM check, the amount allocated for the camera is stored here
uint8_t PicturesToTake;                 //how many pictures left to take at each wake
uint16_t totalerrors;                   //sum of ERROR01 to ERROR06 

//program settings
uint8_t NumberPictures = 3;                       //how many pictures to take at each wakeup
uint16_t TimeToSleep = 10;                        //number of seconds to sleep   
uint8_t StandardImageSize = FRAMESIZE_SVGA;       //size of image to take, see file 'frame_sizes.h' fo the available sizes.


void loop() {
  uint8_t index;
  PicturesToTake = NumberPictures;                //number of pictures to take at each wakeup

  if (CAMERAFOUND) {
    sensor->set_reg(sensor, 0x3008, 0xFF, 0x02);  //ensure camera out of standby
    delay(200);
  }

  digitalWrite(LED1, HIGH);
  Serial.println(F("Take Picture"));

  if (CAMERAFOUND)                                //Camera is available, so take pictures
  {
    for (index = 1; index <= PicturesToTake; index++) {
      sprintf(filename, "/%d.jpg", imageCount);   //create filename in buffer, replaces %d with imagecount

      if (pictureTake(filename, sizeof(filename), (PicturesToTake - index))) {
        disp.clearBuffer();
        disp.setCursor(0, 15);
        disp.print("Picture ");
        disp.print(imageCount);
        disp.print(" OK");
        disp.sendBuffer();
        imageCount++;
        delay(200);
      } else {
        Serial.println("ERROR04 - No picture taken");
        Serial.println();
        disp.print(" Error");
        ERROR04++;
        delay(2000);
      }
    }
  } else {
    Serial.println(F("ERROR03 - No Camera"));
    disp.clearBuffer();
    disp.setCursor(0, 15);
    disp.print("Camera error");
    disp.sendBuffer();
    delay(2000);
  }

  digitalWrite(LED1, LOW);

  if (CAMERAFOUND) {
    Serial.println(F("Camera to idle"));
    delay(50);
    sensor->set_reg(sensor, 0x3008, 0xFF, 0x42);  //OV3660 or OV5640 camera to standby
    delay(200);
  }

  print_status();

  Serial.println();
  Serial.println(F("Deep sleep"));
  Serial.println();
  Serial.flush();

  display_status();

  totalerrors = ERROR01 + ERROR02 + ERROR03 + ERROR04 + ERROR05 + ERROR06;

  if (totalerrors) {
    Serial.println(F("Errors detected"));
    rgbLedWrite(RGB_LED, 127, 0, 0);
  }
  else
  {
    rgbLedWrite(RGB_LED, 0, 0, 0);        //neopixel LED off  
  }

  delay(2500);                            //leave screen on for a couple of seconds
  display_imagecount();
  delay(2500);                            //leave screen on for a couple of seconds

  disp.clear();
  disp.setPowerSave(1);                   //power save display

  esp_sleep_enable_timer_wakeup(TimeToSleep * uS_TO_S_FACTOR);
  esp_deep_sleep_start();

  Serial.println(F("Should not see this !!!!!!!!!!"));
}


void print_status() {
  Serial.println();
  Serial.print(F("BootCount "));
  Serial.println(bootcount);
  Serial.print(F("ImageCount "));
  Serial.println(imageCount - 1);

  Serial.print(F("SD card init errors "));
  Serial.println(ERROR01);

  Serial.print(F("PSRAM init errors "));
  Serial.println(ERROR02);

  Serial.print(F("Camera init errors "));
  Serial.println(ERROR03);

  Serial.print(F("Image errors "));
  Serial.println(ERROR04);

  Serial.print(F("File errors "));
  Serial.println(ERROR05);

  Serial.print(F("Buffer errors "));
  Serial.println(ERROR06);

  Serial.println();
}

void display_status() {
  disp.clearBuffer();
  disp.setFont(u8g2_font_5x7_mf);
  disp.setCursor(0, 7);
  disp.print("Images     ");
  disp.print(imageCount - 1);
  disp.setCursor(0, 15);
  disp.print("Boots      ");
  disp.print(bootcount);

  disp.setCursor(0, 23);
  disp.print("SD errs    ");
  disp.print(ERROR01);

  disp.setCursor(0, 31);
  disp.print("PSRAM errs ");
  disp.print(ERROR02);

  disp.setCursor(0, 39);
  disp.print("Cam errs   ");
  disp.print(ERROR03);

  disp.setCursor(0, 47);
  disp.print("Image errs ");
  disp.print(ERROR04);

  disp.setCursor(0, 55);
  disp.print("File errs  ");
  disp.print(ERROR05);

  disp.setCursor(0, 63);
  disp.print("Buff errs  ");
  disp.print(ERROR06);
  disp.sendBuffer();
}


void display_imagecount() {
  disp.clearBuffer();
  //disp.setFont(u8g2_font_pxplusibmvga8_mr);         //10 pixel height
  //disp.setFont(u8g2_font_crox3cb_mf);               //12 pixel height
  disp.setFont(u8g2_font_chargen_92_mn);            //14 pixel height
  //disp.setFont(u8g2_font_inb16_mr);                 //16 pixel height

  disp.setCursor(8, 29);
  disp.print(imageCount - 1);
  
  if (totalerrors)
  {
  disp.setCursor(8, 49);
  disp.print(totalerrors);
  }
  
  disp.sendBuffer();
}


bool pictureTake(char *Name, uint8_t namesize, uint8_t picsleft) {
  camera_fb_t *fb = esp_camera_fb_get();             //take a picture
  esp_camera_fb_return(fb);                          //release image buffer
  delay(100);

  sensor->set_framesize(sensor, (framesize_t)StandardImageSize);
  esp_camera_fb_get();                               //take a picture
  esp_camera_fb_return(fb);                          //release image buffer
  Serial.print(F("StandardImageSize "));
  print_ImageSize(StandardImageSize);
  Serial.println();

  esp_camera_fb_get();

  if (!fb) {
    Serial.println("ERROR - Failed to get camera frame buffer");
    esp_camera_fb_return(fb);  //release image buffer
    disp.setCursor(0, 31);
    disp.print("Buffer error");
    disp.sendBuffer();
    ERROR06++;
    delay(2000);
    return false;
  }

  PSRAMptr = fb->buf;                        //put pointer to frame buffer in global variable
  fbsize = fb->len;                          //save image size in global variable

  Serial.print("Picture taken, Image size ");
  Serial.print(fbsize);
  Serial.println(" bytes");

  if (SDFOUND) {
    if (writeFile(Name, PSRAMptr, fbsize))  //save picture to file
    {
      SDSAVEOK = true;
    } else {
      SDSAVEOK = false;
      ERROR05++;
      Serial.println("ERROR05 File save error");
      disp.setCursor(0, 31);
      disp.print("File error");
      disp.sendBuffer();
      delay(2000);
    }
  } else {
    Serial.println(F("ERROR01 - No SD card"));
    disp.setCursor(0, 31);
    disp.print("SD error");
    disp.sendBuffer();
    delay(2000);
    SDSAVEOK = false;
  }

  Serial.print("Images left in sequence ");
  Serial.println(picsleft);

  esp_camera_fb_return(fb);                //release image buffer
  Serial.println("Released image buffer ");
  Serial.flush();
  return true;
}


void print_ImageSize(uint8_t imagesize) {
  switch (imagesize) {
    case FRAMESIZE_96X96:
      Serial.print(F("0 FRAMESIZE_96X96 96x96"));
      break;

    case FRAMESIZE_QQVGA:
      Serial.print(F("1 FRAMESIZE_QQVGA 160x120"));
      break;

    case FRAMESIZE_128X128:
      Serial.print(F("2 FRAMESIZE_128X128 128x128"));
      break;

    case FRAMESIZE_QCIF:
      Serial.print(F("3 FRAMESIZE_QCIF 176x144"));
      break;

    case FRAMESIZE_HQVGA:
      Serial.print(F("4 FRAMESIZE_HQVGA 240x176"));
      break;

    case FRAMESIZE_240X240:
      Serial.print(F("5 FRAMESIZE_240x240 240x240"));
      break;

    case FRAMESIZE_QVGA:
      Serial.print(F("6 FRAMESIZE_QVGA 320x240"));
      break;

    case FRAMESIZE_320X320:
      Serial.print(F("7 FRAMESIZE_320X320 320X320"));
      break;

    case FRAMESIZE_CIF:
      Serial.print(F("8 FRAMESIZE_CIF 400x296"));
      break;

    case FRAMESIZE_HVGA:
      Serial.print(F("9 FRAMESIZE_HVGA 480x320"));
      break;

    case FRAMESIZE_VGA:
      Serial.print(F("10 FRAMESIZE_VGA 640x480"));
      break;

    case FRAMESIZE_SVGA:
      Serial.print(F("11 FRAMESIZE_SVGA 800x600"));
      break;

    case FRAMESIZE_XGA:
      Serial.print(F("12 FRAMESIZE_XGA 1024x768"));
      break;

    case FRAMESIZE_HD:
      Serial.print(F("13 FRAMESIZE_HD 1280x720"));
      break;

    case FRAMESIZE_SXGA:
      Serial.print(F("14 FRAMESIZE_SXGA 1280x1024"));
      break;

    case FRAMESIZE_UXGA:
      Serial.print(F("15 FRAMESIZE_UXGA 1600x1200"));
      break;

    //3MP Sensors

    case FRAMESIZE_FHD:
      Serial.print(F("16 FRAMESIZE_FHD 1920x1080"));
      break;

    case FRAMESIZE_P_HD:
      Serial.print(F("17 FRAMESIZE_P_HD 720x1280"));
      break;

    case FRAMESIZE_P_3MP:
      Serial.print(F("18 FRAMESIZE_P_3MP 864x1536"));
      break;

    case FRAMESIZE_QXGA:
      Serial.print(F("19 FRAMESIZE_QXGA 2048x1536"));
      break;

    //5MP Sensors

    case FRAMESIZE_QHD:
      Serial.print(F("20 FRAMESIZE_QHD 2560x1440"));
      break;

    case FRAMESIZE_WQXGA:
      Serial.print(F("21 FRAMESIZE_WQXGA 2560x1600"));
      break;

    case FRAMESIZE_P_FHD:
      Serial.print(F("22 FRAMESIZE_P_FHD 1080x1920"));
      break;

    case FRAMESIZE_QSXGA:
      Serial.print(F("23 FRAMESIZE_QSXGA 2560x1920"));
      break;

    case FRAMESIZE_5MP:
      Serial.print(F("24 FRAMESIZE_5MP 2592x1944"));
      break;

    case FRAMESIZE_INVALID:
      Serial.print(F("25 FRAMESIZE_INVALID"));
      break;

    default:
      Serial.print(F("FRAMESIZE_INVALID"));
      break;
  }
}


bool writeFile(const char *path, uint8_t *data, size_t len) {
  Serial.printf("Writing file to SD: %s\r\n", path);

  File file = SD_MMC.open(path, FILE_WRITE);

  if (!file) {
    Serial.println("ERROR05 Failed to open file for writing");
    return false;
  }

  if (file.write(data, len) == len) {
    Serial.println("File written");
    return true;
  } else {
    Serial.println("ERROR05 File write failed");
    return false;
  }
  file.close();
}


bool camera_init() {
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QXGA;        //do intial setup with largest image size
  config.pixel_format = PIXFORMAT_JPEG;
  //config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);  //camera init

  if (err != ESP_OK) {
    Serial.printf("ERROR - Camera init failed with error 0x%x", err);
    return false;
  } else {
    sensor = esp_camera_sensor_get();       //referance for camera
    sensor->set_hmirror(sensor, 0);         //0 = disable , 1 = enable
    sensor->set_vflip(sensor, 0);           //0 = disable , 1 = enable
    return true;
  }
}


bool PSRAM_check() {
  if (psramFound()) {
    available_PSRAM = ESP.getFreePsram();
    Serial.print("PSRAM available: ");
    Serial.print(available_PSRAM);
    Serial.println(" bytes");

    allocated_PSRAM = available_PSRAM / 2;  // Dont use all PSRAM for array
    Serial.print("Allocate ");
    Serial.print(allocated_PSRAM);
    Serial.println(" bytes of PSRAM as image array");

    uint8_t *byte_array = (uint8_t *)ps_malloc((allocated_PSRAM) * sizeof(uint8_t));
    PSRAMptr = byte_array;                 // Save the pointer to byte_array as global pointer

    return true;
  } else {
    return false;
  }
  return false;
}


bool SD_MMCcardinit() {

  SD_MMC.setPins(MMC_SCK, MMC_CMD, MMC_D0);

  if (!SD_MMC.begin("/sdcard", true))     //start MMC in 1 bit mode
  {
    return false;
  } else {
    return true;
  }
}


bool init_Display(uint8_t oledaddress) {

  Wire.begin(SDA, SCL);

  Wire.beginTransmission(oledaddress);
  int16_t error = Wire.endTransmission();

  if (error == 0) {
    disp.begin();
    disp.clearBuffer();
    disp.sendBuffer();
    return true;
  } else {
    return false;
  }
}


void led_Flash(uint16_t flashes, uint16_t delaymS) {
  uint16_t index;

  for (index = 1; index <= flashes; index++) {
    digitalWrite(LED1, LEDON);   //Turn the RGB LED white
    delay(delaymS);
    digitalWrite(LED1, LEDOFF);  //Turn the RGB LED off
    delay(delaymS);
  }
}


void rgb_Flash(uint16_t flashes, uint16_t delaymS, uint8_t Redbright,  uint8_t Greenbright,  uint8_t Bluebright)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    rgbLedWrite(RGB_LED, Redbright, Greenbright, Bluebright); //RGB LED on
    delay(delaymS);
    rgbLedWrite(RGB_LED, 0, 0, 0);                            //RGB LED off
    delay(delaymS);
  }
}


void setup() {
  bootcount++;

  pinMode(LED1, OUTPUT);
  led_Flash(5, 250);

  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println(__FILE__);
  Serial.println(F("Program starting"));

  if (init_Display(OLEDAddress))
  {
    Serial.println("OLED found");
    OLEDFOUND = true;
    disp.clearBuffer();
    disp.setFont(u8g2_font_pxplusibmvga8_mr);
    disp.setCursor(8, 15);
    disp.print("Camera awake");
    disp.sendBuffer();
  }
  else {
    Serial.println("OLED not found.");
    OLEDFOUND = false;
    while(1)
    {
     rgb_Flash(25, 50, 64, 0, 0);
    };
  }

  if (SD_MMCcardinit()) {
    SDFOUND = true;
    Serial.println("SD MMC card mount OK");
  } else {
    SDFOUND = false;
    ERROR01++;
    Serial.println("ERROR01 SD MMC card mount fail");
    disp.clearBuffer();
    disp.setCursor(0, 15);
    disp.print("SD mount error");
    disp.sendBuffer();
    led_Flash(25, 50);
  }

  if (PSRAM_check()) {
    PSRAMFOUND = true;
  } else {
    PSRAMFOUND = false;
    Serial.println("ERROR02 PSRAM fail");
    ERROR02++;
    disp.clearBuffer();
    disp.setCursor(0, 15);
    disp.print("PSRAM error");
    disp.sendBuffer();
    
    while(1)
    {
     rgb_Flash(25, 50, 0, 64, 0);
    };
  }

  if (camera_init()) {
    CAMERAFOUND = true;
  } else {
    CAMERAFOUND = false;
    ERROR03++;
    Serial.println("ERROR03 Camera fail");
    led_Flash(25, 50);
  }

  Serial.println();
}
