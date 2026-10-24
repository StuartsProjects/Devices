/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 16/10/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


//FLRC Modem Parameters
const float Frequency = 868.0;                           //Frequency of transmissions in Mega hertz
const uint16_t BandwidthBitRate = 650;                   //FLRC bandwidth\bitrate in Kbps
const uint8_t CodingRate = RADIOLIB_LR2021_FLRC_CR_3_4;  //FLRC coding rate
const uint16_t PreambleLength = 8;                       //Length of FLRC preamble in bits
const int8_t TXPower = 10;                               //Transmit power in dBm
const uint8_t DataShaping = RADIOLIB_SHAPING_0_5;        //FLRC data shaping
const float TCXOVoltage = 1.6;                           //Voltage for TCXO oscillator, 1.6V is standard for LR2021
const uint8_t IRQDIONUM = 11;                            //DIO pin on module connected to LORA_DIO

/*
Allowable vales for parameters, see LR2021_commands.h in library

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

Values for TXpower in dBm
-9.5 to +22  for SubG RF
-18 to +12 for 2.4Ghz

Values for preamble, 8 is standard value less than this might not work
0 to 65535

Values for TCXovoltage, defaults to 1.6V on LR2021
Values for SX126X can be 1.6, 1.7, 1.8, 2.2, 2.4, 2.7, 3.0, 3.3 
*/