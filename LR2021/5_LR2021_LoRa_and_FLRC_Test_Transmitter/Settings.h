/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 21/10/26

  These programs are supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//LoRa Modem Parameters
float Frequency;          //Frequency of transmissions in Mega hertz
float Bandwidth;          //LoRa bandwidth in Khz
uint8_t SpreadingFactor;  //LoRa spreading factor
uint8_t CodeRate;         //LoRa coding rate
uint8_t LoRaSyncword;     //LoRa syncword
uint16_t PreambleLength;  //Length of LoRa preamble in symbols
int8_t TXPower;           //Transmit power in dBm

//FLRC Modem Parameters
uint16_t BandwidthBitRate;  //FLRC bandwidth\bitrate in Kbps
uint8_t CodingRate;         //FLRC coding rate
uint32_t FLRCSyncword;      //FLRC syncword
uint8_t DataShaping;        //FLRC data shaping

const float TCXOVoltage = 1.6;  //Voltage for TCXO oscillator, 1.6V is standard for LR2021
const uint8_t IRQDIONUM = 11;   //DIO pin on module connected to LORA_DIO

const uint8_t LoRaModem = 0;  //Modem type definition
const uint8_t FLRCModem = 1;  //Modem type definition
const uint8_t NoModem = 2;    //Modem type definition

//Configure groups of active settings here. 
uint8_t ModemMode0 = FLRCModem;
uint8_t ModemMode1 = LoRaModem;
uint8_t ModemMode2 = NoModem;
uint8_t ModemMode3 = NoModem;


/************************************************************************************
LoRa mode settings
*************************************************************************************/

//Modem Parameters for ModemMode0 as LoRa
const float Frequency_L0 = 868.0;      //Frequency of transmissions in Mega hertz
const float Bandwidth_L0 = 125.0;      //LoRa bandwidth in Khz
const uint8_t SpreadingFactor_L0 = 7;  //LoRa spreading factor
const uint8_t CodeRate_L0 = 5;         //LoRa coding rate
const uint8_t LoRaSyncword_L0 = RADIOLIB_LR2021_LORA_SYNC_WORD_PRIVATE;
const uint16_t PreambleLength_L0 = 8;  //Length of LoRa preamble in symbols
const int8_t TXPower_L0 = 10;          //Transmit power in dBm

//Modem Parameters for ModemMode1 as LoRa
const float Frequency_L1 = 868.0;      //Frequency of transmissions in Mega hertz
const float Bandwidth_L1 = 1000.0;      //LoRa bandwidth in Khz
const uint8_t SpreadingFactor_L1 = 5;  //LoRa spreading factor
const uint8_t CodeRate_L1 = 5;         //LoRa coding rate
const uint8_t LoRaSyncword_L1 = RADIOLIB_LR2021_LORA_SYNC_WORD_PRIVATE;
const uint16_t PreambleLength_L1 = 8;  //Length LoRa preamble in symbols
const int8_t TXPower_L1 = -9;          //Transmit power in dBm

//Modem Parameters for ModemMode2 as LoRa
const float Frequency_L2 = 868.0;      //Frequency of transmissions in Mega hertz
const float Bandwidth_L2 = 125.0;      //LoRa bandwidth in Khz
const uint8_t SpreadingFactor_L2 = 7;  //LoRa spreading factor
const uint8_t CodeRate_L2 = 5;         //LoRa coding rate
const uint8_t LoRaSyncword_L2 = RADIOLIB_LR2021_LORA_SYNC_WORD_PRIVATE;
const uint16_t PreambleLength_L2 = 8;  //Length of LoRa preamble in symbols
const int8_t TXPower_L2 = 10;          //Transmit power in dBm

//Modem Parameters for ModemMode3 as LoRa
const float Frequency_L3 = 868.0;      //Frequency of transmissions in Mega hertz
const float Bandwidth_L3 = 125;       //LoRa bandwidth in Khz
const uint8_t SpreadingFactor_L3 = 7;  //LoRa spreading factor
const uint8_t CodeRate_L3 = 5;         //LoRa coding rate
const uint8_t LoRaSyncword_L3 = RADIOLIB_LR2021_LORA_SYNC_WORD_PRIVATE;
const uint16_t PreambleLength_L3 = 8;  //Length of LoRa preamble in symbols
const int8_t TXPower_L3 = 10;          //Transmit power in dBm

/************************************************************************************
FLRC mode settings
*************************************************************************************/

//Modem Parameters for Mode0 as FLRC
const float Frequency_F0 = 868.0;                           //Frequency of transmissions in Mega hertz
const uint16_t BandwidthBitRate_F0 = 260;                   //FLRC bandwidth\bitrate in Kbps
const uint8_t CodingRate_F0 = RADIOLIB_LR2021_FLRC_CR_3_4;  //FLRC coding rate
const uint32_t FLRCSyncword_F0 = 0x2D014B1D;                //Default FLRC syncword set automatically by library
const uint8_t PreambleLength_F0 = 8;                        //Length of FLRC preamble in bits
const int8_t TXPower_F0 = -9;                               //Transmit power in dBm
const uint8_t DataShaping_F0 = RADIOLIB_SHAPING_0_5;        //Data shaping for FLRC

//Modem Parameters for Mode1 as FLRC
const float Frequency_F1 = 868.0;                           //Frequency of transmissions in Mega hertz
const uint16_t BandwidthBitRate_F1 = 650;                   //FLRC bandwidth\bitrate in Kbps
const uint8_t CodingRate_F1 = RADIOLIB_LR2021_FLRC_CR_3_4;  //FLRC coding rate
const uint32_t FLRCSyncword_F1 = 0x2D014B1D;                //Default FLRC syncword set automatically by library
const uint8_t PreambleLength_F1 = 8;                        //Length of FLRC preamble in bits
const int8_t TXPower_F1 = 10;                               //Transmit power in dBm
const uint8_t DataShaping_F1 = RADIOLIB_SHAPING_0_5;        //Data shaping for FLRC

//Modem Parameters for Mode2 as FLRC
const float Frequency_F2 = 868.0;                           //Frequency of transmissions in Mega hertz
const uint16_t BandwidthBitRate_F2 = 650;                   //FLRC bandwidth\bitrate in Kbps
const uint8_t CodingRate_F2 = RADIOLIB_LR2021_FLRC_CR_3_4;  //FLRC coding rate
const uint32_t FLRCSyncword_F2 = 0x2D014B1D;                //Default FLRC syncword set automatically by library
const uint8_t PreambleLength_F2 = 8;                        //Length of FLRC preamble in bits
const int8_t TXPower_F2 = 10;                               //Transmit power in dBm
const uint8_t DataShaping_F2 = RADIOLIB_SHAPING_0_5;        //Data shaping for FLRC

//Modem Parameters for Mode3 as FLRC
const float Frequency_F3 = 868.0;                           //Rrequency of transmissions in Mega hertz
const uint16_t BandwidthBitRate_F3 = 650;                   //FLRC bandwidth\bitrate in Kbps
const uint8_t CodingRate_F3 = RADIOLIB_LR2021_FLRC_CR_3_4;  //FLRC coding rate
const uint32_t FLRCSyncword_F3 = 0x2D014B1D;                //Default FLRC syncword set automatically by library
const uint8_t PreambleLength_F3 = 8;                        //Length of FLRC preamble in bits
const int8_t TXPower_F3 = 10;                               //Transmit power in dBm
const uint8_t DataShaping_F3 = RADIOLIB_SHAPING_0_5;        //Data shaping for FLRC


/************************************************************************************
Allowable vales for LoRa parameters, see LR2021_commands.h in library
*************************************************************************************/

/*
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

Values for syncword, best to stick to the two standard values;
0x12 = RADIOLIB_LR2021_LORA_SYNC_WORD_PRIVATE
0x34 = RADIOLIB_LR2021_LORA_SYNC_WORD_LORAWAN

Values for TXPower in dBm
-9 to +22  for SubG RF
-19.5 to +12 for 2.4Ghz RF

Values for preamble, 8 is standard value less than this might not work
0 to 65535

Values for TCXovoltage, defaults to 1.6V on LR2021
Values for SX126X can be 1.6, 1.7, 1.8, 2.2, 2.4, 2.7, 3.0, 3.3 

/************************************************************************************
Allowable vales for FLRC parameters, see LR2021_commands.h in library
*************************************************************************************/

/*
Values for BandwidthBitRate in kbps
2600 = RADIOLIB_LR2021_FLRC_BR_2600                           (0x00UL << 0)   //  7     0     bitrate/bandwidth: 2600 kbps, 2666 kHz
2080 = RADIOLIB_LR2021_FLRC_BR_2080                           (0x01UL << 0)   //  7     0                        2080 kbps, 2222 kHz
1300 = RADIOLIB_LR2021_FLRC_BR_1300                           (0x02UL << 0)   //  7     0                        1300 kbps, 1333 kHz
1040 = RADIOLIB_LR2021_FLRC_BR_1040                           (0x03UL << 0)   //  7     0                        1040 kbps, 1333 kHz
650 = RADIOLIB_LR2021_FLRC_BR_650                             (0x04UL << 0)   //  7     0                        650 kbps, 888 kHz
520 = RADIOLIB_LR2021_FLRC_BR_520                             (0x05UL << 0)   //  7     0                        520 kbps, 769 kHz
325 = RADIOLIB_LR2021_FLRC_BR_325                             (0x06UL << 0)   //  7     0                        325 kbps, 444 kHz
260 = RADIOLIB_LR2021_FLRC_BR_260                             (0x07UL << 0)   //  7     0                        260 kbps, 444 kHz

Values for CodingRate
RADIOLIB_LR2021_FLRC_CR_1_2  (avoid - lots of errors)   (0x00UL << 0)   //  7     0     coding rate: 1/2
RADIOLIB_LR2021_FLRC_CR_3_4                             (0x01UL << 0)   //  7     0                  3/4
RADIOLIB_LR2021_FLRC_CR_1_0                             (0x02UL << 0)   //  7     0                  1 (uncoded)
RADIOLIB_LR2021_FLRC_CR_2_3  (avoid - lots of errors)   (0x03UL << 0)   //  7     0                  2/3

Values for DataShaping
RADIOLIB_SHAPING_0_3
RADIOLIB_SHAPING_0_5
RADIOLIB_SHAPING_0_7
RADIOLIB_SHAPING_1_0
RADIOLIB_SHAPING_NONE

Values for TXPower in dBm
-9 to +22  for SubG RF
-19.5 to +12 for 2.4Ghz RF

Values for preamble bits; 4 to 32 bits, see section 18.4.2 (0x0249) in datasheet, allowable values;
4,8,12,16,20,24,28,32

Values for TCXovoltage, defaults to 1.6V on LR2021
Values for SX126X can be 1.6, 1.7, 1.8, 2.2, 2.4, 2.7, 3.0, 3.3 
*/