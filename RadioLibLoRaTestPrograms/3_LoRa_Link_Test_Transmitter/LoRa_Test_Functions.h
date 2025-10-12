/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 02/10/25

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

#include <Wire.h>
#include <SD.h>
File logFile;

//Serial monitor print functions
void print_packet_TX_detail(int16_t lorastate);
void print_packet_RX_detail(int16_t lorastate);
void printHexByte(uint8_t num);
//void print_PowerCount_RX();

//OLED display functions
void init_Display();
void display_packet_RX_detail(int16_t lorastate);
void display_packet_TX_detail(int16_t lorastate);
void display_LoRa_error(int16_t lorastate);
void display_Details();
void display_packet_LinkRX_detail(int16_t lorastate);
void display_packet_LinkTX_detail(int16_t lorastate);

//SD card functions
uint16_t setup_SDLog(char *filename);
void log_Setup_SD(char *title);
void log_packet_RX_SD(int16_t lorastate);
void logHexByte(uint8_t num);
//void log_PowerCount_RX_SD();

//************************************************************************
//Start Serial monitor print functions
//************************************************************************

void print_packet_TX_detail(int16_t lorastate) {
  uint8_t index;

  if (lorastate != 0) {
    Serial.print(F("TXError,"));
    Serial.print(lorastate);
  } else {
#ifdef PRINT_ASCII
    for (index = 0; index < TXPacketL; index++) {
      if (TXbuff[index] > 0)  //dont print null characters
      {
        Serial.write(TXbuff[index]);
      }
    }
    Serial.print(F(","));
#endif
  }

  Serial.print(F("TXBytes,"));
  Serial.print(TXPacketL);

  Serial.print(F(",TXOK,"));
  Serial.print(TXPacketsOK);

  Serial.print(F(",Errors,"));
  Serial.print(PacketErrors);

#ifdef PRINT_HEX
  Serial.print(F(",HEX"));
  for (index = 0; index < TXPacketL; index++) {
    Serial.print(",");
    printHexByte(TXbuff[index]);
  }
#endif
}


void print_packet_RX_detail(int16_t lorastate) {
  uint8_t index;

  if (lorastate != 0) {
    Serial.print(F("RXError,"));
    Serial.print(lorastate);
    Serial.print(F(","));
  } else {
#ifdef PRINT_ASCII
    for (index = 0; index < RXPacketL; index++) {
      if (RXbuff[index] > 0)  //dont print null characters
      {
        Serial.write(RXbuff[index]);
      }
    }
    Serial.print(F(","));
#endif
  }

  Serial.print(F("RXBytes,"));
  Serial.print(RXPacketL);

  Serial.print(F(",RSSI,"));
  Serial.print(PacketRSSI, 0);
  Serial.print(F("dBm"));

  Serial.print(F(",SNR,"));
  Serial.print(PacketSNR);
  Serial.print(F("dB"));

  Serial.print(F(",RXOK,"));
  Serial.print(RXPacketsOK);

  Serial.print(F(",Errors,"));
  Serial.print(PacketErrors);

#ifdef PRINT_HEX
  Serial.print(F(",HEX"));
  for (index = 0; index < RXPacketL; index++) {
    Serial.print(",");
    printHexByte(RXbuff[index]);
  }
#endif
}


void printHexByte(uint8_t num) {
  if (num < 0x10) {
    Serial.print("0");
  }

  Serial.print(num, HEX);
}

void print_Details() {
  Serial.print(F("Device "));
  Serial.println(selecteddevice);

  Serial.print(F("Frequency "));
  Serial.print(Frequency, 6);
  Serial.println(F("Mhz"));

  Serial.print(F("Bandwidth "));
  Serial.print(Bandwidth, 1);
  Serial.println(F("Khz"));

  Serial.print(F("SpreadingFactor "));
  Serial.println(SpreadingFactor);

  Serial.print(F("CodeRate 4:"));
  Serial.println(CodeRate);

  Serial.print(F("SyncWord 0x"));
  Serial.println(SyncWord, HEX);

  Serial.print(F("TXpower "));
  Serial.print(TXpower);
  Serial.println(F("dBm"));

  Serial.print(F("CurrentLimit "));
  Serial.print(CurrentLimit, 0);
  Serial.println(F("mA"));

  Serial.print(F("PreambleLength "));
  Serial.println(PreambleLength);
}

//************************************************************************
//End Start Serial monitor print functions
//************************************************************************

//************************************************************************
//Start OLED display functions
//************************************************************************

#ifdef USE_DISPLAY

void init_Display(uint8_t oledaddress) {

#ifdef USE_ESP32
  Wire.begin(I2C_SDA, I2C_SCL);
#else
  Wire.begin();
#endif

  Wire.beginTransmission(oledaddress);
  int16_t error = Wire.endTransmission();

  if (error == 0) {
    Serial.println("OLED found");
    OLED_FOUND = true;
    disp.begin();
    disp.clearBuffer();
    disp.setFont(u8g2_font_pxplusibmvga8_mr);
    disp.setCursor(8, 15);

    if (LORA_FOUND) {
      disp.print("LoRa OK");
      disp.setCursor(8, 29);
      disp.print(selecteddevice);
      disp.setCursor(8, 43);
      disp.print(F("SyncWord 0x"));
      disp.println(SyncWord, HEX);
    } else {
      disp.print("LoRa fail");
    }

    disp.sendBuffer();
  } else {
    Serial.println("OLED not found.");
    OLED_FOUND = false;
  }
}

void display_packet_RX_detail(int16_t lorastate) {
  disp.clearBuffer();
  disp.setFont(u8g2_font_pxplusibmvga8_mr);
  disp.setCursor(8, 15);

  if (lorastate != 0) {
    disp.print("Error ");
    disp.print(lorastate);
  } else {
//disp.print("RX ");
#ifdef PRINT_ASCII
    for (uint8_t index = 0; index < RXPacketL; index++) {
      if (RXbuff[index] > 0)  //dont display null characters
      {
        disp.write(RXbuff[index]);
      }
    }
#endif
  }

  disp.setCursor(8, 29);
  disp.print("RSSI ");
  disp.print(PacketRSSI, 0);
  disp.print("dBm");

  disp.setCursor(8, 43);
  disp.print("SNR  ");
  disp.print(PacketSNR, 2);
  disp.print("dB");

  disp.setCursor(7, 57);
  disp.print("Pks ");
  disp.print(RXPacketsOK);

  disp.setCursor(85, 57);
  disp.print(Voltage, 2);
  disp.print("v");

  disp.sendBuffer();
}


void display_packet_TX_detail(int16_t lorastate) {
  disp.clearBuffer();
  disp.setFont(u8g2_font_pxplusibmvga8_mr);
  disp.setCursor(8, 15);

  if (lorastate != 0) {
    disp.print("Error ");
    disp.print(lorastate);
  } else {
//disp.print("TX ");
#ifdef PRINT_ASCII
    for (uint8_t index = 0; index < TXPacketL; index++) {
      if (TXbuff[index] > 0)  //dont display null characters
      {
        disp.write(TXbuff[index]);
      }
    }
#endif
  }

  disp.setCursor(7, 57);
  disp.print("Pks ");
  disp.print(TXPacketsOK);

  disp.setCursor(85, 57);
  disp.print(Voltage, 2);
  disp.print("v");

  disp.sendBuffer();
}


void display_LoRa_error(int16_t lorastate) {
  disp.clearBuffer();
  disp.setFont(u8g2_font_pxplusibmvga8_mr);
  disp.setCursor(8, 15);

  if (lorastate != 0) {
    disp.print("LoRa Error ");
    disp.print(lorastate);
  }
  disp.sendBuffer();
}


void display_Details() {
  disp.clearBuffer();
  disp.setFont(u8g2_font_5x7_mf);

  disp.setCursor(0, 7);
  disp.print(selecteddevice);

  disp.setCursor(0, 15);
  disp.print("Freq ");
  disp.print(Frequency, 6);

  disp.setCursor(0, 23);
  disp.print("BW ");
  disp.print(Bandwidth, 0);

  disp.setCursor(0, 31);
  disp.print("SF ");
  disp.print(SpreadingFactor);

  disp.setCursor(0, 39);
  disp.print("CR 4:");
  disp.print(CodeRate);

  disp.setCursor(0, 47);
  disp.print("Sync 0x");
  disp.print(SyncWord, HEX);

  disp.setCursor(0, 55);
  disp.print("TXpower ");
  disp.print(TXpower);

  disp.sendBuffer();

  delay(1500);

#ifdef USE_SD
  disp.setFont(u8g2_font_pxplusibmvga8_mr);

  if (SD_Found) {
    disp.clearBuffer();
    disp.setCursor(15, 55);
    disp.print(FileName);
    disp.sendBuffer();
  } else {
    disp.clearBuffer();
    disp.setCursor(15, 20);
    disp.print("SD card fail");
    disp.sendBuffer();
  }
#endif
}


void display_packet_LinkRX_detail(int16_t lorastate) {
  disp.clearBuffer();
  disp.setFont(u8g2_font_pxplusibmvga8_mr);
  disp.setCursor(8, 15);

  if (lorastate != 0) {
    disp.print("Error ");
    disp.print(lorastate);
  } else {
//disp.print("RX ");
#ifdef PRINT_ASCII
    for (uint8_t index = 0; index < RXPacketL; index++) {
      if (RXbuff[index] > 0)  //dont display null characters
      {
        disp.write(RXbuff[index]);
      }
    }
#endif
  }

  disp.setCursor(8, 29);
  disp.print("RSSI ");
  disp.print(PacketRSSI, 0);
  disp.print("dBm");

  disp.setCursor(8, 43);
  disp.print("SNR  ");
  disp.print(PacketSNR, 2);
  disp.print("dB");

  disp.setCursor(7, 57);
  disp.print("Cyc ");
  if (Test_Cycles > 0) {
    disp.print(Test_Cycles - 1);
  } else {
    disp.print("0");
  }

  disp.setCursor(85, 57);
  disp.print(Voltage, 2);
  disp.print("v");

  disp.sendBuffer();
}


void display_packet_LinkTX_detail(int16_t lorastate) {
  disp.clearBuffer();
  disp.setFont(u8g2_font_pxplusibmvga8_mr);
  disp.setCursor(8, 15);

  if (lorastate != 0) {
    disp.print("Error ");
    disp.print(lorastate);
  } else {
//disp.print("TX ");
#ifdef PRINT_ASCII
    for (uint8_t index = 0; index < TXPacketL; index++) {
      if (TXbuff[index] > 0)  //dont display null characters
      {
        disp.write(TXbuff[index]);
      }
    }
#endif
  }

  disp.setCursor(7, 57);
  disp.print("Cyc ");
  disp.print(Test_Cycles);

  disp.setCursor(85, 57);
  disp.print(Voltage, 2);
  disp.print("v");

  disp.sendBuffer();
}

#endif

//************************************************************************
//End OLED display functions
//************************************************************************


//************************************************************************
//Start SD card functions
//************************************************************************
#ifdef USE_SD

uint16_t setup_SDLog(char *filename) {
  uint16_t index;
  Serial.println(F("setup_SDLog()"));

#ifdef USE_ESP32
  SDCardSPI.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI);

  if (!SD.begin(SDCARD_CS, SDCardSPI)) {
    Serial.println();
    Serial.println("ERROR Card Mount Failed");
    Serial.println();
    SD_Found = false;
  } else {
    Serial.println("Card Mount OK");
    SD_Found = true;
  }
#else
  if (!SD.begin(SDCARD_CS)) {
    Serial.println();
    Serial.println("ERROR Card Mount Failed");
    Serial.println();
    SD_Found = false;
  } else {
    Serial.println("Card Mount OK");
    SD_Found = true;
  }
#endif

  for (index = 1; index <= 9999; index++) {
    filename[4] = index / 1000 + '0';
    filename[5] = ((index % 1000) / 100) + '0';
    filename[6] = ((index % 100) / 10) + '0';
    filename[7] = index % 10 + '0';

    if (!SD.exists(filename)) {
      //only open a new file if it doesn't exist
      logFile = SD.open(filename, FILE_WRITE);
      break;
    } else {
      //Serial.print(filename);
      //Serial.println(F(" exists"));
    }
  }

  if (SD.exists(filename)) {
    Serial.print(F("Writing log to "));
    Serial.println(filename);
    SD_Found = true;
  } else {
    Serial.println(F("Error creating SD log file"));
    SD_Found = false;
  }
  return index;
}


void log_Setup_SD(char *title) {
  logFile.print(F("Title "));
  logFile.println(title);

  logFile.print(F("Device "));
  logFile.println(selecteddevice);

  logFile.print(F("Frequency "));
  logFile.print(Frequency, 6);
  logFile.println(F("Mhz"));

  logFile.print(F("Bandwidth "));
  logFile.print(Bandwidth, 1);
  logFile.println(F("Khz"));

  logFile.print(F("SpreadingFactor "));
  logFile.println(SpreadingFactor);

  logFile.print(F("CodeRate 4:"));
  logFile.println(CodeRate);

  logFile.print(F("SyncWord 0x"));
  logFile.println(SyncWord, HEX);

  logFile.print(F("TXpower "));
  logFile.print(TXpower);
  logFile.println(F("dBm"));

  logFile.print(F("CurrentLimit "));
  logFile.print(CurrentLimit, 0);
  logFile.println(F("mA"));

  logFile.print(F("PreambleLength "));
  logFile.println(PreambleLength);

  logFile.flush();
}


void log_packet_RX_SD(int16_t lorastate) {

  if (lorastate != 0) {
    logFile.print(F("RXError,"));
    logFile.print(lorastate);
    logFile.print(F(","));
  } else {
#ifdef PRINT_ASCII
    for (uint8_t index = 0; index < RXPacketL; index++) {
      if (RXbuff[index] > 0)  //dont log null characters
      {
        logFile.write(RXbuff[index]);
      }
    }
    logFile.print(F(","));
#endif
  }

  //log packet length
  logFile.print(F("RXbytes,"));
  logFile.print(RXPacketL);

  //log RSSI
  logFile.print(F(",RSSI,"));
  logFile.print(PacketRSSI, 0);

  //log SNR
  logFile.print(F(",SNR,"));
  logFile.print(PacketSNR, 2);

  //log RXPacketsOK
  logFile.print(F(",RXOK,"));
  logFile.print(RXPacketsOK);

  //log PacketErrors
  logFile.print(F(",Errors,"));
  logFile.print(PacketErrors);

#ifdef PRINT_HEX
  logFile.print(F(",HEX"));
  for (uint8_t index = 0; index < RXPacketL; index++) {
    logFile.print(",");
    logHexByte(RXbuff[index]);
  }
#endif

  logFile.println();
  logFile.flush();
}


void logHexByte(uint8_t num) {
  if (num < 0x10) {
    logFile.print("0");
  }

  logFile.print(num, HEX);
}

#endif

//************************************************************************
//End SD card functions
//************************************************************************
