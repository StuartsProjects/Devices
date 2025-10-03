/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 02/10/25

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/
#define PRINT_ASCII  //enable to print ASCII of packet
//#define PRINT_HEX                             //enable to print HEX of packet
#define USE_DISPLAY  //enable OLED
#define USE_SD       //enable SD card for logging
//#define USE_BLUETOOTH                         //requires an ESP32 processor and version 3.0.0 and above of Expressif ESP32 Core for IDE.
//#define USE_BUZZER                            //on GPIO10, pin 3 of P3, connector nearest long edge of board, receiver only

//LoRa Modem Parameters
const float Frequency = 868.0;       //frequency of transmissions in Mega hertz
const float Bandwidth = 250;         //LoRa bandwidth in Kilo hertz
const uint8_t SpreadingFactor = 11;  //LoRa spreading factor
const uint8_t CodeRate = 5;          //LoRa coding rate 4:?
const uint8_t SyncWord = 0x2B;       //LoRa SyncWord = 0x12 or 0x34, Meshtastic SyncWord = 0x2B
const int8_t TXpower = 10;           //LoRa transmit power in dBm
const uint16_t PreambleLength = 16;  //length of packet preamble
bool CRCon = true;                   //set to true to use packets CRC
const float TCXOvolts = 3.0;         //voltage of TCXO
const uint16_t packet_delay = 500;   //mS delay between packets
const float CurrentLimit = 140.0;    //current limit during TX
