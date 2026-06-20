/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 10/06/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//#define DEBUG

#ifndef Monitorport
#define Monitorport Serial
#endif

/*
  Issues:
*/

//general functions
void print_temp_settings();
void print_active_settings();
void clear_serial();
uint32_t get_number32(char* numprompt, uint8_t numpromptsize, uint8_t digits, uint32_t minval, uint32_t maxval, uint32_t timeout);

//menus
bool menu_Main(uint32_t timeout);
void menu_LoRa(uint32_t timeout);
void menu_Camera(uint32_t timeout);
void menu_CameraNumber(uint32_t timeout);
uint8_t menu_ImageSize(uint32_t timeout, uint8_t tempimage);
uint8_t menu_ImageSizeNumber(uint32_t timeout);
uint32_t menu_RequestImage(uint32_t timeout);
void menu_Program(uint32_t timeout);
void menu_SettingsSource(uint32_t timeout);
void menu_FREQ(uint32_t timeout);
void menu_FREQOFF(uint32_t timeout);
void menu_SF(uint32_t timeout);
void menu_BW(uint32_t timeout);
void menu_CR(uint32_t timeout);
void menu_TXPOW(uint32_t timeout);
void menu_SegmentSize(uint32_t timeout);
void menu_PicturesToTake(uint32_t timeout);
void menu_TimeToSleep(uint32_t timeout);
void menu_MenuTimeout(uint32_t timeout);
void print_ImageSize(uint8_t imagesize);
void print_structure(uint8_t* ptr, uint8_t structsize);


//variables
bool serialtimeout;
bool numbervalid;
bool usingSX126X = false;
bool usingSX127X = false;
uint16_t SegmentTime;  //estimated time mS for a segment


void copy_var_vartemp() {
  memcpy(&vartemp, &var, sizeof(settings));  //memcpy(dest, src, size);
}

void copy_vartemp_var() {
  memcpy(&var, &vartemp, sizeof(settings));  //memcpy(dest, src, size);
}


void print_temp_settings() {
  Monitorport.println(F("Receiver Settings"));
  Monitorport.println(F("-----------------"));

  Monitorport.print(F("ListenCameraNumber "));
  Monitorport.println(vartemp.ListenCameraNumber);

  Monitorport.print(F("TXActions 0x"));
  Monitorport.println(vartemp.TXActions, HEX);

  Monitorport.print(F("Frequency "));
  Monitorport.print(vartemp.Frequency);
  Monitorport.println(F("Hz"));

  Monitorport.print(F("FreqOffset "));
  Monitorport.print(vartemp.FreqOffset);
  Monitorport.println(F("Hz"));

  Monitorport.print(F("SpreadingFactor "));
  Monitorport.println(vartemp.SpreadingFactor);

  Monitorport.print(F("Bandwidth "));
  Monitorport.print(LoRa.returnBandwidth(vartemp.Bandwidth));
  Monitorport.println(F("Hz"));

  Monitorport.print(F("CodeRate,4:"));
  Monitorport.println(vartemp.CodeRate + 4);

  Monitorport.print(F("TXpower "));
  Monitorport.print(vartemp.TXpower);
  Monitorport.println(F("dBm"));

  Monitorport.print(F("SegmentSize "));
  Monitorport.print(vartemp.SegmentSize);
  SegmentTime = ATestimatePacketTime(vartemp.SegmentSize, vartemp.SpreadingFactor, vartemp.Bandwidth);
  Monitorport.print(F(" ("));
  Monitorport.print(SegmentTime);
  Monitorport.println(F("mS)"));

  Monitorport.print(F("StandardImageSize "));
  print_ImageSize(vartemp.StandardImageSize);
  Monitorport.println();

  Monitorport.print(F("SentImageSize "));
  print_ImageSize(vartemp.SentImageSize);
  Monitorport.println();

  Monitorport.print(F("TimeToSleep "));
  Monitorport.print(vartemp.TimeToSleep);
  Monitorport.println(F(" Secs"));

  Monitorport.print(F("PicturesToTake "));
  Monitorport.println(vartemp.PicturesToTake);

  Monitorport.print(F("ListenCameraNumber "));
  Monitorport.println(vartemp.ListenCameraNumber);

  Monitorport.print(F("RequestedImage "));
  Monitorport.println(vartemp.RequestedImage);

  if (vartemp.EnforceBasicSettings) {
    Monitorport.println(F("EnforceBasicSettings enabled"));
  }

  if (vartemp.EnforceExtendedSettings) {
    Monitorport.println(F("EnforceExtendedSettings enabled"));
  }

  if ((!vartemp.EnforceExtendedSettings) && (!vartemp.EnforceBasicSettings)) {
    Monitorport.println(F("AutomaticSettings enabled"));
  }

  Monitorport.println();
}


void print_active_settings() {
  Monitorport.println(F("Active Receiver Settings"));
  Monitorport.println(F("-------------------------"));

  Monitorport.print(F("Frequency "));
  Monitorport.print(var.Frequency);
  Monitorport.println(F("Hz"));

  Monitorport.print(F("FreqOffset "));
  Monitorport.print(var.FreqOffset);
  Monitorport.println(F("Hz"));

  Monitorport.print(F("SpreadingFactor "));
  Monitorport.println(var.SpreadingFactor);

  Monitorport.print(F("Bandwidth "));
  Monitorport.print(LoRa.returnBandwidth(var.Bandwidth));
  Monitorport.println(F("Hz"));

  Monitorport.print(F("CodeRate 4:"));
  Monitorport.println(var.CodeRate + 4);

  Monitorport.print(F("TXpower "));
  Monitorport.print(var.TXpower);
  Monitorport.println(F("dBm"));

  Monitorport.print(F("SegmentSize "));
  Monitorport.print(var.SegmentSize);
  SegmentTime = ATestimatePacketTime(var.SegmentSize, var.SpreadingFactor, var.Bandwidth);
  Monitorport.print(F(" ("));
  Monitorport.print(SegmentTime);
  Monitorport.println(F("mS)"));

  Monitorport.print(F("StandardImageSize "));
  print_ImageSize(var.StandardImageSize);
  Monitorport.println();

  Monitorport.print(F("SentImageSize "));
  print_ImageSize(var.SentImageSize);
  Monitorport.println();

  Monitorport.print(F("TimeToSleep "));
  Monitorport.print(var.TimeToSleep);
  Monitorport.println(F(" Secs"));

  Monitorport.print(F("PicturesToTake "));
  Monitorport.println(var.PicturesToTake);

  Monitorport.print(F("ListenCameraNumber "));
  Monitorport.println(var.ListenCameraNumber);

  Monitorport.print(F("RequestedImage "));
  Monitorport.println(var.RequestedImage);

  Monitorport.println();
}


void clear_serial() {
  uint8_t nochar;
  delay(10);  //allow time for a few characters to arrive, '1' can be followed by CR & LF.

  while (Monitorport.available() > 0) {
    nochar = Monitorport.read();
  };
}


uint32_t get_number32(char* numprompt, uint8_t numpromptsize, uint8_t digits, uint32_t minval, uint32_t maxval, uint32_t timeout) {
  uint8_t readchar;
  uint8_t index = 0;
  uint8_t index2 = 0;
  int32_t num;
  uint32_t timeoutmS;
  char numberbuff[11];
  clear_serial();
  memset(numberbuff, 0, sizeof(numberbuff));
  serialtimeout = false;
  numbervalid = false;
  timeoutmS = millis() + timeout;

  do {
    if (Monitorport.available() > 0) {
      readchar = Monitorport.read();
      timeoutmS = millis() + timeout;  //restart timeout

      if ((readchar == 0x0a) || (readchar == 0x0d)) {
        clear_serial();
        if (index == 0) {
          return 0;
        }

        num = atoi(numberbuff);

        if ((num < minval) || (num > maxval)) {
          Monitorport.print(F("  "));
          Monitorport.print(num);
          Monitorport.println(F(" Number outside limits"));
          return 0;
        } else {
          numbervalid = true;
          return num;
        }
      }

      if ((index == 0) && ((readchar == '-') || (readchar == '+'))) {
        Monitorport.write(readchar);
        numberbuff[0] = readchar;
        index++;
      } else {
        if ((readchar == 'x') || (readchar == 'X') || (readchar == 0x1B))  //0x1B is ESC key
        {
          return false;
        }

        if ((readchar < '0') || (readchar > '9')) {
          Monitorport.write(readchar);
          Monitorport.println(F(" ERROR - not number"));
          index = 0;
          memset(numberbuff, 0, sizeof(numberbuff));  //fill array with nulls
          clear_serial();
          for (index2 = 0; index2 < numpromptsize; index2++) {
            Monitorport.write(numprompt[index2]);
          }
        } else {
          numberbuff[index] = readchar;
          index++;

          if (index > digits) {
            Monitorport.write(readchar);
            Monitorport.println(F(" ERROR - to long"));
            Monitorport.println();
            index = 0;
            memset(numberbuff, 0, sizeof(numberbuff));  //fill array with nulls
            clear_serial();
          } else {
            Monitorport.write(readchar);
          }
        }
      }
    }
  } while (millis() < timeoutmS);

  if (millis() >= timeoutmS) {
    serialtimeout = true;
    return 0;
  }
  return 0;
}

//***********************************************************
// Menus
//***********************************************************

bool menu_Main(uint32_t timeout) {
  uint32_t timeoutmS;
  uint8_t readchar;
  bool printmenu = false;
  serialtimeout = false;
  timeoutmS = millis() + timeout;
  clear_serial();
  uint8_t tempimage;

  copy_var_vartemp();  //make a temporary copy of the settings

  SENDACK2 = false;
  SENDACK3 = false;

  do {
    printmenu = false;
    Monitorport.println();
    Monitorport.println(F("Change Settings"));
    Monitorport.println(F("---------------"));
    Monitorport.println(F("1 LoRa settings"));
    Monitorport.println(F("2 Camera settings"));
    Monitorport.println(F("3 Transfer Settings Source"));
    Monitorport.println(F("4 Program settings"));
    Monitorport.println(F("5 Erase receiver store"));
    Monitorport.println(F("6 Update receiver store"));
    Monitorport.println(F("7 Erase transmitter (camera) store"));
    Monitorport.println(F("8 Update transmitter (camera) store"));
    Monitorport.println(F("9 Request image download"));
    Monitorport.println();

    Monitorport.println(F("P Print settings"));
    Monitorport.println(F("U Use changes"));
    Monitorport.println(F("X Exit"));
    Monitorport.println();

    do {
      if (Monitorport.available() > 0) {
        readchar = Monitorport.read();
        clear_serial();
        Monitorport.write(readchar);
        timeoutmS = millis() + timeout;

        if (readchar == '1') {
          Monitorport.println();
          menu_LoRa(timeout);
          Monitorport.println();
          if (serialtimeout) return false;
          timeoutmS = millis() + timeout;
          printmenu = true;
          continue;
        }

        if (readchar == '2') {
          Monitorport.println();
          menu_Camera(timeout);
          Monitorport.println();
          if (serialtimeout) return false;
          timeoutmS = millis() + timeout;
          printmenu = true;
          continue;
        }

        if (readchar == '3') {
          Monitorport.println();
          menu_SettingsSource(timeout);
          Monitorport.println();
          if (serialtimeout) return false;
          timeoutmS = millis() + timeout;
          printmenu = true;
          continue;
        }

        if (readchar == '4') {
          Monitorport.println();
          menu_Program(timeout);
          Monitorport.println();
          if (serialtimeout) return false;
          timeoutmS = millis() + timeout;
          printmenu = true;
          continue;
        }

        if (readchar == '5') {
          Monitorport.println(F(" Erasing receiver store "));
          Monitorport.println();
          erase_store();
          Monitorport.println(F("Reset receiver"));
          Monitorport.flush();
          delay(1000);
          esp_sleep_enable_timer_wakeup(1000000ULL);  //1 second deep sleep
          esp_deep_sleep_start();
        }

        if (readchar == '6') {
          Monitorport.println(F(" Saving settings to receiver Flash store"));
          Monitorport.println();
          copy_vartemp_store();
          copy_vartemp_var();
          Monitorport.println();
          Monitorport.println(F("Settings saved to receiver store"));
          Monitorport.println();
          delay(1500);
          printmenu = true;
          continue;
        }

        if (readchar == '7') {
          Monitorport.println(F(" Store in transmitter (camera) to be erased"));
          Monitorport.println();
          delay(1500);
          bitSet(vartemp.TXActions, TXerasestore);
          SENDACK3 = true;
          printmenu = true;
          continue;
        }

        if (readchar == '8') {
          Monitorport.println(F(" Settings to be saved in transmitter (camera) store"));
          Monitorport.println();
          delay(1500);
          bitSet(vartemp.TXActions, TXsavestore);
          SENDACK3 = true;
          printmenu = true;
          continue;
        }

        if (readchar == '9') {
          Monitorport.println();
          vartemp.RequestedImage = menu_RequestImage(timeout);
          bitSet(vartemp.TXActions, TXRequestedimage);
          Monitorport.println();
          if (serialtimeout) return false;
          timeoutmS = millis() + timeout;
          printmenu = true;
          continue;
        }

        if ((readchar == 'p') || (readchar == 'P')) {
          Monitorport.println();
          Monitorport.println();
          print_temp_settings();
          printmenu = true;
          continue;
        }

        if ((readchar == 'u') || (readchar == 'U')) {

          if ((vartemp.RequestedImage > 0) && (vartemp.ListenCameraNumber > 0)) {
            bitSet(vartemp.TXActions, TXRequestedimage);
          }
          Monitorport.println();
          copy_vartemp_var();

#ifdef DEBUG
          if (SENDACK2) {
            Monitorport.println(F("SENDACK2 true"));
          } else {
            Monitorport.println(F("SENDACK2 false"));
          }

          if (SENDACK3) {
            Monitorport.println(F("SENDACK3 true"));
          } else {
            Monitorport.println(F("SENDACK3 false"));
          }
#endif

          if ((SENDACK2 == false) && (SENDACK3 == false)) {
            Monitorport.println();
            Monitorport.println(F("Get settings from TX ready broadcast"));
          }

          if (SENDACK2 == true) {
            Monitorport.println();
            Monitorport.println(F("Send settings with ATReadyACK2"));
          }

          if (SENDACK3 == true) {
            Monitorport.println();
            Monitorport.println(F("Send settings with ATReadyACK3"));
          }
          return true;
        }

        if ((readchar == 'x') || (readchar == 'X') || (readchar == 0x1B))  //0x1B is ESC key
        {
          Monitorport.println(F(" Exit menu"));
          SENDACK2 = false;
          SENDACK3 = false;
          Monitorport.println();
          print_temp_settings();
          Monitorport.println();
          return false;
        }

        //if (printmenu) {
        //break;
        //}

        Monitorport.println(F(" Not recognised"));
        continue;
      }

      if (printmenu) {
        break;
      }

    } while (millis() < timeoutmS);

    if (millis() >= timeoutmS) {
      serialtimeout = true;
    }

    if (!printmenu) {
      break;
    }
  } while (1);
  return false;
}


void menu_LoRa(uint32_t timeout) {
  uint32_t timeoutmS;
  uint8_t readchar;
  bool printmenu = false;
  serialtimeout = false;
  timeoutmS = millis() + timeout;
  clear_serial();

  do {
    printmenu = false;
    Monitorport.println();
    Monitorport.println(F("Receiver LoRa Settings"));
    Monitorport.println(F("----------------------"));
    Monitorport.print(F("1 Frequency "));
    Monitorport.print(vartemp.Frequency);
    Monitorport.println(F("Hz"));
    Monitorport.print(F("2 FreqOffset "));
    Monitorport.print(vartemp.FreqOffset);
    Monitorport.println(F("Hz"));
    Monitorport.print(F("3 SpreadingFactor "));
    Monitorport.println(vartemp.SpreadingFactor);
    Monitorport.print(F("4 Bandwidth "));
    Monitorport.print(LoRa.returnBandwidth(vartemp.Bandwidth));
    Monitorport.println(F("Hz"));
    Monitorport.print(F("5 CodeRate "));
    Monitorport.println(vartemp.CodeRate + 4);
    Monitorport.print(F("6 TXpower "));
    Monitorport.print(vartemp.TXpower);
    Monitorport.println(F("dBm"));
    Monitorport.print(F("7 SegmentSize "));
    Monitorport.print(vartemp.SegmentSize);
    Monitorport.println();
    Monitorport.println(F("X Exit"));
    Monitorport.println();

    do {
      if (Monitorport.available() > 0) {
        readchar = Monitorport.read();
        clear_serial();
        Monitorport.write(readchar);
        timeoutmS = millis() + timeout;

        if (readchar == '1') {
          menu_FREQ(timeout);
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;  //run menu print again
        }

        if (readchar == '2') {
          menu_FREQOFF(timeout);
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }

        if (readchar == '3') {
          menu_SF(timeout);
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }

        if (readchar == '4') {
          menu_BW(timeout);
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }

        if (readchar == '5') {
          menu_CR(timeout);
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }

        if (readchar == '6') {
          menu_TXPOW(timeout);
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }

        if (readchar == '7') {
          menu_SegmentSize(timeout);
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }


        if ((readchar == 'x') || (readchar == 'X') || (readchar == 0x1B))  //0x1B is ESC key
        {
          Monitorport.println(F(" Exit menu"));
          return;
        }

        if (printmenu) {
          break;
        }

        Monitorport.println(F(" Not recognised"));
        continue;
      }
    } while (millis() < timeoutmS);

    if (millis() >= timeoutmS) {
      serialtimeout = true;
    }

    if (!printmenu) {
      break;
    }

  } while (1);
  return;
}


void menu_Camera(uint32_t timeout) {
  uint32_t timeoutmS;
  uint8_t readchar;
  bool printmenu = false;
  serialtimeout = false;
  timeoutmS = millis() + timeout;
  uint8_t tempimage;
  clear_serial();

  do {
    printmenu = false;
    Monitorport.println();
    Monitorport.println(F("Change Transmitter Camera Settings"));
    Monitorport.println(F("------ ---------------------------"));
    Monitorport.print(F("1 Receive from camera number "));
    Monitorport.println(vartemp.ListenCameraNumber);
    Monitorport.print(F("2 Standard image size > "));
    print_ImageSize(vartemp.StandardImageSize);
    Monitorport.println();
    Monitorport.print(F("3 Sent image size     > "));
    print_ImageSize(vartemp.SentImageSize);
    Monitorport.println();
    Monitorport.print(F("4 Set pictures to take "));
    Monitorport.println(vartemp.PicturesToTake);
    Monitorport.print(F("5 Set time to sleep "));
    Monitorport.print(vartemp.TimeToSleep);
    Monitorport.println(F(" Secs"));

    Monitorport.println();
    Monitorport.println(F("X Exit"));
    Monitorport.println();

    do {
      if (Monitorport.available() > 0) {
        readchar = Monitorport.read();
        clear_serial();
        Monitorport.write(readchar);
        timeoutmS = millis() + timeout;

        if (readchar == '1') {
          menu_CameraNumber(timeout);
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;  //run menu print again
        }

        if (readchar == '2') {
          Monitorport.println();
          Monitorport.print(F("Standard "));
          tempimage = menu_ImageSize(timeout, vartemp.StandardImageSize);
          vartemp.StandardImageSize = tempimage;
          SENDACK3 = true;
          Monitorport.println();
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }

        if (readchar == '3') {
          Monitorport.println();
          Monitorport.print(F("Sent "));
          tempimage = menu_ImageSize(timeout, vartemp.SentImageSize);
          vartemp.SentImageSize = tempimage;
          SENDACK3 = true;
          Monitorport.println();
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }

        if (readchar == '4') {
          menu_PicturesToTake(timeout);
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }

        if (readchar == '5') {
          menu_TimeToSleep(timeout);
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }

        if ((readchar == 'x') || (readchar == 'X') || (readchar == 0x1B))  //0x1B is ESC key
        {
          Monitorport.println(F(" Exit menu"));
          return;
        }

        if (printmenu) {
          break;
        }

        Monitorport.println(F(" Not recognised"));
        continue;
      }
    } while (millis() < timeoutmS);

    if (millis() >= timeoutmS) {
      serialtimeout = true;
    }

    if (!printmenu) {
      break;
    }

  } while (1);
  return;
}


uint32_t menu_RequestImage(uint32_t timeout) {

  uint32_t num;
  serialtimeout = false;
  char numprompt[] = "Enter image number > ";
  Monitorport.println();
  Monitorport.print(numprompt);
  num = get_number32(numprompt, sizeof(numprompt), 9, 0, 0xFFFFFFFF, timeout);

  if (num == 0) {
    Monitorport.println(F("Cancel Requested image"));
    return 0;
  }

  if (numbervalid) {
    Monitorport.println();
    Monitorport.print(F("Requested image "));
    Monitorport.println(num);
    SENDACK3 = true;
    return num;
  }
  return 0;
}


void menu_SettingsSource(uint32_t timeout) {
  uint32_t timeoutmS;
  uint8_t readchar;
  bool printmenu = false;
  serialtimeout = false;
  timeoutmS = millis() + timeout;
  clear_serial();

  do {
    printmenu = false;
    Monitorport.println();
    Monitorport.println(F("Settings Source"));
    Monitorport.println(F("---------------"));
    Monitorport.print(F("1 Automatic"));

    if ((!vartemp.EnforceBasicSettings) && (!vartemp.EnforceExtendedSettings)) {
      Monitorport.println(F(" - Currently enabled"));
    } else {
      Monitorport.println();
    }

    Monitorport.print(F("2 Enforce Basic"));

    if (vartemp.EnforceBasicSettings) {
      Monitorport.println(F(" - Currently enabled"));
    } else {
      Monitorport.println();
    }

    Monitorport.print(F("3 Enforce Extended"));

    if (vartemp.EnforceExtendedSettings) {
      Monitorport.println(F(" - Currently enabled"));
    } else {
      Monitorport.println();
    }
    Monitorport.println();
    Monitorport.println(F("X Exit"));
    Monitorport.println();


    do {
      if (Monitorport.available() > 0) {
        readchar = Monitorport.read();
        clear_serial();
        Monitorport.write(readchar);
        timeoutmS = millis() + timeout;

        if (readchar == '1') {
          Monitorport.println(F(" Auto transfer settings used"));
          vartemp.EnforceBasicSettings = false;
          vartemp.EnforceExtendedSettings = false;
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }

        if (readchar == '2') {
          Monitorport.println(F(" Basic transfer settings used"));
          vartemp.EnforceBasicSettings = true;
          vartemp.EnforceExtendedSettings = false;
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }

        if (readchar == '3') {
          Monitorport.println(F(" Extended transfer settings used"));
          vartemp.EnforceBasicSettings = false;
          vartemp.EnforceExtendedSettings = true;
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }

        if ((readchar == 'x') || (readchar == 'X') || (readchar == 0x1B))  //0x1B is ESC key
        {
          Monitorport.println(F(" Exit menu"));
          return;
        }

        if (printmenu) {
          break;
        }

        Monitorport.println(F(" Not recognised"));
        continue;
      }
    } while (millis() < timeoutmS);

    if (millis() >= timeoutmS) {
      serialtimeout = true;
    }

    if (!printmenu) {
      break;
    }
  } while (1);
  return;
}


void menu_Program(uint32_t timeout) {
  uint32_t timeoutmS;
  uint8_t readchar;
  bool printmenu = false;
  serialtimeout = false;
  timeoutmS = millis() + timeout;
  clear_serial();

  do {
    printmenu = false;
    Monitorport.println();
    Monitorport.println(F("Program Settings"));
    Monitorport.println(F("----------------"));
    Monitorport.print(F("1 MenuTimeout, "));
    Monitorport.print(vartemp.MenuTimeout);
    Monitorport.println(F("mS"));
    Monitorport.println();
    Monitorport.println(F("X Exit"));
    Monitorport.println();

    do {
      if (Monitorport.available() > 0) {
        readchar = Monitorport.read();
        clear_serial();
        Monitorport.write(readchar);
        timeoutmS = millis() + timeout;

        if (readchar == '1') {
          menu_MenuTimeout(timeout);
          if (serialtimeout) return;
          timeoutmS = millis() + timeout;
          printmenu = true;
        }

        if ((readchar == 'x') || (readchar == 'X') || (readchar == 0x1B))  //0x1B is ESC key
        {
          Monitorport.println(F(" Exit menu"));
          return;
        }

        if (printmenu) {
          break;
        }

        Monitorport.println(F(" Not recognised"));
        continue;
      }
    } while (millis() < timeoutmS);

    if (millis() >= timeoutmS) {
      serialtimeout = true;
    }

    if (!printmenu) {
      break;
    }
  } while (1);
  return;
}


uint8_t menu_ImageSize(uint32_t timeout, uint8_t tempimage) {
  uint32_t timeoutmS;
  uint8_t readchar;
  bool printmenu = false;
  serialtimeout = false;
  timeoutmS = millis() + timeout;
  clear_serial();
  uint8_t num;

  do {
    Monitorport.println(F("Image size"));
    Monitorport.println(F("-------------------"));
    Monitorport.println(F("0  FRAMESIZE_96X96      96x96"));
    Monitorport.println(F("1  FRAMESIZE_QQVGA    160x120"));
    Monitorport.println(F("2  FRAMESIZE_128x128  128x128"));
    Monitorport.println(F("3  FRAMESIZE_QCIF     176x144"));
    Monitorport.println(F("4  FRAMESIZE_HQVGA    240x176"));
    Monitorport.println(F("5  FRAMESIZE_240X240  240x240"));
    Monitorport.println(F("6  FRAMESIZE_QVGA     320x240"));
    Monitorport.println(F("7  FRAMESIZE_320x320  320x320"));
    Monitorport.println(F("8  FRAMESIZE_CIF      400x296"));
    Monitorport.println(F("9  FRAMESIZE_HVGA     480x320"));
    Monitorport.println(F("10 FRAMESIZE_VGA      640x480"));
    Monitorport.println(F("11 FRAMESIZE_SVGA     800x600"));
    Monitorport.println(F("12 FRAMESIZE_XGA     1024x768"));
    Monitorport.println(F("13 FRAMESIZE_HD      1280x720"));
    Monitorport.println(F("14 FRAMESIZE_SXGA    1280x1024"));
    Monitorport.println(F("15 FRAMESIZE_UXGA    1600x1200"));
    Monitorport.println(F("3MP Sensors"));
    Monitorport.println(F("16 FRAMESIZE_FHD     1920x1080"));
    Monitorport.println(F("17 FRAMESIZE_P_HD     720x1280"));
    Monitorport.println(F("18 FRAMESIZE_P_3MP    864x1536"));
    Monitorport.println(F("19 FRAMESIZE_QXGA    2048x1536"));
    Monitorport.println(F("5MP Sensors"));
    Monitorport.println(F("20 FRAMESIZE_QHD     2560x1440"));
    Monitorport.println(F("21 FRAMESIZE_WQXGA   2560x1600"));
    Monitorport.println(F("22 FRAMESIZE_P_FHD   1080x1920"));
    Monitorport.println(F("23 FRAMESIZE_QSXGA   2560x1920"));
    Monitorport.println(F("24 FRAMESIZE_5MP   2592x1944"));
    Monitorport.println();

    do {

      num = menu_ImageSizeNumber(timeout);

      if (num < 25) {
        return num;
      } else {
        return tempimage;
      }

    } while (millis() < timeoutmS);

    if (millis() >= timeoutmS) {
      serialtimeout = true;
      return tempimage;
    }

    if (!printmenu) {
      break;
    }
  } while (1);
}


uint8_t menu_ImageSizeNumber(uint32_t timeout) {

  int32_t num;
  serialtimeout = false;
  Monitorport.println();
  char numprompt[] = "Select Image size > ";
  Monitorport.print(numprompt);

  num = get_number32(numprompt, sizeof(numprompt), 2, 0, 24, timeout);

  if (numbervalid) {
    Monitorport.println();
    Monitorport.print(F("Entered Image Size "));
    Monitorport.println(num);
    return num;
  }

  return 25;
}


void menu_CameraNumber(uint32_t timeout) {
  int32_t num;
  serialtimeout = false;
  Monitorport.println();
  Monitorport.println();
  Monitorport.println(F("Camera number to listen for 0 for all or 1 to 255"));
  char numprompt[] = "Camera number > ";
  Monitorport.println();
  Monitorport.print(numprompt);
  num = get_number32(numprompt, sizeof(numprompt), 3, 0, 255, timeout);

  if (numbervalid) {
    Monitorport.println();
    Monitorport.print(F("Entered Camera number "));
    Monitorport.println(num);
    vartemp.ListenCameraNumber = num;
  }

  return;
}


void menu_FREQ(uint32_t timeout) {
  int32_t num;
  serialtimeout = false;
  char numprompt[] = "Enter Frequency in Hz > ";
  Monitorport.println();
  Monitorport.print(numprompt);
  num = get_number32(numprompt, sizeof(numprompt), 9, 400000000, 950000000, timeout);

  if (numbervalid) {
    Monitorport.println();
    Monitorport.print(F("Entered Frequency "));
    Monitorport.print(num);
    Monitorport.println(F("Hz"));
    vartemp.Frequency = num;
    SENDACK2 = true;
  }
  return;
}


void menu_FREQOFF(uint32_t timeout) {
  int32_t num;
  serialtimeout = false;
  char numprompt[] = "Enter Offset Frequency in Hz > ";
  Monitorport.println();
  Monitorport.print(numprompt);
  num = get_number32(numprompt, sizeof(numprompt), 6, -250000, 250000, timeout);

  if (numbervalid) {
    Monitorport.println();
    Monitorport.print(F("Entered Frequency Offset "));
    Monitorport.print(num);
    Monitorport.println(F("Hz"));
    vartemp.FreqOffset = num;
  }
}


void menu_SF(uint32_t timeout) {
  int32_t num;
  serialtimeout = false;

  if (usingSX127X) {
    char numprompt[] = "Enter LoRa Spreading factor, 7 to 12 > ";
    Monitorport.println();
    Monitorport.print(numprompt);
    num = get_number32(numprompt, sizeof(numprompt), 3, 7, 12, timeout);
  }

  if (usingSX126X) {
    char numprompt[] = "Enter LoRa Spreading factor, 5 to 12 > ";
    Monitorport.println();
    Monitorport.print(numprompt);
    num = get_number32(numprompt, sizeof(numprompt), 3, 5, 12, timeout);
  }

  if (numbervalid) {
    Monitorport.println();
    Monitorport.print(F("Entered Spreading Factor "));
    Monitorport.println(num);
    SENDACK2 = true;

    switch (num) {
      case 5:
        vartemp.SpreadingFactor = LORA_SF5;
        break;
      case 6:
        vartemp.SpreadingFactor = LORA_SF6;
        break;
      case 7:
        vartemp.SpreadingFactor = LORA_SF7;
        break;
      case 8:
        vartemp.SpreadingFactor = LORA_SF8;
        break;
      case 9:
        vartemp.SpreadingFactor = LORA_SF9;
        break;
      case 10:
        vartemp.SpreadingFactor = LORA_SF10;
        break;
      case 11:
        vartemp.SpreadingFactor = LORA_SF11;
        break;
      case 12:
        vartemp.SpreadingFactor = LORA_SF12;
        break;
    }
  }
  return;
}


void menu_BW(uint32_t timeout) {
  uint32_t timeoutmS;
  uint8_t readchar;
  bool printmenu = false;
  serialtimeout = false;
  timeoutmS = millis() + timeout;
  clear_serial();

  do {
    Monitorport.println();
    Monitorport.println(F("LoRa Bandwidth"));
    Monitorport.println(F("--------------"));
    Monitorport.println(F("1 7810Hz"));
    Monitorport.println(F("2 10420Hz"));
    Monitorport.println(F("3 15630Hz"));
    Monitorport.println(F("4 20830Hz"));
    Monitorport.println(F("5 31250Hz"));
    Monitorport.println(F("6 41670Hz"));
    Monitorport.println(F("7 62500Hz"));
    Monitorport.println(F("8 125000Hz"));
    Monitorport.println(F("9 250000Hz"));
    Monitorport.println(F("A 500000Hz"));
    Monitorport.println();
    Monitorport.println(F("X Exit menu"));
    Monitorport.println();
    Monitorport.print(F("Select Bandwidth > "));

    do {
      if (Monitorport.available() > 0) {
        readchar = Monitorport.read();
        timeoutmS = millis() + timeout;  //restart timeout
        clear_serial();                  //get rid of any spurious serial characters

        if ((readchar == 'x') || (readchar == 'X') || (readchar == 0x1B))  //0x1B is ESC key
        {
          Monitorport.println(F("Exit menu"));
          clear_serial();
          return;
        }

        if (readchar == '1') {
          Monitorport.println(F("7810Hz selected"));
          vartemp.Bandwidth = LORA_BW_007;
          printmenu = true;  //run menu print again
          SENDACK2 = true;
          break;
        }

        if (readchar == '2') {
          Monitorport.println(F("10420Hz selected"));
          vartemp.Bandwidth = LORA_BW_010;
          printmenu = true;
          SENDACK2 = true;
          break;
        }

        if (readchar == '3') {
          Monitorport.println(F("15630Hz selected"));
          vartemp.Bandwidth = LORA_BW_015;
          printmenu = true;
          SENDACK2 = true;
          break;
        }

        if (readchar == '4') {
          Monitorport.println(F("20830Hz selected"));
          vartemp.Bandwidth = LORA_BW_020;
          printmenu = true;
          SENDACK2 = true;
          break;
        }

        if (readchar == '5') {
          Monitorport.println(F("31250Hz selected"));
          vartemp.Bandwidth = LORA_BW_031;
          printmenu = true;
          SENDACK2 = true;
          break;
        }

        if (readchar == '6') {
          Monitorport.println(F("41670Hz selected"));
          vartemp.Bandwidth = LORA_BW_041;
          printmenu = true;
          SENDACK2 = true;
          break;
        }

        if (readchar == '7') {
          Monitorport.println(F("62500Hz selected"));
          vartemp.Bandwidth = LORA_BW_062;
          printmenu = true;
          SENDACK2 = true;
          break;
        }

        if (readchar == '8') {
          Monitorport.println(F("125000Hz selected"));
          vartemp.Bandwidth = LORA_BW_125;
          printmenu = true;
          SENDACK2 = true;
          break;
        }

        if (readchar == '9') {
          Monitorport.println(F("250000Hz selected"));
          vartemp.Bandwidth = LORA_BW_250;
          printmenu = true;
          SENDACK2 = true;
          break;
        }

        if ((readchar == 'a') || (readchar == 'A')) {
          Monitorport.println(F("500000Hz selected"));
          vartemp.Bandwidth = LORA_BW_500;
          printmenu = true;
          SENDACK2 = true;
          break;
        }
      }
    } while (millis() < timeoutmS);

    if (millis() >= timeoutmS) {
      serialtimeout = true;
      return;
    }

    if (!printmenu) {
      break;
    }
  } while (1);
  return;
}


void menu_CR(uint32_t timeout) {
  uint32_t timeoutmS;
  uint8_t readchar;
  bool printmenu = false;
  serialtimeout = false;
  timeoutmS = millis() + timeout;
  clear_serial();

  do {
    Monitorport.println();
    Monitorport.println(F("LoRa Coding rate"));
    Monitorport.println(F("----------------"));
    Monitorport.println(F("5 4 : 5"));
    Monitorport.println(F("6 4 : 6"));
    Monitorport.println(F("7 4 : 7"));
    Monitorport.println(F("8 4 : 8"));
    Monitorport.println();
    Monitorport.println(F("X Exit menu"));
    Monitorport.println();
    Monitorport.print(F("Select Coding rate > "));

    do {
      if (Monitorport.available() > 0) {
        readchar = Monitorport.read();
        timeoutmS = millis() + timeout;  //restart timeout
        clear_serial();                  //get rid of any spurious serial characters

        if ((readchar == 'x') || (readchar == 'X') || (readchar == 0x1B))  //0x1B is ESC key
        {
          Monitorport.println(F("Exit menu"));
          clear_serial();
          return;
        }

        if (readchar == '5') {
          Monitorport.println(F("4 : 5 selected"));
          vartemp.CodeRate = LORA_CR_4_5;
          printmenu = true;  //run menu print again
          SENDACK2 = true;
          break;
        }

        if (readchar == '6') {
          Monitorport.println(F("4 : 6 selected"));
          vartemp.CodeRate = LORA_CR_4_6;
          printmenu = true;
          SENDACK2 = true;
          break;
        }

        if (readchar == '7') {
          Monitorport.println(F("4 : 7 selected"));
          vartemp.CodeRate = LORA_CR_4_7;
          printmenu = true;
          SENDACK2 = true;
          break;
        }

        if (readchar == '8') {
          Monitorport.println(F("4 : 8 selected"));
          vartemp.CodeRate = LORA_CR_4_8;
          printmenu = true;
          SENDACK2 = true;
          break;
        }
      }
    } while (millis() < timeoutmS);

    if (millis() >= timeoutmS) {
      serialtimeout = true;
      return;
    }

    if (!printmenu) {
      break;
    }
  } while (1);
  return;
}


void menu_TXPOW(uint32_t timeout) {
  int32_t num;
  serialtimeout = false;
  char numprompt[] = "Enter LoRa TX Power dBm > ";
  Monitorport.println();
  Monitorport.print(numprompt);
  num = get_number32(numprompt, sizeof(numprompt), 3, -5, 10, timeout);

  if (numbervalid) {
    Monitorport.println();
    Monitorport.print(F("Entered TXpower "));
    Monitorport.print(num);
    Monitorport.println(F("dBm"));
    vartemp.TXpower = num;
    SENDACK3 = true;
  }
  return;
}


void menu_SegmentSize(uint32_t timeout) {
  int32_t num;
  serialtimeout = false;
  char numprompt[] = "Enter SegmentSize > ";
  Monitorport.println();
  Monitorport.print(numprompt);
  num = get_number32(numprompt, sizeof(numprompt), 3, 1, 245, timeout);

  if (numbervalid) {
    Monitorport.println();
    Monitorport.print(F("Entered SegmentSize "));
    Monitorport.println(num);
    vartemp.SegmentSize = num;
    SENDACK3 = true;
  }
  return;
}


void menu_PicturesToTake(uint32_t timeout) {
  uint8_t num;
  serialtimeout = false;
  char numprompt[] = "Enter Pictures To Take > ";
  Monitorport.println();
  Monitorport.print(numprompt);
  num = get_number32(numprompt, sizeof(numprompt), 3, 1, 245, timeout);

  if (numbervalid) {
    Monitorport.println();
    Monitorport.print(F("Entered Pictures To Take "));
    Monitorport.println(num);
    vartemp.PicturesToTake = num;
    SENDACK3 = true;
  }
  return;
}


void menu_TimeToSleep(uint32_t timeout) {
  uint32_t num;
  serialtimeout = false;
  char numprompt[] = "Enter Time To Sleep in seconds > ";
  Monitorport.println();
  Monitorport.print(numprompt);
  num = get_number32(numprompt, sizeof(numprompt), 9, 1, 65535, timeout);

  if (numbervalid) {
    Monitorport.println();
    Monitorport.print(F("Entered Time To Sleep  "));
    Monitorport.print(num);
    Monitorport.println(F(" Secs"));
    vartemp.TimeToSleep = num;
    SENDACK3 = true;
  }
}


void menu_MenuTimeout(uint32_t timeout) {
  int32_t num;
  serialtimeout = false;
  char numprompt[] = "Enter MenuTimeout in mS > ";
  Monitorport.println();
  Monitorport.print(numprompt);
  num = get_number32(numprompt, sizeof(numprompt), 9, 5000, 3600000, timeout);

  if (numbervalid) {
    Monitorport.println();
    Monitorport.print(F("Entered MenuTimeout  "));
    Monitorport.print(num);
    Monitorport.println(F("mS"));
    vartemp.MenuTimeout = num;
    SENDACK3 = true;
  }
}


void print_ImageSize(uint8_t imagesize) {
  switch (imagesize) {
    case FRAMESIZE_96X96:
      Monitorport.print(F("0 FRAMESIZE_96X96 96x96"));
      break;

    case FRAMESIZE_QQVGA:
      Monitorport.print(F("1 FRAMESIZE_QQVGA 160x120"));
      break;

    case FRAMESIZE_128X128:
      Monitorport.print(F("2 FRAMESIZE_128X128 128x128"));
      break;

    case FRAMESIZE_QCIF:
      Monitorport.print(F("3 FRAMESIZE_QCIF 176x144"));
      break;

    case FRAMESIZE_HQVGA:
      Monitorport.print(F("4 FRAMESIZE_HQVGA 240x176"));
      break;

    case FRAMESIZE_240X240:
      Monitorport.print(F("5 FRAMESIZE_240x240 240x240"));
      break;

    case FRAMESIZE_QVGA:
      Monitorport.print(F("6 FRAMESIZE_QVGA 320x240"));
      break;

    case FRAMESIZE_320X320:
      Monitorport.print(F("7 FRAMESIZE_320X320 320X320"));
      break;

    case FRAMESIZE_CIF:
      Monitorport.print(F("8 FRAMESIZE_CIF 400x296"));
      break;

    case FRAMESIZE_HVGA:
      Monitorport.print(F("9 FRAMESIZE_HVGA 480x320"));
      break;

    case FRAMESIZE_VGA:
      Monitorport.print(F("10 FRAMESIZE_VGA 640x480"));
      break;

    case FRAMESIZE_SVGA:
      Monitorport.print(F("11 FRAMESIZE_SVGA 800x600"));
      break;

    case FRAMESIZE_XGA:
      Monitorport.print(F("12 FRAMESIZE_XGA 1024x768"));
      break;

    case FRAMESIZE_HD:
      Monitorport.print(F("13 FRAMESIZE_HD 1280x720"));
      break;

    case FRAMESIZE_SXGA:
      Monitorport.print(F("14 FRAMESIZE_SXGA 1280x1024"));
      break;

    case FRAMESIZE_UXGA:
      Monitorport.print(F("15 FRAMESIZE_UXGA 1600x1200"));
      break;

      //3MP Sensors

    case FRAMESIZE_FHD:
      Monitorport.print(F("16 FRAMESIZE_FHD 1920x1080"));
      break;

    case FRAMESIZE_P_HD:
      Monitorport.print(F("17 FRAMESIZE_P_HD 720x1280"));
      break;

    case FRAMESIZE_P_3MP:
      Monitorport.print(F("18 FRAMESIZE_P_3MP 864x1536"));
      break;

    case FRAMESIZE_QXGA:
      Monitorport.print(F("19 FRAMESIZE_QXGA 2048x1536"));
      break;

      //5MP Sensors

    case FRAMESIZE_QHD:
      Monitorport.print(F("20 FRAMESIZE_QHD 2560x1440"));
      break;

    case FRAMESIZE_WQXGA:
      Monitorport.print(F("21 FRAMESIZE_WQXGA 2560x1600"));
      break;

    case FRAMESIZE_P_FHD:
      Monitorport.print(F("22 FRAMESIZE_P_FHD 1080x1920"));
      break;

    case FRAMESIZE_QSXGA:
      Monitorport.print(F("23 FRAMESIZE_QSXGA 2560x1920"));
      break;

    case FRAMESIZE_5MP:
      Monitorport.print(F("24 FRAMESIZE_5MP 2592x1944"));
      break;

    case FRAMESIZE_INVALID:
      Monitorport.print(F("25 FRAMESIZE_INVALID"));
      break;

    case REQUESTEDIMAGE:
      Monitorport.print(F("No size - REQUESTEDIMAGE"));
      break;

    case FILEERROR:
      Monitorport.print(F("FILE ERROR"));
      break;

    default:
      Monitorport.print(F("FRAMESIZE_INVALID"));
      break;
  }
}


void print_structure(uint8_t* ptr, uint8_t structsize) {
  uint8_t index, byteread;

#ifdef DEBUG
  Monitorport.print("Structure contents ");
#endif

  for (index = 0; index < structsize; index++) {
    byteread = (*(ptr + index));
#ifdef DEBUG
    Monitorport.print(byteread, HEX);
    Monitorport.print(" ");
#endif
  }

#ifdef DEBUG
  Monitorport.println();
#endif

  return;
}
