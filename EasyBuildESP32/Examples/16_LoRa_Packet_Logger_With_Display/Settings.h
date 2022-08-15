/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 18/07/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//*******  Setup hardware pin definitions here ! ***************

//These are the pin definitions for one of my own boards based on a ATMega328 ProMini 
//be sure to change the definitions to match your own setup. 

#define NSS 5                                  //select pin on LoRa device
#define NRESET 27                              //reset pin on LoRa device
#define DIO0 35                                //DIO0 pin on LoRa device, used for RX and TX done 

#define SCK 18                                 //SPI SCK 
#define MOSI 23                                //SPI MOSI
#define MISO 19                                //SPI MISO

#define LORA_DEVICE DEVICE_SX1278               //we need to define the device we are using


//*******  Setup LoRa Parameters Here ! ***************

//LoRa Modem Parameters
const uint32_t Frequency = 434000000;           //frequency of transmissions in hertz
const uint32_t Offset = 0;                      //offset frequency for calibration purposes

const uint8_t Bandwidth = LORA_BW_125;          //LoRa bandwidth
const uint8_t SpreadingFactor = LORA_SF7;       //LoRa spreading factor
const uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting, normally set to auto

const uint32_t RXtimeoutmS = 60000;             //RXtimeout in mS  
const uint8_t RXBUFFER_SIZE = 128;              //RX buffer size  

const uint8_t LCD20x4_I2C_ADDRESS = 0x3F;       //I2C address of PCF8574 controller on LCD, could be 0x27 or 0x3F
