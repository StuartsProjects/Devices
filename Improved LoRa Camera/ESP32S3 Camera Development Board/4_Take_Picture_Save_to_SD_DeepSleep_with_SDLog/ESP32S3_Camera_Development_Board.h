/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 11/06/26
*******************************************************************************************************/

//**************************************************************************************************
// These are the pin definitions for the ESP32S3_Camera_Development_Board
//**************************************************************************************************

#define LED1 RGB_BUILTIN  //evaluates to GPIO 97
#define LEDON HIGH        //logic state for LED to be on
#define LEDOFF LOW        //logic state for LED to be off
#define RGB_LED 48        //pin number for RGB (neopixel)

#define MMC_CMD 38  //SD card data in pin when in MMC mode
#define MMC_SCK 39  //SD card SCK pin when in MMC mode
#define MMC_D0 40   //SD card data out pin when in MMC mode

//pins for CAMERA_MODEL_ESP32S3_EYE
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5

#define Y2_GPIO_NUM 11
#define Y3_GPIO_NUM 9
#define Y4_GPIO_NUM 8
#define Y5_GPIO_NUM 10
#define Y6_GPIO_NUM 12
#define Y7_GPIO_NUM 18
#define Y8_GPIO_NUM 17
#define Y9_GPIO_NUM 16

#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13
