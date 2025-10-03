/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 02/10/25

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//#define USE_LR1121  //enable for LR1121
#define USE_SX1262  //enable for SX1262

#define USE_ESP32       //enable define if an ESP32 in use which allows SPI and I2C pins to be defined
#define USE_LILYGOT3S3  //enable if using Lilygo T3S3, needed to display battery voltage

//For Lilygo T3S3 with LR1121 or SX1262 module
#define NSS_PIN 7    //select pin on LoRa device
#define RESET_PIN 8  //reset pin on LoRa device
#define IRQ_PIN 36   //IRQ pin on LoRa LR1121 device, DIO9, used for sensing RX and TX done
#define DIO1_PIN 33  //DIO1 pin on LoRa SX1262 device, used for sensing RX and TX done
#define BUSY_PIN 34  //BUSY pin on LoRa device

#define LED1 37
#define BUZZER_PIN 10  //pin 3 of P3, connector nearest long edge of board
#define DIO9_PIN 36
#define RADIO_SCLK_PIN 5
#define RADIO_MISO_PIN 3
#define RADIO_MOSI_PIN 6

#define SDCARD_MOSI 11
#define SDCARD_MISO 2
#define SDCARD_SCLK 14
#define SDCARD_CS 13

#define I2C_SDA 18
#define I2C_SCL 17
#define OLEDAddress 0x3C

#define ADC_PIN 1

#define BAT_ADC_PULLUP_RES (100000.0)
#define BAT_ADC_PULLDOWN_RES (100000.0)
#define BAT_MAX_VOLTAGE (4.2)
#define BAT_VOL_COMPENSATION (0.0)

//QWIIC connectors, pin 1 is GND, pin 2 is 3.3v
//P1 is connector nearest to USB\battery connector
//P3 is connector nearest long edge of board
#define QWIIC_P1_U0TXD 43  //pin 3 of P1
#define QWIIC_P1_U0RXD 44  //pin 4 of P1

#define QWIIC_P3_LORA_TX 10  //pin 3 of P3, would be SDA for I2C QWICC
#define QWIIC_P3_LORA_RX 21  //pin 4 of P3, would be SCL for I2C QWICC
