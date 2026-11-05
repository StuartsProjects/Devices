/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 11/10/26

  This programs is supplied as is, it is up to the user of the program to decide if the programs are
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*
Todo:

*/

#ifndef DISPLAYFLIPMODE
#define DISPLAYFLIPMODE 0
#endif

bool setup_Display(uint8_t dispadd);  //setup the 20x4 LCD
void display_LoRaRXpacket(uint8_t *buffer, uint8_t rxpacketl, int8_t packetrssi, int8_t packetsnr, uint32_t rxpacketcount, uint32_t rxerrors, int16_t state);
void display_FLRCRXpacket(uint8_t *buffer, uint8_t rxpacketl, int8_t packetrssi, uint32_t rxpacketcount, uint32_t rxerrors, int16_t state);
void display_TXpacket(uint8_t *buffer, uint8_t txpacketl, uint32_t txpacketcount, uint32_t txerrors, int8_t pw, int16_t state);
void display_LoRaModemSettings(float freq, float bw, uint8_t sf, uint8_t cr, uint8_t sw, int8_t pw, uint16_t preamlen);  //display the modem settings in use
void display_FLRCModemSettings(float freq, uint16_t bwbr, uint8_t cr, int8_t pw, uint16_t preamlen, uint8_t ds);
void clearLine(uint8_t linenum);

bool PACKETISASCII = true;

bool setup_Display(uint8_t dispadd) {
  //checks that an I2C device is present on the provide address
  uint8_t error;

  Wire.begin();
  Wire.beginTransmission(dispadd);
  error = Wire.endTransmission();

  if (error == 0) {
    disp.begin(20, 4);  //initialize for 20 x 4 lcd
    disp.setBacklight(255);
    return true;
  } else {
    return false;
  }
}


void display_LoRaRXpacket(uint8_t *buffer, uint8_t rxpacketl, int8_t packetrssi, int8_t packetsnr, uint32_t rxpacketcount, uint32_t rxerrors, int16_t state) {
  uint8_t index, dispchar, readchar;
  bool PACKETISASCII = true;

  clearLine(0);
  disp.setCursor(0, 0);

  if (rxpacketl > 16) {
    rxpacketl = 16;
  }

  for (index = 0; index <= (rxpacketl - 1); index++) {
    readchar = buffer[index];
    if (((readchar > 0) && (readchar < 0x20)) || (readchar > 0x7F)) {
      PACKETISASCII = false;
    }
  }

  if (state == RADIOLIB_ERR_NONE) {
    if (PACKETISASCII) {
      for (index = 0; index <= (rxpacketl - 1); index++) {
        dispchar = buffer[index];
        disp.write((uint8_t)dispchar);
      }
    } else {
      if (rxpacketl > 5) {
        rxpacketl = 5;
      }

      for (index = 0; index <= (rxpacketl - 1); index++) {
        dispchar = buffer[index];
        if (dispchar < 0x10) {
          disp.print(F("0"));
        }
        disp.print(dispchar, HEX);
        disp.print(F(" "));
      }
    }
  } else {
    disp.print(F("***Error***"));
  }

  clearLine(1);
  disp.setCursor(0, 1);
  disp.print(F("RSSI"));
  disp.print(packetrssi);
  disp.print(F("dBm"));
  disp.print(F(" SNR"));

  if (packetsnr > 0) {
    disp.print(F("+"));
  }

  disp.print(packetsnr);
  disp.print(F("dB"));

  clearLine(2);
  disp.setCursor(0, 2);
  disp.print(F("Len"));
  disp.print(rxpacketl);
  disp.print(F(" Pks"));
  disp.print(rxpacketcount);
  disp.print(F(" Ers"));
  disp.print(rxerrors);
  clearLine(3);
}


void display_FLRCRXpacket(uint8_t *buffer, uint8_t rxpacketl, int8_t packetrssi, uint32_t rxpacketcount, uint32_t rxerrors, int16_t state) {
  uint8_t index, dispchar, readchar;
  bool PACKETISASCII = true;

  clearLine(0);
  disp.setCursor(0, 0);

  if (rxpacketl > 16) {
    rxpacketl = 16;
  }

  for (index = 0; index <= (rxpacketl - 1); index++) {
    readchar = buffer[index];
    if (((readchar > 0) && (readchar < 0x20)) || (readchar > 0x7F)) {
      PACKETISASCII = false;
    }
  }

  if (state == RADIOLIB_ERR_NONE) {
    if (PACKETISASCII) {
      for (index = 0; index <= (rxpacketl - 1); index++) {
        dispchar = buffer[index];
        disp.write((uint8_t)dispchar);
      }
    } else {
      if (rxpacketl > 5) {
        rxpacketl = 5;
      }

      for (index = 0; index <= (rxpacketl - 1); index++) {
        dispchar = buffer[index];
        if (dispchar < 0x10) {
          disp.print(F("0"));
        }
        disp.print(dispchar, HEX);
        disp.print(F(" "));
      }
    }
  } else {
    disp.print(F("***Error***"));
  }

  clearLine(1);
  disp.setCursor(0, 1);
  disp.print(F("RSSI "));
  disp.print(packetrssi);
  disp.print(F("dBm"));

  clearLine(2);
  disp.setCursor(0, 2);
  disp.print(F("Len"));
  disp.print(rxpacketl);
  disp.print(F(" Pkts"));
  disp.print(rxpacketcount);
  disp.print(F(" Ers"));
  disp.print(rxerrors);
}



void display_TXpacket(uint8_t *buffer, uint8_t txpacketl, uint32_t txpacketcount, uint32_t txerrors, int8_t pw, int16_t state) {
  uint8_t index, dispchar, readchar;
  disp.clear();
  disp.setCursor(0, 0);

  if (txpacketl > 16) {
    txpacketl = 16;
  }

  PACKETISASCII = true;

  for (index = 0; index <= (txpacketl - 1); index++) {
    readchar = buffer[index];
    if (((readchar > 0) && (readchar < 0x20)) || (readchar > 0x7F)) {
      PACKETISASCII = false;
    }
  }

  if (state == RADIOLIB_ERR_NONE) {
    if (PACKETISASCII) {
      for (index = 0; index <= (txpacketl - 1); index++) {
        dispchar = buffer[index];
        disp.write((uint8_t)dispchar);
      }
    } else {
      if (txpacketl > 5) {
        txpacketl = 5;
      }

      for (index = 0; index <= (txpacketl - 1); index++) {
        dispchar = buffer[index];
        if (dispchar < 0x10) {
          disp.print(F("0"));
        }
        disp.print(dispchar, HEX);
        disp.print(F(" "));
      }
    }
  } else {
    disp.print(F("***Error***"));
  }

  disp.setCursor(0, 1);
  disp.print(F("Power "));
  disp.print(pw);
  disp.print(F("dBm"));

  disp.setCursor(0, 2);
  disp.print(F("Length "));
  disp.print(txpacketl);

  disp.setCursor(0, 3);
  disp.print(F("Pks "));
  disp.print(txpacketcount);
  disp.print(F(" Ers "));
  disp.print(txerrors);
}


void display_LoRaModemSettings(float freq, float bw, uint8_t sf, uint8_t cr, uint8_t sw, int8_t pw, uint16_t preamlen) {

  disp.clear();
  disp.setCursor(0, 0);
  disp.print(F("LoRa"));

  disp.setCursor(0, 1);
  disp.print(F("FR "));
  disp.print(freq, 3);
  disp.print(F("Mhz"));
  disp.setCursor(0, 2);
  disp.print(F("BW "));
  disp.print(bw, 1);
  disp.print(F("Khz"));
  disp.setCursor(0, 3);
  disp.print(F("SF"));
  disp.print(sf);
  disp.print(F(" CR4:"));
  disp.print(cr);
  disp.print(F(" PW"));
  disp.print(pw);
  disp.print(F("dBm"));
}


void display_FLRCModemSettings(float freq, uint16_t bwbr, uint8_t cr, int8_t pw, uint16_t preamlen, uint8_t ds) {

  bool VALIDCODINGRATE = false;
  bool VALIDBTMODSHAPING = false;

  disp.clear();

  clearLine(0);
  disp.setCursor(0, 0);
  disp.print(F("FLRC"));

  clearLine(1);
  disp.setCursor(0, 1);
  disp.print(F("FR "));
  disp.print(Frequency, 3);
  disp.print(F("Mhz"));

  clearLine(2);
  disp.setCursor(0, 2);
  disp.print(F("BWBR "));
  disp.print(bwbr);
  disp.print(F("Khz"));

  disp.print(F("  CR"));

  if (cr == RADIOLIB_LR2021_FLRC_CR_1_2) {
    disp.print(F("1_2"));
    VALIDCODINGRATE = true;
  }

  if (cr == RADIOLIB_LR2021_FLRC_CR_2_3) {
    disp.print(F("2_3"));
    VALIDCODINGRATE = true;
  }

  if (cr == RADIOLIB_LR2021_FLRC_CR_3_4) {
    disp.print(F("3_4"));
    VALIDCODINGRATE = true;
  }

  if (cr == RADIOLIB_LR2021_FLRC_CR_1_0) {
    disp.print(F("1_0"));
    VALIDCODINGRATE = true;
  }

  if (!VALIDCODINGRATE) {
    disp.print(F("Error"));
  }

  clearLine(3);
  disp.setCursor(0, 3);
  disp.print(F("DS "));

  if (ds == RADIOLIB_SHAPING_NONE) {
    disp.print(F("NONE"));
    VALIDBTMODSHAPING = true;
  }

  if (ds == RADIOLIB_SHAPING_1_0) {
    disp.print(F("1_0"));
    VALIDBTMODSHAPING = true;
  }

  if (ds == RADIOLIB_SHAPING_0_7) {
    disp.print(F("0_7"));
    VALIDBTMODSHAPING = true;
  }

  if (ds == RADIOLIB_SHAPING_0_5) {
    disp.print(F("0_5"));
    VALIDBTMODSHAPING = true;
  }

  if (ds == RADIOLIB_SHAPING_0_3) {
    disp.print(F("0_3"));
    VALIDBTMODSHAPING = true;
  }

  if (!VALIDBTMODSHAPING) {
    disp.print(F("Error"));
  }

  disp.print(F(" PW "));
  disp.print(TXPower);
  disp.print(F("dBm"));
}


void clearLine(uint8_t linenum) {
  disp.setCursor(0, linenum);
  disp.print(F("                    "));
}