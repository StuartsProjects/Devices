/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 03/11/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//*******  Setup hardware pin definitions here ! ***************

#define NSS D3                                  //select on LoRa device
#define NRESET D2                               //reset on LoRa device
#define DIO0 D1                                 //DIO0 on LoRa device, used for RX and TX done 
#define SDCS D0                                 //CS pin for SD card      

//red on board LED is pin 17, green 16, blue 25
#define RED1 17
#define GREEN1 16
#define BLUE1 25

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
