/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 12/12/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//**************************************************************************************************
// 1) Hardware related definitions and options - specify LoRa board type and pins here
//**************************************************************************************************

//These are the pin definitions for the Easy ESP32CAM board

#define NSS 13                //select on LoRa device
#define SCK 14                //SCK on SPI3
#define MISO 2                //MISO on SPI3 
#define MOSI 15               //MOSI on SPI3 
#define LORA_DEVICE DEVICE_SX1278   //this is the device we are using

#define REDLED 33             //pin number for ESP32CAM on board red LED, set logic level low for on
#define WHITELED 4            //pin number for ESP32CAM on board white LED, set logic level high for on
#define BOOTSWITCH 0          //boot switch
 
#define SupplyAD 14           //Resistor divider analogue read for battery connected here 
#define ADMultiplier 2.9      //adjustment to convert AD value read into mV of battery voltage

#define RXD0 3                //RX pin for GPS port.
#define TXD0 1                //TX pin for GPS port

#define RXD2 40               //RX pin for debug port, not used 
#define TXD2 33               //TX pin for debug port 
#define DEBUGBAUD 115200      //this is the serial baud rate that will be used for the debug port 

const int8_t temperature_compensate = +13;      //value, degrees centigrade, to add to read temperature for calibration. Can be negative
                                                //this compensate value will be different for each LoRa device instance, so best to
                                                //label and record values for each device

//**************************************************************************************************
// 3) LoRa modem settings 
//**************************************************************************************************

//LoRa Modem Parameters
const uint32_t Offset = 0;                             //offset frequency for calibration or adjustement purposes

//Tracker mode
const uint32_t TrackerFrequency = 434000000;           //frequency of transmissions
const uint8_t TrackerSpreadingFactor = LORA_SF8;       //LoRa spreading factor
const uint8_t TrackerBandwidth = LORA_BW_062;          //LoRa bandwidth
const uint8_t TrackerCodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t TrackerOptimisation = LDRO_AUTO;         //low data rate optimisation setting
const int8_t TrackerTXpower = 10;                      //LoRa TX power in dBm

//Search mode
const uint32_t SearchFrequency = 434000000;            //frequency of transmissions
const uint8_t SearchSpreadingFactor = LORA_SF12;       //LoRa spreading factor
const uint8_t SearchBandwidth = LORA_BW_062;;          //LoRa bandwidth
const uint8_t SearchCodeRate = LORA_CR_4_5;            //LoRa coding rate
const uint8_t SearchOptimisation = LDRO_AUTO;          //low data rate optimisation setting
const int8_t SearchTXpower = 10;                       //LoRa TX power in dBm

const uint16_t deviation = 10000;                      //deviation in hz for FM tones
const float adjustfreq = 0.9;                          //adjustment to tone frequency 
const int8_t ToneTXpower = 2;                          //FM Tone power in dBm

const byte TXBUFFER_SIZE = 128;                        //defines the maximum size of the trasnmit buffer;
const byte ThisNode = '1';                             //tracker number for search packet



