/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 16/10/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  This is a program that uses the LR2021 to receive. The packet is assumed to be in ASCII and the count
  of packets sent is in the packet. The Serial monitor output can be re-directed to a named Serial port
  so that for instance the serial output can be sent to the Serial port on pins D6, D7 of a SEEED XIAO
  so that the output can be saved to an SD card logger. 
  
  The file defining the pins for the used microcontroller board such as SEEED_XIAO_Pins.h contains the pin 
  definitions used for the LoRa module and the definition of the Serial port where the Serial output is sent.

  The program can use 4 different set of modem parameters, LoRa or FLRC. The 4 ModemModes are defined in the
  Settings.h file. Define the mode as either LoRa, FLRC or none like this;

  //Configure groups of active settings here. 
  uint8_t ModemMode0 = FLRCModem;
  uint8_t ModemMode1 = LoRaModem;
  uint8_t ModemMode2 = NoModem;
  uint8_t ModemMode3 = NoModem;

  So the above defibes two modes, 0 and 1. Mode0 is FLRC and the detailed setting are also defined in Settings.h as thus;

  /Modem Parameters for Mode0 as FLRC
  const float Frequency_F0 = 868.0;                           //Frequency of transmissions in Mega hertz
  const uint16_t BandwidthBitRate_F0 = 260;                   //FLRC bandwidth\bitrate in Kbps
  const uint8_t CodingRate_F0 = RADIOLIB_LR2021_FLRC_CR_3_4;  //FLRC coding rate
  const uint32_t FLRCSyncword_F0 = 0x2D014B1D;                //Default FLRC syncword set automatically by library
  const uint8_t PreambleLength_F0 = 8;                        //Length of FLRC preamble in bits
  const int8_t TXPower_F0 = -9;                               //Transmit power in dBm
  const uint8_t DataShaping_F0 = RADIOLIB_SHAPING_0_5;        //Data shaping for FLRC

  Mode1 is LoRa and the detailed setting are defined in Settings.h;

  //Modem Parameters for ModemMode1 as LoRa
  const float Frequency_L1 = 868.0;      //Frequency of transmissions in Mega hertz
  const float Bandwidth_L1 = 1000.0;      //LoRa bandwidth in Khz
  const uint8_t SpreadingFactor_L1 = 5;  //LoRa spreading factor
  const uint8_t CodeRate_L1 = 5;         //LoRa coding rate
  const uint8_t LoRaSyncword_L1 = RADIOLIB_LR2021_LORA_SYNC_WORD_PRIVATE;
  const uint16_t PreambleLength_L1 = 8;  //Length LoRa preamble in symbols
  const int8_t TXPower_L1 = -9;          //Transmit power in dBm

  Then at power up or reset the SWITCH0 and SWITCH1 pins are read. If the SWITCH0 pin is left floating then SWITCH0 reads high and
  Mode1 is selected. If the SWITCH0 pin is shorted to GND then SWITCH0 reads low and Mode0 is selected.

  Sample serial port output;

  1 Transmitted > LoRa00017,Power,-9dBm,Length,10,Packets,17,Errors,0,Transmittime,3906uS
  1 Transmitted > LoRa00018,Power,-9dBm,Length,10,Packets,18,Errors,0,Transmittime,3906uS
  1 Transmitted > LoRa00019,Power,-9dBm,Length,10,Packets,19,Errors,0,Transmittime,3907uS

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*
  For default module settings, see the library wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration#lr2021---lora-modem

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// include the LR2021 library
#include <RadioLib.h>
#include "Settings.h"
#include "SEEED_XIAO_Pins.h"  //include if processor board is XIAO SAMD
//#include "Arduino_DUE_Pins.h"  //include if processor board is Arduino DUE

LR2021 radio = new Module(LORA_NSS, LORA_DIO, LORA_RESET, LORA_BUSY);

uint8_t RXbuffer[10];    //buffer to receive packet, transmitted packet is 10 char array
uint8_t RXPacketL;       //stores length of packet received
int8_t PacketRSSI;       //stores RSSI of received packet
int8_t PacketSNR;        //stores signal to noise ratio (SNR) of received packet
uint32_t RXpacketCount;  //keep count of packets received
uint32_t RXerrors;       //keep count of any packets with errors
int16_t state;           //reported state of radio. functions
uint8_t ModemType;       //device modem configured type, LoRa or FLRC
uint8_t SelectedMode;    //mode number selected by switches read at startup, normally 0 to 3


void loop() {

  state = radio.receive(RXbuffer, sizeof(RXbuffer), 60000);  //put receive timeout at 60 seconds

  PacketRSSI = radio.getRSSI();         //read the received packets RSSI value
  PacketSNR = radio.getSNR();           //read the received packets SNR value
  RXPacketL = radio.getPacketLength();  //read the length of packet received

  digitalWrite(LED1, LEDON);

#ifdef ENABLEBUZZER
  DIO_Buzzer(DIOBUZZER, 50, 0);  //DIO10 has buzzer connected, pulse 100 times with no delays
#endif

  if (state == RADIOLIB_ERR_NONE) {
    packet_is_OK();
  } else {
    packet_is_Error();
  }

  digitalWrite(LED1, LEDOFF);
}


void packet_is_OK() {
  RXpacketCount++;
  mySerial.print(SelectedMode);
  mySerial.print(F(" "));
  printElapsedTime();
  mySerial.print(F(","));
  mySerial.print((char*)RXbuffer);

  mySerial.print(F(",RSSI,"));
  mySerial.print(PacketRSSI);

  if (ModemType == LoRaModem)
  {
  mySerial.print(F("dBm,SNR,"));
  mySerial.print(PacketSNR);
  mySerial.print(F("dB,Length,"));
  }

  if (ModemType == FLRCModem)
  {
  mySerial.print(F("dBm,Length,"));
  } 

  mySerial.print(RXPacketL);
  mySerial.print(F(",Packets,"));
  mySerial.print(RXpacketCount);
  mySerial.print(F(",Errors,"));
  mySerial.print(RXerrors);
  mySerial.println();
}


void packet_is_Error() {

  mySerial.print(SelectedMode);
  mySerial.print(F(" "));
  printElapsedTime();

  if (state == RADIOLIB_ERR_RX_TIMEOUT) {
    mySerial.print(F(" RX Timeout"));
  } else {
    RXerrors++;
    mySerial.print(F(" PacketError"));
    mySerial.print(F(",RSSI,"));
    mySerial.print(PacketRSSI);
    mySerial.print(F("dBm,SNR,"));
    mySerial.print(PacketSNR);
    mySerial.print(F("dB,Length,"));
    mySerial.print(RXPacketL);
    mySerial.print(F(",Packets,"));
    mySerial.print(RXpacketCount);
    mySerial.print(F(",Errors,"));
    mySerial.print(RXerrors);
    mySerial.print(F(",State,"));
    mySerial.print(state);
    mySerial.flush();
#ifdef ENABLEBUZZER
    DIO_Buzzer(DIOBUZZER, 50, 0);  //DIO10 has buzzer connected, pulse 50 times with no delays
#endif
  }
  mySerial.println();
}

void led_Flash(uint16_t flashes, uint16_t delaymS) {
  uint16_t index;

  for (index = 1; index <= flashes; index++) {
    digitalWrite(LED1, LEDON);
    delay(delaymS);
    digitalWrite(LED1, LEDOFF);
    delay(delaymS);
  }
}


void printElapsedTime() {
  float seconds;
  seconds = millis() / 1000;
  mySerial.print(seconds, 0);
  mySerial.print(F("s"));
}


void print_Device_Settings() {

  mySerial.print(F("LORA_RESET pin "));
  mySerial.println(LORA_RESET);

  mySerial.print(F("LORA_DIO pin "));
  mySerial.println(LORA_DIO);

  mySerial.print(F("LORA_BUSY pin "));
  mySerial.println(LORA_BUSY);

  mySerial.print(F("LORA_NSS pin "));
  mySerial.println(LORA_NSS);

  mySerial.print(F("TCXOVoltage "));
  mySerial.print(TCXOVoltage, 1);
  mySerial.println(F("V"));

  mySerial.print(F("IRQDIONUM "));
  mySerial.println(IRQDIONUM);
}


void setup() {
  pinMode(SWITCH0, INPUT_PULLUP);
  pinMode(SWITCH1, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);     //initialize digital pin LED1 as an output.
  digitalWrite(LED1, HIGH);  //LED on
  led_Flash(10, 250);

  mySerial.begin(115200);
  if (!Serial) {};
  mySerial.println();
  mySerial.println();
  mySerial.println(__FILE__);

  SelectedMode = read_Mode_Switch();
  mySerial.print(F("SelectedMode "));
  mySerial.println(SelectedMode);

  set_Modem_Parameters(SelectedMode);  //setup modem for chosen LoRa or FLRC settings
  setup_Modem(ModemType);              //setup modem for LoRa or FLRC as selected

  print_Device_Settings();
  mySerial.println();
  mySerial.print(F("Listening for packets in Mode "));
  mySerial.println(SelectedMode);
}

bool check_Mode_Configured(uint8_t mode) {
  if ((mode == 0) && (ModemMode0 > 1)) {
    mySerial.println(F("Mode 0 not configured - defaulting to Mode 3"));
    SelectedMode = 3;
    return false;
  }

  if ((mode == 1) && (ModemMode1 > 1)) {
    mySerial.println(F("Mode 1 not configured - defaulting to Mode 3"));
    SelectedMode = 3;
    return false;
  }

  if ((mode == 2) && (ModemMode2 > 1)) {
    mySerial.println(F("Mode 2 not configured - defaulting to Mode 3"));
    SelectedMode = 3;
    return false;
  }

  if ((mode == 3) && (ModemMode3 > 1)) {
    mySerial.println(F("Mode 3 not configured - defaulting to Mode 3"));
    SelectedMode = 3;
    return false;
  }

  return true;
}



void setup_Modem(uint8_t ModemType) {

  // LR2021 allows to use any of the DIO pins as the interrupt
  // as an example, we set DIO11 to be the IRQ
  // this has to be done prior to calling begin()!
  radio.irqDioNum = IRQDIONUM;

  // also before calling begin(), correct crystal has to be selected
  // some LR2021 have a TCXO which needs 1.6V reference
  // set to 0 if your radio has an XTAL
  radio.tcxoVoltage = TCXOVoltage;

  if (ModemType == LoRaModem) {
    mySerial.print(F("Initializing Module for LoRa... "));
    ConfigLoRa_t config;
    config.frequency = Frequency;
    config.bandwidth = Bandwidth;
    config.spreadingFactor = SpreadingFactor;
    config.codingRate = CodeRate;
    config.syncWord = LoRaSyncword;
    config.power = TXPower;
    config.preambleLength = PreambleLength;
    int state = radio.begin(config);

    //Alternative begin, listed in API but now depracated apparently
    //begin (float freq = 868.0, float bw = 125.0, uint8_t sf = 7, uint8_t cr = 5, uint8_t syncWord = RADIOLIB_LR2021_LORA_SYNC_WORD_PRIVATE, int8_t power = 10, uint16_t preambleLength = 8, float tcxoVoltage = 1.60;
    //int state = radio.begin(Frequency, Bandwidth, SpreadingFactor, CodeRate, SyncWord, TXPower, PreambleLength, TCXOVoltage);
    //int state = radio.begin(868.0, 125.0, 7, 5, 0x12, 10, 8, 1.6);
  }

  if (ModemType == FLRCModem) {
    mySerial.print(F("Initializing Module for FLRC ... "));
    ConfigFLRC_t config;
    config.frequency = Frequency;
    config.bitRate = BandwidthBitRate;
    config.codingRate = CodingRate;
    config.power = TXPower;
    config.preambleLength = PreambleLength;
    config.dataShaping = DataShaping;
    state = radio.beginFLRC(config);

    //Alternative begin, listed in API but now depracated apparently
    //int16_t beginFLRC (float freq = 868.0, uint16_t br = 650, uint8_t cr = RADIOLIB_LR2021_FLRC_CR_2_3, int8_t pwr = 10, uint16_t preambleLength = 16, uint8_t dataShaping = RADIOLIB_SHAPING_0_5, float tcxoVoltage = 1.6)
    //example int state = radio.beginFLRC(868.0, 650, RADIOLIB_LR2021_FLRC_CR_2_3, TXpower, PreambleLength)
    //state = radio.beginFLRC(Frequency, BandwidthBitRate, CodingRate, 10, 32, DataShaping, TCXOVoltage);
  }

  if (state == RADIOLIB_ERR_NONE) {
    mySerial.println(F("success!"));
  } else {
    while (1) {
      led_Flash(5, 25);
      mySerial.print(F("No device responding, code "));
      mySerial.println(state);
    };
  }

  print_ModemSettings(ModemType, SelectedMode);
}



void set_Modem_Parameters(uint8_t ModeNumber) {

  if (ModeNumber == 0) {

    if (ModemMode0 == LoRaModem) {
      ModemType = LoRaModem;
      mySerial.println(F("Using LoRa Mode 0"));
      Frequency = Frequency_L0;              //frequency of transmissions
      Bandwidth = Bandwidth_L0;              //LoRa bandwidth
      SpreadingFactor = SpreadingFactor_L0;  //LoRa spreading factor
      CodeRate = CodeRate_L0;                //LoRa coding rate
      LoRaSyncword = LoRaSyncword_L0;        //LoRa syncword
      PreambleLength = PreambleLength_L0;    //LoRa preamble length in symbols
      TXPower = TXPower_L0;                  //Transmit power used when sending packets
    }

    if (ModemMode0 == FLRCModem) {
      ModemType = FLRCModem;
      mySerial.println(F("Using FLRC Mode 0"));
      Frequency = Frequency_F0;                //frequency of transmissions
      BandwidthBitRate = BandwidthBitRate_F0;  //FLRC bandwidth and bit rate
      CodingRate = CodingRate_F0;              //FLRC coding rate
      DataShaping = DataShaping_F0;            //FLRC datashaping
      FLRCSyncword = FLRCSyncword_F0;          //FLRC syncword
      PreambleLength = PreambleLength_F0;      //FLRC preamble length in bits
      TXPower = TXPower_F0;                    //Transmit power used when sending packets
    }
  }

  if (ModeNumber == 1) {

    if (ModemMode1 == LoRaModem) {
      ModemType = LoRaModem;
      mySerial.println(F("Using LoRa Mode 1"));
      Frequency = Frequency_L1;              //frequency of transmissions
      Bandwidth = Bandwidth_L1;              //LoRa bandwidth
      SpreadingFactor = SpreadingFactor_L1;  //LoRa spreading factor
      CodeRate = CodeRate_L1;                //LoRa coding rate
      LoRaSyncword = LoRaSyncword_L1;        //LoRa syncword
      PreambleLength = PreambleLength_L1;    //LoRa preamble length in symbols
      TXPower = TXPower_L1;                  //Transmit power used when sending packets
    }

    if (ModemMode1 == FLRCModem) {
      ModemType = FLRCModem;
      mySerial.println(F("Using FLRC Mode 1"));
      Frequency = Frequency_F1;                //frequency of transmissions
      BandwidthBitRate = BandwidthBitRate_F1;  //FLRC bandwidth and bit rate
      CodingRate = CodingRate_F1;              //FLRC coding rate
      DataShaping = DataShaping_F1;            //FLRC datashaping
      FLRCSyncword = FLRCSyncword_F1;          //FLRC syncword
      PreambleLength = PreambleLength_F1;      //FLRC preamble length in bits
      TXPower = TXPower_F1;                    //Transmit power used when sending packets
    }
  }

  if (ModeNumber == 2) {

    if (ModemMode2 == LoRaModem) {
      ModemType = LoRaModem;
      mySerial.println(F("Using LoRa Mode 2"));
      Frequency = Frequency_L2;              //frequency of transmissions
      Bandwidth = Bandwidth_L2;              //LoRa bandwidth
      SpreadingFactor = SpreadingFactor_L2;  //LoRa spreading factor
      CodeRate = CodeRate_L2;                //LoRa coding rate
      LoRaSyncword = LoRaSyncword_L2;        //LoRa syncword
      PreambleLength = PreambleLength_L2;    //LoRa preamble length in symbols
      TXPower = TXPower_L2;                  //Transmit power used when sending packets
    }

    if (ModemMode2 == FLRCModem) {
      ModemType = FLRCModem;
      mySerial.println(F("Using FLRC Mode 2"));
      Frequency = Frequency_F2;                //frequency of transmissions
      BandwidthBitRate = BandwidthBitRate_F2;  //FLRC bandwidth and bit rate
      CodingRate = CodingRate_F2;              //FLRC coding rate
      DataShaping = DataShaping_F2;            //FLRC datashaping
      FLRCSyncword = FLRCSyncword_F2;          //FLRC syncword
      PreambleLength = PreambleLength_F2;      //FLRC preamble length in bits
      TXPower = TXPower_F2;                    //Transmit power used when sending packets
    }
  }

  if (ModeNumber == 3) {

    if (ModemMode3 == LoRaModem) {
      ModemType = LoRaModem;
      mySerial.println(F("Using LoRa Mode 3"));
      Frequency = Frequency_L3;              //frequency of transmissions
      Bandwidth = Bandwidth_L3;              //LoRa bandwidth
      SpreadingFactor = SpreadingFactor_L3;  //LoRa spreading factor
      CodeRate = CodeRate_L3;                //LoRa coding rate
      LoRaSyncword = LoRaSyncword_L3;        //LoRa syncword
      PreambleLength = PreambleLength_L3;    //LoRa preamble length in symbols
      TXPower = TXPower_L3;                  //Transmit power used when sending packets
    }

    if (ModemMode3 == FLRCModem) {
      ModemType = FLRCModem;
      mySerial.println(F("Using FLRC Mode 3"));
      Frequency = Frequency_F3;                //frequency of transmissions
      BandwidthBitRate = BandwidthBitRate_F3;  //FLRC bandwidth and bit rate
      CodingRate = CodingRate_F3;              //FLRC coding rate
      DataShaping = DataShaping_F3;            //FLRC datashaping
      FLRCSyncword = FLRCSyncword_F3;          //FLRC syncword
      PreambleLength = PreambleLength_F3;      //FLRC preamble length in bits
      TXPower = TXPower_F3;                    //Transmit power used when sending packets
    }
  }
}


uint8_t read_Mode_Switch() {
  //note if mode switch not connected switch reads as 1
  uint8_t ModeNumber;

  if (digitalRead(SWITCH0)) {
    mySerial.println(F("SWITCH0 is HIGH"));
    ModeNumber = 1;
  } else {
    mySerial.println(F("SWITCH0 is LOW"));
    ModeNumber = 0;
  }

  if ((ModemMode2 == NoModem) && (ModemMode3 == NoModem)) {
    mySerial.println(F("SWITCH1 not read"));
    return ModeNumber;
  }


  if (digitalRead(SWITCH1)) {
    mySerial.println(F("SWITCH1 is HIGH"));
    ModeNumber = ModeNumber + 2;
  } else {
    mySerial.println(F("SWITCH1 is LOW"));
  }

  return ModeNumber;
}


void print_ModemSettings(uint8_t type, uint8_t ModeNumber) {

  bool VALIDCODINGRATE = false;
  bool VALIDDATASHAPING = false;

  mySerial.println();

  if (type == LoRaModem) {
    mySerial.print(F("LoRa Mode "));
    mySerial.println(ModeNumber);

    mySerial.print(F("Frequency "));
    mySerial.print(Frequency);
    mySerial.println(F("Mhz"));

    mySerial.print(F("Bandwidth "));
    mySerial.print(Bandwidth, 2);
    mySerial.println(F("Khz"));

    mySerial.print(F("SpreadingFactor "));

    mySerial.println(SpreadingFactor);

    mySerial.print(F("Coderate 4:"));
    mySerial.println(CodeRate);

    mySerial.print(F("LoRaSyncword 0x"));
    mySerial.println(LoRaSyncword, HEX);

    mySerial.print(F("TX Power "));
    mySerial.print(TXPower);
    mySerial.println(F("dBm"));

    //mySerial.print(F("TCXOVoltage "));
    //mySerial.print(TCXOVoltage, 1);
    //mySerial.println(F("V"));
  }

  if (type == FLRCModem) {
    mySerial.print(F("FLRC Mode "));
    mySerial.println(ModeNumber);
    mySerial.print(F("Frequency "));
    mySerial.print(Frequency);
    mySerial.println(F("Mhz"));

    mySerial.print(F("BandwidthBitRate "));
    mySerial.print(BandwidthBitRate);
    mySerial.println(F("Kbps"));

    mySerial.print(F("CodingRate "));

    if (CodingRate == RADIOLIB_LR2021_FLRC_CR_1_0) {
      mySerial.println(F("1_0"));
      VALIDCODINGRATE = true;
    }

    if (CodingRate == RADIOLIB_LR2021_FLRC_CR_3_4) {
      mySerial.println(F("3_4"));
      VALIDCODINGRATE = true;
    }

    if (CodingRate == RADIOLIB_LR2021_FLRC_CR_2_3) {
      mySerial.println(F("2_3"));
      VALIDCODINGRATE = true;
    }

    if (CodingRate == RADIOLIB_LR2021_FLRC_CR_1_2) {
      mySerial.println(F("1_2"));
      VALIDCODINGRATE = true;
    }

    if (!VALIDCODINGRATE) {
      mySerial.println(F("Error"));
    }

    mySerial.print(F("DataShaping "));

    if (DataShaping == RADIOLIB_SHAPING_NONE) {
      mySerial.println(F("NONE"));
      VALIDDATASHAPING = true;
    }

    if (DataShaping == RADIOLIB_SHAPING_0_3) {
      mySerial.println(F("0_3"));
      VALIDDATASHAPING = true;
    }

    if (DataShaping == RADIOLIB_SHAPING_0_5) {
      mySerial.println(F("0_5"));
      VALIDDATASHAPING = true;
    }

    if (DataShaping == RADIOLIB_SHAPING_0_7) {
      mySerial.println(F("0_7"));
      VALIDDATASHAPING = true;
    }

    if (DataShaping == RADIOLIB_SHAPING_1_0) {
      mySerial.println(F("1_0"));
      VALIDDATASHAPING = true;
    }

    if (!VALIDDATASHAPING) {
      mySerial.println(F("Error"));
    }

    //mySerial.print(F("FLRCSyncword 0x"));
    //mySerial.println(FLRCSyncword, HEX);

    mySerial.print(F("TXPower "));
    mySerial.print(TXPower);
    mySerial.println(F("dBm"));
  }
}


void DIO_Buzzer(uint8_t dio, uint16_t pulses, uint32_t delaytimeus) {

  uint16_t index;

  for (index = 0; index <= pulses; index++) {
    set_DIO(dio, 0);
    delayMicroseconds(delaytimeus);
    set_DIO(dio, 1);
    delayMicroseconds(delaytimeus);
  }
}


void set_DIO(uint8_t dionum, uint8_t level) {
  //can generate an approx 500hz clock signal

  digitalWrite(LORA_NSS, LOW);

  SPI.transfer(0x01);
  SPI.transfer(0x12);
  SPI.transfer(dionum);

  if (level == 0) {
    SPI.transfer(0x50);
  } else {
    SPI.transfer(0x60);
  }
  digitalWrite(LORA_NSS, HIGH);

  checkBusy();
}


void checkBusy() {
  uint8_t busy_timeout_cnt;
  busy_timeout_cnt = 0;

  while (digitalRead(LORA_BUSY)) {
    delay(1);
    busy_timeout_cnt++;

    if (busy_timeout_cnt > 10)  //wait 10mS for busy to complete
    {
      busy_timeout_cnt = 0;
      Serial.println(F("ERROR - Busy Timeout!"));
      break;
    }
  }
}