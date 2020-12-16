/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 12/12/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//**************************************************************************************************
// 1) Hardware related definitions and options - specify lora board type and pins here
//**************************************************************************************************

//These are the pin definitions for one of my own boards, the Easy Mikrobus Pro Mini,
//be sure to change the definitions to match your own setup. 

#define NSS 13                //select on LoRa device
#define SCK 14                //SCK on SPI3
#define MISO 2                //MISO on SPI3 
#define MOSI 15               //MOSI on SPI3 

#define NRESET 41             //reset on LoRa device - not used
#define DIO0 12               //DIO0 on LoRa device, can be used for RX and TX done 
#define LED1 13               //indicator LED, low for on

#define REDLED 33             //pin number for ESP32CAM on board red LED, set logic level low for on
#define WHITELED 4            //pin number for ESP32CAM on board white LED, set logic level high for on
#define BOOTSWITCH 0          //boot switch

#define SupplyAD 14           //Resitor divider analogue read for battery connected here 
#define ADMultiplier 2.9      //adjustment to convert AD value read into mV of battery voltage

#define RXD0 3              //RX pin for GPS port.
#define TXD0 1              //TX pin for GPS port

#define RXD2 40               //RX pin for debug port, not used 
#define TXD2 33               //TX pin for debug port 

#define GPSBAUD 9600          //this is the serial baud rate that will be used for the GPS, a common default
#define DEBUGBAUD 115200      //this is the serial baud rate that will be used for the debug port 

const int8_t temperature_compensate = +13;      //value, degrees centigrade, to add to read temperature for calibration. Can be negative
                                                //this compensate value will be different for each LoRa device instance, so best to
                                                //label and record values for each device

#define LORA_DEVICE DEVICE_SX1278               //this is the device we are using


//**************************************************************************************************
// 3) LoRa modem settings 
//**************************************************************************************************

//LoRa Modem Parameters
const uint32_t Offset = 0;                       //offset frequency for calibration purposes

//Tracker mode
const uint32_t TrackerFrequency = 434000000;     //frequency of transmissions
const uint8_t TrackerBandwidth = LORA_BW_062;    //LoRa bandwidth
const uint8_t TrackerSpreadingFactor = LORA_SF8; //LoRa spreading factor
const uint8_t TrackerCodeRate = LORA_CR_4_5;     //LoRa coding rate
const uint8_t TrackerOptimisation = LDRO_AUTO;   //low data rate optimisation setting
const int8_t TrackerTXpower = 10;                //LoRa TX power in dBm

//Search mode
const uint32_t SearchFrequency = 434000000;      //frequency of transmissionsconst 
uint8_t SearchBandwidth = LORA_BW_062;           //LoRa bandwidth
const uint8_t SearchSpreadingFactor = LORA_SF12; //LoRa spreading factor
const uint8_t SearchCodeRate = LORA_CR_4_5;      //LoRa coding rate
const uint8_t SearchOptimisation = LDRO_AUTO;    //low data rate optimisation setting
const int8_t SearchTXpower = 10;                 //LoRa TX power in dBm

const uint16_t deviation = 10000;                //deviation in hz for FM tones
const float adjustfreq = 0.9;                    //adjustment to tone frequency 
const int8_t ToneTXpower = 2;                    //FM Tone power in dBm
const byte TXBUFFER_SIZE = 128;                  //defines the maximum size of the trasnmit buffer;


//**************************************************************************************************
// 4) GPS Options
//**************************************************************************************************

#define GPSBaud 9600                             //GPS Baud rate

const uint16_t WaitGPSFixSeconds = 60;           //the time to wait for a new GPS fix 

//#define GPS_Library <UBLOX_HWSerialGPS.h>      //use library file for UBLOX hardware GPS                    
#define GPS_Library <Quectel_HWSerialGPS.h>      //use library file for Quectel GPS

#define GPSserial Serial2                        //GPS is on Serial2


//**************************************************************************************************
// 5) FSK RTTY Settings
//**************************************************************************************************

uint32_t FrequencyShift = 500;                    //hertz frequency shift for audio  
uint8_t NumberofPips = 4;                         //number of marker pips to send
uint16_t PipDelaymS = 1000;                       //mS between pips when carrier is off 
uint16_t PipPeriodmS = 100;                       //mS length of pip
uint16_t BaudPerioduS = 10000;                    //uS period for baud, 10000uS for 100baud 
uint16_t LeadinmS = 1000;                         //ms of leadin constant shifted carrier 


//****************************************************************************************************
// 6) Program Default Option settings - This section determines which options are on or off by default,
//    these are saved in the Default_config1 byte. These options are set in this way so that it is 
//    possible (in future program changes) to alter the options remotly. 
//**************************************************************************************************

uint8_t OptionOff = 0;
uint8_t OptionOn = 1;

const char option_SearchEnable = OptionOn;        //set to OptionOn to enable transmit of Search mode packet       
const char option_FSKRTTYEnable = OptionOff;      //set to OptionOn to enable transmit of FSKRTTY

#define option_SearchEnable_SUM (option_SearchEnable*1)
#define option_FSKRTTYEnable_SUM (option_FSKRTTYEnable*4)

const unsigned int Default_config1 = (option_SearchEnable_SUM + option_FSKRTTYEnable_SUM);
//const unsigned int Default_config1 = 0x05;      //Phew, the default config can always be set manually........
                                                  //0x05 would turn on transmit of search mode and FSKRTTY 

                                                   
//**************************************************************************************************
// 7) Memory settings - define the type of memory to use for non-Volatile storage.
//     
//**************************************************************************************************

//#define Memory_Library <FRAM_MB85RC16PNF.h>
//#define Memory_Library <FRAM_FM24CL64.h>
#define Memory_Library <EEPROMESP32_Memory.h>      //allows ESP32 Flash to emulate EEPROM

int16_t Memory_Address = 0x50;                     //default I2C address of MB85RC16PNF and FM24CL64 FRAM

//**************************************************************************************************
// 8) HAB Flight Settings
//**************************************************************************************************

char FlightID[] = "ESP32CAM1";                     //flight ID for HAB packet

const uint16_t SleepTimesecs = 10;                 //sleep time in seconds after each TX loop

const char ThisNode = '1';                         //tracker number for search packet

//**************************************************************************************************
// 9) Camera Settings
// The camera needs time to adjust the exposure, the first picture taken outdoors can be a whitewash
// it takes a few seconds for the camera to adjust, so take several pictures with a gap between 
//**************************************************************************************************

const uint8_t PicturesToTake = 3;                //number of pictures to take at each wakeup
const uint32_t PictureDelaymS = 1000;            //delay in mS between pictures

