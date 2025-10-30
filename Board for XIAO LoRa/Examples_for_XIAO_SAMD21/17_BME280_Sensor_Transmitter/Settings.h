/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 18/10/22
  
  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

// *******  Setup hardware pin definitions here ! ***************

#define NSS A3                                  //select pin on LoRa device, matching D3 pins not recognised ?
#define NRESET A2                               //reset pin on LoRa device
#define DIO0 A1                                 //DIO0 pin on LoRa device, used for sensing RX and TX done 
#define LED1 13                                 //on board LED, yellow    
#define ADMultiplier 1.628                      //adjustment to convert AD value read into mV of battery voltage 
#define SupplyAD A0                             //Resitor divider for battery voltage read connected here 

#define LORA_DEVICE DEVICE_SX1278               //this is the device we are using

// *******  Setup LoRa Parameters Here ! ***************

//LoRa Modem Parameters
const uint32_t Frequency = 434000000;           //frequency of transmissions in hertz
const uint32_t Offset = 0;                      //offset frequency for calibration purposes

const uint8_t Bandwidth = LORA_BW_125;          //LoRa bandwidth
const uint8_t SpreadingFactor = LORA_SF7;       //LoRa spreading factor
const uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting
const int8_t TXpower = 10;                      //LoRa transmit power in dBm

const uint16_t NetworkID = 0x3210;              //NetworkID identifies this connection, needs to match value in receiver
const uint8_t ThisNode = 49;                    //source node 49 = '1'
const uint16_t TXattempts = 5;                  //number of times to transmit packet and wait for an acknowledge 
const uint32_t SleepSeconds = 60;               //number of seconds spent asleep
const uint32_t ACKtimeout = 1000;               //Acknowledge timeout in mS                      
const uint32_t TXtimeout = 5000;                //transmit timeout in mS. If 0 return from transmit function after send.  

const uint8_t BME280_ADDRESS = 0x76;            //I2C bus address of BME280 
const uint8_t BME280_REGISTER_CONTROL = 0xF4;   //BME280 register number for power control
