/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 11/10/26

  This programs is supplied as is, it is up to the user of the program to decide if the programs are
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/
/*
ToDo:
Implement ASCIILogSize, HEXLogSize
*/

//library functions
uint8_t read_Mode_Switch();                                  //read the switches at startup to choose modem settings to use
void set_Modem_Parameters(uint8_t modenum);                  //set the modem parameters according to parameters in the Settings.h file
bool setup_INA219();                                         //setup the I2C INA219 power monitor
bool setup_SD();                                             //setup the SD Card
uint8_t setup_LOG(char* buff);                               //setup the SD LogFile name to use
void print_ModemSettings(uint8_t type, uint8_t modenum);     //print the modem settings in use

void SDWrite_ModemSettings(uint8_t type, uint8_t modenum);   //write the modem settings in use to the SD log file
void SDWrite_HEXByte(uint8_t temp);                          //write HEX byte to SD card LogFile
void SDWrite_ASCIIPacket(uint8_t *buffer, uint8_t pktlength);        //write packet as ASCII to SD card LogFile
void SDWrite_HEXPacket(uint8_t *buffer, uint8_t pktlength);          //write packet as HEX bytes to SD card LogFile
void SDWrite_RXpacket_is_Error(uint8_t pktlength, uint16_t irqs);  //write packet with errors to SD card LogFile
void SDWrite_RXpacket_is_OK(uint8_t *buffer, uint8_t pktlength);                  //write packet with no errors to SD card LogFile
void RXpacket_is_Error(uint8_t *buffer, uint8_t rxpacketl, int8_t packetrssi, int8_t packetsnr, uint32_t rxpacketcount, uint32_t rxerrors, uint16_t irqs);          //process received packet with errors
void RXpacket_is_OK(uint8_t *buffer, uint8_t rxpacketl, int8_t packetrssi, int8_t packetsnr, uint32_t rxpacketcount, uint32_t rxerrors, uint16_t irqs);
void TXpacket_is_OK(uint8_t *buffer, uint8_t txpacketl, uint16_t irqs);                          //process transmitted packet with no errors
void TXpacket_is_Error(uint8_t txpacketl, uint16_t irqs);                       //process transmitted packet with errors
uint32_t returnLoRaBandwidth(uint8_t bw);     //function to return LoRa bandwidth in hertz   
uint16_t returnFLRCBandwidth(uint8_t bw);     //function to return FLRC bandwidth in hertz
void readRegisters(uint16_t address, uint8_t *buffer, uint16_t size);   //read SX1280 registers into buffer
uint8_t readRegister(uint16_t address);       //read single SX1280 register
void checkBusy();                             //check BUSY pin on LoRa device 


void TXpacket_is_Error(uint8_t txpacketl, uint16_t irqs) {
  //process transmitted packet with errors
  Serial.print(F(" SendError,"));
  Serial.print(F("Length,"));
  Serial.print(txpacketl);
  Serial.print(F(",IRQreg,0x"));
  Serial.print(irqs, HEX);
  delay(50);  //change LED flash so packet error visible
  digitalWrite(LED1, HIGH);
  delay(50);
  digitalWrite(LED1, LOW);
}


void TXpacket_is_OK(uint8_t *buffer, uint8_t txpacketl, uint16_t irqs) {
  //process transmitted packet with no errors
  uint8_t index, readchar, len;
  bool PACKETISASCII = true;

  for (index = 0; index <= (txpacketl - 1); index++) {
    readchar = buffer[index];
    if (((readchar > 0) && (readchar < 0x20)) || (readchar > 0x7F)) {
      PACKETISASCII = false;
    }
  }

  if (PACKETISASCII) {
    for (index = 0; index <= (txpacketl - 1); index++) {
      readchar = buffer[index];
      Serial.write((uint8_t)readchar);
    }
  } else {
    Serial.print(F("Not ASCII"));
    if (txpacketl > 8) {
      len = 8;
    } else {
      len = txpacketl;
    }

    for (index = 0; index < len; index++) {
      readchar = buffer[index];

      if (readchar < 0x10) {
        Serial.print(F("0"));
      }
      Serial.print(readchar, HEX);
      Serial.print(F(" "));
    }
  }
   
  Serial.print(F(" "));
  Serial.print(txpacketl);
  Serial.print(F(" Bytes SentOK"));
}


void RXpacket_is_OK(uint8_t *buffer, uint8_t rxpacketl, int8_t packetrssi, int8_t packetsnr, uint32_t rxpacketcount, uint32_t rxerrors, uint16_t irqs) {
  //process received packet with no errors
  uint8_t index, readchar, len;
  bool PACKETISASCII = true;

  for (index = 0; index <= (rxpacketl - 1); index++) {
    readchar = buffer[index];
    if (((readchar > 0) && (readchar < 0x20)) || (readchar > 0x7F)) {
      PACKETISASCII = false;
    }
  }

  if (PACKETISASCII) {
    for (index = 0; index <= (rxpacketl - 1); index++) {
      readchar = buffer[index];
      Serial.write((uint8_t)readchar);
    }
  } else {
    Serial.print(F("Not ASCII"));
    if (rxpacketl > 8) {
      len = 8;
    } else {
      len = rxpacketl;
    }

    for (index = 0; index < len; index++) {
      readchar = buffer[index];

      if (readchar < 0x10) {
        Serial.print(F("0"));
      }
      Serial.print(readchar, HEX);
      Serial.print(F(" "));
    }
  }

  Serial.print(F(" RSSI,"));
  Serial.print(packetrssi);
  Serial.print(F("dBm,"));

  if (ModemType == LoRaModem) {
    Serial.print(F("SNR,"));
    Serial.print(packetsnr);
    Serial.print(F("dB,"));
  }

  Serial.print(F("Length,"));
  Serial.print(rxpacketl);
  Serial.print(F(",Packets,"));
  Serial.print(rxpacketcount);
  Serial.print(F(",Errors,"));
  Serial.print(rxerrors);  //count of packet errors
}

void RXpacket_is_Error(uint8_t *buffer, uint8_t rxpacketl, int8_t packetrssi, int8_t packetsnr, uint32_t rxpacketcount, uint32_t rxerrors, uint16_t irqs)
{
  if (irqs & IRQ_RX_TIMEOUT) {
    Serial.print(F(" RXTimeout"));
  } else {
    Serial.print(F(" PacketError"));
    Serial.print(F(",RSSI,"));
    Serial.print(packetrssi);
    Serial.print(F("dBm,"));

    if (ModemType == LoRaModem) {
      Serial.print(F("SNR,"));
      Serial.print(packetsnr);
      Serial.print(F("dB,"));
    }

    Serial.print(F("Length,"));
    Serial.print(rxpacketl);  //get the real packet length
    Serial.print(F(",Packets,"));
    Serial.print(rxpacketcount);
    Serial.print(F(",Errors,"));
    Serial.print(rxerrors);
    Serial.print(F(",IRQreg,0x"));
    Serial.print(irqs, HEX);
  }
}


void SDWrite_RXpacket_is_OK(uint8_t *buffer, uint8_t pktlength) {

  if (PACKETISASCII) {
    SDWrite_ASCIIPacket(buffer, pktlength);
  } else {
    SDWrite_HEXPacket(buffer, pktlength);
  }
  LogFile.print(F(" RSSI,"));
  LogFile.print(PacketRSSI);
  LogFile.print(F("dBm,"));

  if (ModemType == LoRaModem) {
    LogFile.print(F("SNR,"));
    LogFile.print(PacketSNR);
    LogFile.print(F("dB,"));
  }

  LogFile.print(F("Length,"));
  LogFile.print(pktlength);
  LogFile.print(F(",Packets,"));
  LogFile.print(RXpacketCount);
  LogFile.print(F(",Errors,"));
  LogFile.print(RXErrors);
  LogFile.println();
  LogFile.flush();
}


void SDWrite_RXpacket_is_Error(uint8_t pktlength, uint16_t irqs) {

  if (irqs & IRQ_RX_TIMEOUT) {
    LogFile.print(F(" RXTimeout"));
  } else {
    LogFile.print(F(" PacketError"));
    LogFile.print(F(",RSSI,"));
    LogFile.print(PacketRSSI);
    LogFile.print(F("dBm,"));

    if (ModemType == LoRaModem) {
      LogFile.print(F("SNR,"));
      LogFile.print(PacketSNR);
      LogFile.print(F("dB,"));
    }

    LogFile.print(F("Length,"));
    LogFile.print(pktlength);
    LogFile.print(F(",Packets,"));
    LogFile.print(RXpacketCount);
    LogFile.print(F(",Errors,"));
    LogFile.print(RXErrors);
    LogFile.print(F(",IRQreg,0x"));
    LogFile.print(irqs, HEX);
    LogFile.println();
  }
  LogFile.flush();
}


void SDWrite_HEXPacket(uint8_t *buffer, uint8_t pktlength) {
  uint8_t index, readbyte;

  for (index = 0; index < pktlength; index++) {
    readbyte = buffer[index];
    SDWrite_HEXByte(readbyte);  //print the byte to SD
    LogFile.print(F(" "));
  }
  LogFile.flush();
}


void SDWrite_ASCIIPacket(uint8_t *buffer, uint8_t pktlength) {
  uint8_t index, readchar;

  for (index = 0; index <= (pktlength - 1); index++) {
    readchar = buffer[index];
    LogFile.write((uint8_t)readchar);
  }
  LogFile.flush();
}


void SDWrite_HEXByte(uint8_t temp) {
  if (temp < 0x10) {
    LogFile.print(F("0"));
  }
  LogFile.print(temp, HEX);
}


void SDWrite_ModemSettings(uint8_t type, uint8_t modenum) {

  bool VALIDCODINGRATE = false;
  bool VALIDBTMODSHAPING = false;

  uint8_t sftemp;

  if (type == LoRaModem) {
    LogFile.print(modenum);
    LogFile.println(F(" LoRaModem"));

    LogFile.print(F("Frequency "));
    LogFile.print(Frequency);
    LogFile.println(F("hz"));

    LogFile.print(F("Bandwidth "));
    LogFile.print(returnLoRaBandwidth(Bandwidth));
    LogFile.println(F("hz"));

    LogFile.print(F("Spreading factor "));

    sftemp = ((SpreadingFactor - 0x50) >> 4) + 5;

    LogFile.println(sftemp);

    LogFile.print(F("Coderate  CR4:"));
    LogFile.println((CodeRate) + 4);

    LogFile.print(F("TX Power "));
    LogFile.print(TXPower);
    LogFile.println(F("dBm"));
  }

  if (type == FLRCModem) {
    LogFile.print(modenum);
    LogFile.println(F(" FLRCModem"));

    LogFile.print(F("Frequency "));
    LogFile.print(Frequency);
    LogFile.println(F("hz"));

    LogFile.print(F("BandwidthBitRate "));
    LogFile.println(returnFLRCBandwidth(BandwidthBitRate));

    LogFile.print(F("CodingRate "));

    if (CodingRate == FLRC_CR_1_2) {
      LogFile.println(F("1_2"));
      VALIDCODINGRATE = true;
    }

    if (CodingRate == FLRC_CR_3_4) {
      LogFile.println(F("3_4"));
      VALIDCODINGRATE = true;
    }

    if (CodingRate == FLRC_CR_1_0) {
      LogFile.println(F("1_0"));
      VALIDCODINGRATE = true;
    }

    if (!VALIDCODINGRATE) {
      LogFile.println(F("Error"));
    }

    LogFile.print(F("BT "));

    if (BT == RADIO_MOD_SHAPING_BT_OFF) {
      LogFile.println(F("OFF"));
      VALIDBTMODSHAPING = true;
    }

    if (BT == RADIO_MOD_SHAPING_BT_1_0) {
      LogFile.println(F("1_0"));
      VALIDBTMODSHAPING = true;
    }

    if (BT == RADIO_MOD_SHAPING_BT_0_5) {
      LogFile.println(F("0_5"));
      VALIDBTMODSHAPING = true;
    }

    if (!VALIDBTMODSHAPING) {
      LogFile.println(F("Error"));
    }

    LogFile.print(F("FLRC_Syncword 0x"));
    LogFile.println(FLRC_Syncword, HEX);

    LogFile.print(F("TXPower "));
    LogFile.print(TXPower);
    LogFile.println(F("dBm"));

    LogFile.flush();
  }
}


void print_ModemSettings(uint8_t type, uint8_t modenum) {

  bool VALIDCODINGRATE = false;
  bool VALIDBTMODSHAPING = false;

  uint8_t sftemp;

  if (type == LoRaModem) {
    Serial.print(modenum);
    Serial.print(F(" LoRa Modem"));

    Serial.print(F("Frequency "));
    Serial.print(Frequency);
    Serial.println(F("hz"));

    Serial.print(F("Bandwidth "));
    Serial.print(returnLoRaBandwidth(Bandwidth));
    Serial.println(F("hz"));

    Serial.print(F("Spreading factor "));

    sftemp = ((SpreadingFactor - 0x50) >> 4) + 5;

    Serial.println(sftemp);

    Serial.print(F("Coderate  CR4:"));
    Serial.println((CodeRate) + 4);

    Serial.print(F("TX Power "));
    Serial.print(TXPower);
    Serial.println(F("dBm"));
  }

  if (type == FLRCModem) {
    Serial.print(modenum);
    Serial.println(F(" FLRC Modem"));
    Serial.print(F("Frequency "));
    Serial.print(Frequency);
    Serial.println(F("hz"));

    Serial.print(F("BandwidthBitRate "));
    Serial.println(returnFLRCBandwidth(BandwidthBitRate));

    Serial.print(F("CodingRate "));

    if (CodingRate == FLRC_CR_1_2) {
      Serial.println(F("1_2"));
      VALIDCODINGRATE = true;
    }

    if (CodingRate == FLRC_CR_3_4) {
      Serial.println(F("3_4"));
      VALIDCODINGRATE = true;
    }

    if (CodingRate == FLRC_CR_1_0) {
      Serial.println(F("1_0"));
      VALIDCODINGRATE = true;
    }

    if (!VALIDCODINGRATE) {
      Serial.println(F("Error"));
    }

    Serial.print(F("BT "));

    if (BT == RADIO_MOD_SHAPING_BT_OFF) {
      Serial.println(F("OFF"));
      VALIDBTMODSHAPING = true;
    }

    if (BT == RADIO_MOD_SHAPING_BT_1_0) {
      Serial.println(F("1_0"));
      VALIDBTMODSHAPING = true;
    }

    if (BT == RADIO_MOD_SHAPING_BT_0_5) {
      Serial.println(F("0_5"));
      VALIDBTMODSHAPING = true;
    }

    if (!VALIDBTMODSHAPING) {
      Serial.println(F("Error"));
    }

    Serial.print(F("FLRC_Syncword 0x"));
    Serial.println(FLRC_Syncword, HEX);

    Serial.print(F("TXPower "));
    Serial.print(TXPower);
    Serial.println(F("dBm"));
  }

  if (SDFound) {
    Serial.print(LogFileName);
  } else {
    Serial.println(F("No SD card"));
  }
}


uint8_t setup_LOG(char* buff) {
  //creates a new LogFileName

  uint16_t index;

  LogFile = SD.open("/");

  for (index = 1; index <= 9999; index++) {
    buff[4] = index / 1000 + '0';
    buff[5] = ((index % 1000) / 100) + '0';
    buff[6] = ((index % 100) / 10) + '0';
    buff[7] = index % 10 + '0';

    if (!SD.exists(buff)) {
      // only open a new file if it doesn't exist
      LogFile = SD.open(buff, FILE_WRITE);
      break;
    }
  }

  if (!LogFile) {
    return 0;
  }
  return index;  //return number of root created
}


bool setup_SD() {
  if (SD.begin(SDCS)) {
    return true;
  } else {
    return false;
  }
}

#ifdef USEINA219
bool setup_INA219() {
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments).
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  // ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  ina219.setCalibration_16V_400mA();

  if (ina219.begin())  //Initialize the INA219 t address 0x41.
  {
    voltsin = ina219.getBusVoltage_V();
    return true;
  } else {
    return false;
  }
}
#endif


void set_Modem_Parameters(uint8_t modenum) {

  Serial.print(F("set_Modem_Parameters() "));
  Serial.println(modenum);

  if (modenum == 0) {
    if (ModemMode_L0) {
      ModemType = LoRaModem;
      Serial.println(F("LoRa mode 0"));
      Frequency = Frequency_L0;              //frequency of transmissions
      Bandwidth = Bandwidth_L0;              //LoRa bandwidth
      SpreadingFactor = SpreadingFactor_L0;  //LoRa spreading factor
      CodeRate = CodeRate_L0;                //LoRa coding rate
      LORA_Syncword = LORA_Syncword_L0;      //LoRa syncword
	  PreambleLength = PreambleLength_L0;    //LoRa preamble length in symbols
      TXPower = TXPower_L0;                  //Transmit power used when sending packets
    }
    if (ModemMode_F0) {
      ModemType = FLRCModem;
      Serial.println(F("FLRC mode 0"));
      Frequency = Frequency_F0;                //frequency of transmissions
      BandwidthBitRate = BandwidthBitRate_F0;  //FLRC bandwidth and bit rate
      CodingRate = CodingRate_F0;              //FLRC coding rate
      BT = BT_F0;                              //FLRC BT
      FLRC_Syncword = Syncword_F0;             //FLRC syncword
      PreambleLength = PreambleLength_F0;      //FLRC preamble length in symbols
      TXPower = TXPower_F0;                    //Transmit power used when sending packets
    }
  }

  if (modenum == 1) {
    if (ModemMode_L1) {
      ModemType = LoRaModem;
      Serial.println(F("LoRa mode 1"));
      Frequency = Frequency_L1;              //frequency of transmissions
      Bandwidth = Bandwidth_L1;              //LoRa bandwidth
      SpreadingFactor = SpreadingFactor_L1;  //LoRa spreading factor
      CodeRate = CodeRate_L1;                //LoRa coding rate
      LORA_Syncword = LORA_Syncword_L1;      //LoRa syncword
	  PreambleLength = PreambleLength_L1;    //LoRa preamble length in symbols
      TXPower = TXPower_L1;                  //Transmit power used when sending packets
    }

    if (ModemMode_F1) {
      ModemType = FLRCModem;
      Serial.println(F("FLRC mode 1"));
      Frequency = Frequency_F1;                //frequency of transmissions
      BandwidthBitRate = BandwidthBitRate_F1;  //FLRC bandwidth and bit rate
      CodingRate = CodingRate_F1;              //FLRC coding rate
      BT = BT_F1;                              //FLRC BT
      FLRC_Syncword = Syncword_F1;             //FLRC syncword
      PreambleLength = PreambleLength_F1;    //FLRC preamble length in symbols
      TXPower = TXPower_F1;                    //Transmit power used when sending packets
    }
  }

  if (modenum == 2) {
    if (ModemMode_L2) {
      ModemType = LoRaModem;
      Serial.println(F("LoRa mode 2"));
      Frequency = Frequency_L2;              //frequency of transmissions
      Bandwidth = Bandwidth_L2;              //LoRa bandwidth
      SpreadingFactor = SpreadingFactor_L2;  //LoRa spreading factor
      CodeRate = CodeRate_L2;                //LoRa coding rate
      LORA_Syncword = LORA_Syncword_L2;      //LoRa syncword
	  PreambleLength = PreambleLength_L2;    //LoRa preamble length in symbols
      TXPower = TXPower_L2;                  //Transmit power used when sending packets
    }

    if (ModemMode_F2) {
      ModemType = FLRCModem;
      Serial.println(F("FLRC mode 2"));
      Frequency = Frequency_F2;                //frequency of transmissions
      BandwidthBitRate = BandwidthBitRate_F2;  //FLRC bandwidth and bit rate
      CodingRate = CodingRate_F2;              //FLRC coding rate
      BT = BT_F2;                              //FLRC BT
      FLRC_Syncword = Syncword_F2;             //FLRC syncword
      PreambleLength = PreambleLength_F2;    //FLRC preamble length in symbols
      TXPower = TXPower_F2;                    //Transmit power used when sending packets
    }
  }

  if (modenum == 3) {
    if (ModemMode_L3) {
      ModemType = LoRaModem;
      Serial.println(F("LoRa mode 3"));
      Frequency = Frequency_L3;              //frequency of transmissions
      Bandwidth = Bandwidth_L3;              //LoRa bandwidth
      SpreadingFactor = SpreadingFactor_L3;  //LoRa spreading factor
      CodeRate = CodeRate_L3;                //LoRa coding rate
      LORA_Syncword = LORA_Syncword_L3;      //LoRa syncword
	  PreambleLength = PreambleLength_L3;    //LoRa preamble length in symbols
      TXPower = TXPower_L3;                  //Transmit power used when sending packets
    }

    if (ModemMode_F3) {
      ModemType = FLRCModem;
      Serial.println(F("FLRC mode 3"));
      Frequency = Frequency_F3;                //frequency of transmissions
      BandwidthBitRate = BandwidthBitRate_F3;  //FLRC bandwidth and bit rate
      CodingRate = CodingRate_F3;              //FLRC coding rate
      BT = BT_F3;                              //FLRC BT
      FLRC_Syncword = Syncword_F3;           //FLRC syncword
      PreambleLength = PreambleLength_F3;      //FLRC preamble length in symbols
      TXPower = TXPower_F3;                    //Transmit power used when sending packets
    }
  }
}


uint8_t read_Mode_Switch() {
  //note if mode switch not connected switch reads as 1
  uint8_t modenumber;

  if (digitalRead(SWITCH0)) {
    Serial.println(F("modenumber 1"));
    modenumber = 1;
  } else {
    Serial.println(F("modenumber 0"));
    modenumber = 0;
  }

  Serial.print(F("Selected mode is "));
  Serial.println(modenumber);

  return modenumber;
}


uint32_t returnLoRaBandwidth(uint8_t bw) {
  switch (bw) {
    case LORA_BW_0200:
      return 200000;

    case LORA_BW_0400:
      return 400000;

    case LORA_BW_0800:
      return 800000;

    case LORA_BW_1600:
      return 16000000;

    default:
      return 0;
  }
}


uint16_t returnFLRCBandwidth(uint8_t bw) {
  switch (bw) {
    case FLRC_BR_1_300_BW_1_2:
      return 300;
      break;

    case FLRC_BR_1_000_BW_1_2:
      return 1000;

    case FLRC_BR_0_650_BW_0_6:
      return 650;

    case FLRC_BR_0_520_BW_0_6:
      return 520;

    case FLRC_BR_0_325_BW_0_3:
      return 325;

    case FLRC_BR_0_260_BW_0_3:
      return 260;

    default:
      return 0;
  }
}


void readRegisters(uint16_t address, uint8_t *buffer, uint16_t size)
{
  uint16_t index;
  uint8_t addr_l, addr_h;

  addr_h = address >> 8;
  addr_l = address & 0x00FF;
  checkBusy();

  digitalWrite(LORA_NSS, LOW);
  SPI.transfer(0x19);                 //read register command
  SPI.transfer(addr_h);               //MSB
  SPI.transfer(addr_l);               //LSB
  SPI.transfer(0xFF);
  for (index = 0; index < size; index++)
  {
    *(buffer + index) = SPI.transfer(0xFF);
  }

  digitalWrite(LORA_NSS, HIGH);
}


uint8_t readRegister(uint16_t address)
{
  uint8_t data;

  readRegisters(address, &data, 1);
  return data;
}


void checkBusy()
{
  uint32_t startmS = millis();

  do
  {
    if ( ((uint32_t) (millis() - startmS) > 9))   //wait 10mS for busy to complete
    {
      Serial.println(F("ERROR - Busy Timeout!"));
      break;
    }
  } while (digitalRead(LORA_BUSY));
}



