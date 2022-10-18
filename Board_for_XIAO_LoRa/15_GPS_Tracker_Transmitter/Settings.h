/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 16/12/19

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//*******  Setup hardware pin definitions here ! ***************

//These are the pin definitions for one of my own boards, the Easy Pro Mini,
//be sure to change the definitiosn to match your own setup. 

#define NSS A3                                  //select pin on LoRa device
#define NRESET A2                               //reset pin on LoRa device
#define DIO0 A1                                 //DIO0 pin on LoRa device, used for sensing RX and TX done 
#define LED1 13                                 //on board LED, yellow  
#define ADMultiplier 1.628                      //adjustment to convert AD value read into mV of battery voltage 
#define SupplyAD A0                             //Resitor divider for battery voltage read connected here 

#define LORA_DEVICE DEVICE_SX1278               //this is the device we are using 



//*******  Setup LoRa Parameters Here ! ***************

//LoRa Modem Parameters
const uint32_t Frequency = 434000000;           //frequency of transmissions
const uint32_t Offset = 0;                      //offset frequency for calibration purposes

const uint8_t Bandwidth = LORA_BW_125;          //LoRa bandwidth
const uint8_t SpreadingFactor = LORA_SF7;       //LoRa spreading factor
const uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting

const int8_t TXpower = 10;                       //LoRa TX power

#define ThisNode '1'                             //a character that identifies this tracker

//**************************************************************************************************
// GPS Settings
//**************************************************************************************************

#define GPSBaud 9600                             //GPS Baud rate   

#define WaitGPSFixSeconds 30                     //time in seconds to wait for a new GPS fix 
#define WaitFirstGPSFixSeconds 120               //time to seconds to wait for the first GPS fix at startup
#define Sleepsecs 15                             //seconds between transmissions, this delay is used to set overall transmission cycle time
