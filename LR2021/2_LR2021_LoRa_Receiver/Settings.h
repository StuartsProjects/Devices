/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 16/10/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//LoRa Modem Parameters
const float Frequency = 868.0;      //Frequency of transmissions in Mega hertz
const float Bandwidth = 125.0;      //LoRa bandwidth in Khz
const uint8_t SpreadingFactor = 7;  //LoRa spreading factor
const uint8_t CodeRate = 5;         //LoRa coding rate
const uint8_t LoRaSyncWord = RADIOLIB_LR2021_LORA_SYNC_WORD_PRIVATE;
const uint16_t PreambleLength = 8;  //Length of LoRa preamble in symbols
const int8_t TXPower = 10;          //Transmit power in dBm
const float TCXOVoltage = 1.6;      //Voltage for TCXO oscillator, 1.6V is standard for LR2021
const uint8_t IRQDIONUM = 11;       //DIO pin on module connected to LORA_DIO

/*
Allowable vales for parameters, see LR2021_commands.h in library

Values for bandwidth in Khz, see table 9.3 in datasheet;
1000    RADIOLIB_LR2021_LORA_BW_1000
812     RADIOLIB_LR2021_LORA_BW_812
500     RADIOLIB_LR2021_LORA_BW_500
406     RADIOLIB_LR2021_LORA_BW_406
250     RADIOLIB_LR2021_LORA_BW_250
203     RADIOLIB_LR2021_LORA_BW_203
125     RADIOLIB_LR2021_LORA_BW_125
101     RADIOLIB_LR2021_LORA_BW_101
83.34   RADIOLIB_LR2021_LORA_BW_83
62.5    RADIOLIB_LR2021_LORA_BW_62
41.67   RADIOLIB_LR2021_LORA_BW_41
31.25   RADIOLIB_LR2021_LORA_BW_31

Values for Spreading Factor
5 = SF5
6 = SF6
7 = SF7
8 = SF8
9 = SF9
10 = SF10
11 = SF11
12 = SF12

Values for coderate
5 = 4:5  RADIOLIB_LR2021_LORA_CR_4_5 
6 = 4:6  RADIOLIB_LR2021_LORA_CR_4_6 
7 = 4:5  RADIOLIB_LR2021_LORA_CR_4_7 
8 = 4:8  RADIOLIB_LR2021_LORA_CR_4_8 

Values for LoRaSyncWord, best to stick to the two standard values;
0x12 = RADIOLIB_LR2021_LORA_SYNC_WORD_PRIVATE
0x34 = RADIOLIB_LR2021_LORA_SYNC_WORD_LORAWAN

Values for TXpower in dBm
-9 to +22  for SubG RF
-19.5 to +12 for 2.4Ghz RF

Values for preamble, 8 is standard value less than this might not work
0 to 65535

Values for TCXovoltage, defaults to 1.6V on LR2021
Values for SX126X can be 1.6, 1.7, 1.8, 2.2, 2.4, 2.7, 3.0, 3.3 

*/
