/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 18/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//*******  Setup hardware pin definitions here ! ***************

#define NSS A3                                  //select on LoRa device
#define NRESET A2                               //reset on LoRa device
#define DIO0 A1                                 //DIO0 on LoRa device, used for RX and TX done 
#define LED1 13                                 //On board LED, high for on
#define SDCS A0                                 //CS pin for SD card      

#define LORA_DEVICE DEVICE_SX1278               //this is the device we are using

//*******  Setup LoRa Test Parameters Here ! ***************

//LoRa Modem Parameters
const uint32_t Frequency = 434000000;           //frequency of transmissions
const uint32_t Offset = 0;                      //offset frequency for calibration purposes

const uint8_t Bandwidth = LORA_BW_125;          //LoRa bandwidth
const uint8_t SpreadingFactor = LORA_SF7;       //LoRa spreading factor
const uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting

const uint8_t RXBUFFER_SIZE = 255;              //RX buffer size 
