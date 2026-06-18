/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 11/6/26

  This programs is supplied as is, it is up to the user of the program to decide if the programs are
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  This program is for the ESP32S3 Camera Development board. The program starts, initialises the
  camera and SD then takes pictures which are saved to SD. The camera is then put into standby mode by
  a register write that will only work on OV3660 and OV5640 cameras, an OV2640 camera will not go into
  standby. The ESP32S3 is then put into deep sleep for a number of seconds.
  
  If the RGB LED turns red then a fault has been detected, see the serial console log for details. 

  The program records any errors detected and displays them on the serial monitor and logs a summary 
  to a file on the SD. If there are any errors found then the RGB LED will go rewd just before each 
  sleep period.  

  Current of board whilst taking pictures 130mA
  Current of board during deep sleep with camera and SD card but no LoRa module or OLED display 0.71mA.

  Issues:

*******************************************************************************************************/

#include "ESP32S3_Camera_Development_Board.h"

#include "esp_camera.h"
sensor_t *sensor;  //referance for camera
camera_config_t config;

#include <SPI.h>
#include <SD_MMC.h>  //for SD card

bool SDFOUND = false;      //set true is SD OK on intialisation
bool CAMERAFOUND = false;  //set to true if Camera initially detected OK
bool PSRAMFOUND;           //set to true if PSRAM detected OK
bool SDSAVEOK = false;     //set to true if image save to SD OK

//RTC_NOINIT_ATTR uint16_t bootCount = 0;    //number of boots\wakeups
RTC_DATA_ATTR uint16_t bootCount = 0;      //number of boots\wakeups
RTC_DATA_ATTR uint16_t imageCount = 1;     //number of images processed
RTC_DATA_ATTR uint16_t ERROR_01 = 0;       //SD card errors
RTC_DATA_ATTR uint16_t ERROR_02 = 0;       //PSRAM init errors
RTC_DATA_ATTR uint16_t ERROR_03 = 0;       //Camera init errors
RTC_DATA_ATTR uint16_t ERROR_04 = 0;       //Image errors
RTC_DATA_ATTR uint16_t ERROR_05 = 0;       //Open file errors
RTC_DATA_ATTR uint16_t ERROR_06 = 0;       //File write errors
RTC_DATA_ATTR uint16_t ERROR_07 = 0;       //Buffer errors
RTC_DATA_ATTR uint16_t ERROR_08 = 0;       //logfile errors
RTC_DATA_ATTR uint16_t SECONDWRITE = 0;    //count of required second writes
RTC_DATA_ATTR uint16_t logfilenumber = 0;  //number of logfile saved at bootCount = 1

#define uS_TO_S_FACTOR 1000000ULL  //conversion factor for micro seconds to seconds

char imagefilename[16];              //filename for saving images to SD
char logfilename[16];                //filename for saving errors to log file
char logfilenew[] = "/LOG0000.TXT";  //filename used as base for creating new logfile
uint32_t available_PSRAM;            //amount of PSRAM found
uint32_t filesize;                   //size of image file saved
uint16_t filecrc;                    //CRC of file saved
uint16_t arrayCRC;                   //CRC or image in PSRAM
uint8_t *PSRAMptr;                   //create a global pointer to the array to send, so all functions have accessuint32_t available_PSRAM;
uint32_t fbsize;                     //size of frame buffer = picture size
uint32_t allocated_PSRAM;            //after PSRAM check, the amount allocated for the camera is stored here
uint8_t PicturesToTake;              //how many pictures left to take at each wake
uint16_t totalerrors;                //sum of ERROR_01 to ERROR_08
//program settings
uint8_t NumberPictures = 5;
uint16_t TimeToSleep = 5;
uint8_t StandardImageSize = FRAMESIZE_SXGA;


void loop() {
  uint8_t index;
  PicturesToTake = NumberPictures;  //number of pictures to take at each wakeup

  if (CAMERAFOUND) {
    sensor->set_reg(sensor, 0x3008, 0xFF, 0x02);  //ensure camera out of standby
    delay(250);
  }

  digitalWrite(LED1, HIGH);
  Serial.println(F("Take Picture"));

  if (CAMERAFOUND)  //Camera is available, so take pictures
  {
    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();  //take initial picture to wake camera up
    esp_camera_fb_return(fb);  //release image buffer
    delay(250);

    for (index = 1; index <= PicturesToTake; index++) {
      sprintf(imagefilename, "/%d.jpg", imageCount);  //create filename in buffer, replaces %d with imagecount

      if (pictureTake(imagefilename, sizeof(imagefilename))) {
        imageCount++;
        delay(250);
      } else {
        Serial.println("ERROR_04 - No picture taken");
        Serial.println();
        ERROR_04++;
        delay(1000);
      }
    }
  } else {
    Serial.println(F("ERROR_03 - No Camera"));
    delay(1000);
  }

  digitalWrite(LED1, LOW);

  if (CAMERAFOUND) {
    Serial.println(F("Camera to idle"));
    sensor->set_reg(sensor, 0x3008, 0xFF, 0x42);  //OV3660 or OV5640 camera to standby
    delay(200);
  }

  print_status();

  totalerrors = ERROR_01 + ERROR_02 + ERROR_03 + ERROR_04 + ERROR_05 + ERROR_06 + ERROR_07 + ERROR_08;

  if (totalerrors) {
    Serial.println(F("Errors detected"));
    rgbLedWrite(RGB_LED, 127, 0, 0);
    delay(2000);  //leave screen on for a couple of seconds
  }

  log_status();

  Serial.println();
  Serial.println(F("Deep sleep"));
  Serial.println();
  Serial.flush();

  rgbLedWrite(RGB_LED, 0, 0, 0);  //RGB LED off

  esp_sleep_enable_timer_wakeup(TimeToSleep * uS_TO_S_FACTOR);
  esp_deep_sleep_start();

  Serial.println(F("Should not see this !!!!!!!!!!"));
}


void print_status() {
  Serial.println();
  Serial.print(F("BootCount "));
  Serial.println(bootCount);
  Serial.print(F("ImageCount "));
  Serial.println(imageCount - 1);
  Serial.print(F("Total errors "));
  Serial.println(totalerrors);

  Serial.print(F("SD card errors "));
  Serial.println(ERROR_01);

  Serial.print(F("PSRAM init errors "));
  Serial.println(ERROR_02);

  Serial.print(F("Camera init errors "));
  Serial.println(ERROR_03);

  Serial.print(F("Image errors "));
  Serial.println(ERROR_04);

  Serial.print(F("File open errors "));
  Serial.println(ERROR_05);

  Serial.print(F("File write errors "));
  Serial.println(ERROR_06);

  Serial.print(F("Buffer errors "));
  Serial.println(ERROR_07);

  Serial.print(F("Logfile errors "));
  Serial.println(ERROR_08);

  Serial.print(F("Second file writes "));
  Serial.println(SECONDWRITE);

  Serial.print(F("Log file number "));
  Serial.println(logfilenumber);

  Serial.println();
}


void log_status() {
  File logFile;

  build_logfilename(logfilenew, logfilenumber);

  Serial.print("Writing to logfile ");
  Serial.println(logfilenew);

  logFile = SD_MMC.open(logfilenew, FILE_APPEND);

  logFile.println();
  logFile.print(F("BootCount "));
  logFile.print(bootCount);

  logFile.print(F("  ImageCount "));
  logFile.print(imageCount - 1);

  logFile.print(F("  Total errors "));
  logFile.println(totalerrors);

  logFile.print(F("ERROR_01 "));
  logFile.print(ERROR_01);

  logFile.print(F(",ERROR_02 "));
  logFile.print(ERROR_02);

  logFile.print(F(",ERROR_03 "));
  logFile.print(ERROR_03);

  logFile.print(F(",ERROR_04 "));
  logFile.print(ERROR_02);

  logFile.print(F(",ERROR_04 "));
  logFile.print(ERROR_03);

  logFile.print(F(",ERROR_05 "));
  logFile.print(ERROR_05);

  logFile.print(F(",ERROR_06 "));
  logFile.print(ERROR_06);

  logFile.print(F(",ERROR_07 "));
  logFile.print(ERROR_07);

  logFile.print(F(",ERROR_08 "));
  logFile.print(ERROR_08);

  logFile.print(F(",Second file writes "));
  logFile.println(SECONDWRITE);

  logFile.close();
}


void build_logfilename(char *buff, uint16_t num) {
  buff[0] = '/';
  buff[1] = 'L';
  buff[2] = 'O';
  buff[3] = 'G';

  buff[4] = num / 1000 + '0';
  buff[5] = ((num % 1000) / 100) + '0';
  buff[6] = ((num % 100) / 10) + '0';
  buff[7] = num % 10 + '0';

  buff[8] = '.';
  buff[9] = 't';
  buff[10] = 'x';
  buff[11] = 't';
  buff[12] = 0;
}



bool pictureTake(char *Name, uint8_t namesize) {
  camera_fb_t *fb = NULL;

  sensor->set_framesize(sensor, (framesize_t)StandardImageSize);
  Serial.print(F("ImageSize "));
  print_ImageSize(StandardImageSize);
  Serial.println();

  fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("ERROR_07 - Failed to get camera frame buffer");
    esp_camera_fb_return(fb);  //release image buffer
    ERROR_07++;
    delay(1000);
    return false;
  }

  PSRAMptr = fb->buf;  //put pointer to frame buffer in global variable
  fbsize = fb->len;    //save image size in global variable

  Serial.print("Picture taken, Image size ");
  Serial.print(fbsize);
  Serial.println(" bytes");

  if (SDFOUND) {
    if (writeFile(Name, PSRAMptr, fbsize))  //save picture to file
    {
      SDSAVEOK = true;
    } else {
      SDSAVEOK = false;
      ERROR_06++;
      Serial.println(F("ERROR_06 file write error"));
      delay(1000);
    }
  } else {
    Serial.println(F("No SD card - failed init"));
    delay(1000);
    SDSAVEOK = false;
  }

  esp_camera_fb_return(fb);  //release image buffer
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
  Serial.printf("Opening file on SD: %s\r\n", path);

  File file = SD_MMC.open(path, FILE_WRITE);

  if (!file) {
    Serial.println("ERROR_05 Failed to open file for writing");
    Serial.println("Trying again");
    File file = SD_MMC.open(path, FILE_WRITE);
    if (!file) {
      ERROR_05++;
      Serial.println("ERROR_05 Again Failed to open file for writing");
      return false;
    }
  }

  Serial.println("File opened for writing");

  if (file.write(data, len) == len) {
    Serial.println("File written");
    return true;
  } else {
    ERROR_06++;
    Serial.println("ERROR_06 File write failed");
    file.close();
    File file = SD_MMC.open(path, FILE_WRITE);
    if (file.write(data, len) == len) {
      Serial.println("Second File write OK");
      SECONDWRITE++;
      return true;
    } else {
      Serial.println("ERROR_06 Second File write failed");
      file.close();
      return false;
    }
    file.close();
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
  config.frame_size = FRAMESIZE_QXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  //config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);  //camera init

  if (err != ESP_OK) {
    Serial.printf("ERROR_05 - Camera init failed with error 0x%x", err);
    return false;
  } else {
    sensor = esp_camera_sensor_get();  //referance for camera
    sensor->set_hmirror(sensor, 0);    //0 = disable , 1 = enable
    sensor->set_vflip(sensor, 0);      //0 = disable , 1 = enable
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
    PSRAMptr = byte_array;  // Save the pointer to byte_array as global pointer

    return true;
  } else {
    return false;
  }
  return false;
}


bool SD_MMCcardinit() {

  SD_MMC.setPins(MMC_SCK, MMC_CMD, MMC_D0);

  if (!SD_MMC.begin("/sdcard", true))  //start MMC in 1 bit mode
  {
    return false;
  } else {
    return true;
  }
}


uint16_t setup_Logfile() {
  //creates a new log filename

  uint16_t index;

  File logFile;

  logFile = SD_MMC.open("/");

  for (index = 1; index <= 9999; index++) {
    logfilenew[4] = index / 1000 + '0';
    logfilenew[5] = ((index % 1000) / 100) + '0';
    logfilenew[6] = ((index % 100) / 10) + '0';
    logfilenew[7] = index % 10 + '0';

    if (!SD_MMC.exists(logfilenew)) {
      // only open a new file if it doesn't exist
      logFile = SD_MMC.open(logfilenew, FILE_WRITE);
      break;
    }
  }

  if (SD_MMC.exists(logfilenew)) {
    Serial.print(F("Created logfile "));
    Serial.println(logfilenew);
  } else {
    digitalWrite(LED1, LEDON);
    Serial.println(F("ERROR_08 creating SD log file"));
    ERROR_08++;
  }

  logFile.rewindDirectory();  //stops SD.exists() command causing issues with directory listings
  logFile.close();

  //Serial.print(F("File creation index "));
  //Serial.println(index);

  return index;  //return number of logfile created
}


void led_Flash(uint16_t flashes, uint16_t delaymS) {
  uint16_t index;

  for (index = 1; index <= flashes; index++) {
    digitalWrite(LED1, LEDON);  //Turn the RGB LED white
    delay(delaymS);
    digitalWrite(LED1, LEDOFF);  //Turn the RGB LED off
    delay(delaymS);
  }
}


void setup() {
  bootCount++;

  pinMode(LED1, OUTPUT);
  led_Flash(4, 125);

  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println(__FILE__);
  Serial.println(F("Program starting"));

  if (SD_MMCcardinit()) {
    SDFOUND = true;
    Serial.println("SD MMC card mount OK");
  } else {
    SDFOUND = false;
    ERROR_01++;
    Serial.println("ERROR_01 SD MMC card mount fail");
    led_Flash(50, 50);
  }

  if (bootCount == 1) {
    Serial.println("Setup Logfile");
    logfilenumber = setup_Logfile();
  }

  if (PSRAM_check()) {
    PSRAMFOUND = true;
  } else {
    PSRAMFOUND = false;
    Serial.println("ERROR_02 PSRAM fail");
    ERROR_02++;
    led_Flash(50, 50);
  }

  if (camera_init()) {
    CAMERAFOUND = true;
  } else {
    CAMERAFOUND = false;
    ERROR_03++;
    Serial.println("ERROR_03 Camera fail");
    led_Flash(50, 50);
  }

  Serial.println();
}
