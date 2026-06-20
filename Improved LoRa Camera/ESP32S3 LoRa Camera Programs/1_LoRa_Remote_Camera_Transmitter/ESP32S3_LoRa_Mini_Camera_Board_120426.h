/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 19/06/26
*******************************************************************************************************/

//**************************************************************************************************
// These are the SPI pin definitions for the ESP32S3_LoRa_Mini_Camera_Board_120426 as Image Transmitter
//**************************************************************************************************

#define LORA_DEVICE DEVICE_SX1262  //We need to define the device we are using

#define LORA_MOSI 2    //LoRa MOSI pin
#define LORA_SCK 42    //LoRa SCK pin
#define LORA_MISO 41   //LoRa MISO pin
#define LORA_NSS 14    //LoRa NSS pin
#define LORA_BUSY 47   //LoRa BUSY pin
#define LORA_RESET 21  //LoRa Reset pin
#define LORA_DIO1 1    //LoRa DIO1 pin for SX126X\SX128X
#define LORA_DIO0 1    //LoRa DIO0 pin for SX127X

#define LED1 RGB_BUILTIN  //GPIO 97
#define RGB_LED 48        //pin number for RGB (neopixel)
#define LEDON HIGH        //logic state for LED to be on
#define LEDOFF LOW        //logic state for LED to be off

#define SDA 19  //I2C connection when camera in use
#define SCL 20  //I2C connection when camera in use

#define SDA1 4  //I2C connection when camera not in use
#define SCL1 5  //I2C connection when camera not in use

#define MMC_CMD 38  //SD card data in pin when in MMC mode
#define MMC_SCK 39  //SD card SCK pin when in MMC mode
#define MMC_D0 40   //SD card data out pin when in MMC mode

#define SD_MOSI 38  //SD card MOSI pin in SPI mode
#define SD_SCK 39   //SD card SCK pin in SPI mode
#define SD_MISO 40  //SD card MISO pin in SPI mode
#define SD_CS 3     //SD card CS pin in SPI mode

const uint16_t drotation = 90;   //choice of 0,90,180,270
const uint16_t tftwidth = 320;   //pixel width of TFT display used display used, normally largest number when rotation is 90
const uint16_t tftheight = 240;  //pixel height of display used, normally smallest number when rotation is 90

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

//Camera settings
#define HMIRROR 1  //0 = disable , 1 = enable
#define VFLIP 0    //0 = disable , 1 = enable
