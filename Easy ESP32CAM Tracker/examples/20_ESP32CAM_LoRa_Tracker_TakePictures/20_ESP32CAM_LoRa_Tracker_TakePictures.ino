/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 12/12/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-save-microsd-card
  Github repository at https://github.com/RuiSantosdotme/ESP32-CAM-Arduino-IDE

  IMPORTANT!!!
   - Select Board "AI Thinker ESP32-CAM"
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

/*******************************************************************************************************
  Program Operation - This is a demonstration of using the ESP32CAM as a GPS tracker that can take pictures.

  From restart\reset the program sets up the GPS for high altitude mode and reads the position from the GPS.

  Then the program takes a picture and stores it on the SD card.

  Next the program sets up the LoRa device then builds a HAB style payload and transmits it as LoRa
  according to the settings in the Settings.h file. You can select and option to send a long range search
  mode packet with lat,long and altitude, there is also the option to send the payload as FSK RTTY.

  Once the transmissions are sent the processor goes into a timed deep sleep wakes up after the set time
  and the process starts again.

  The temperature sensor inside the LoRa device (SX127X) is read and sent out in the HAB style payload,
  do check the calibration adjustment in Settings.h.

  The format of the HAB style payload is;

  PayloadID,Sequence,Time,Lat,Lon,Alt,Satellites,Volts,Temperature,PictureNo,Status,TXGPSfixms,Checksum
  Field 0      1      2    3   4   5      6        7       8          9        10      11        12


  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <Arduino.h>
#include "soc/soc.h"                             //disable brownout problems
#include "soc/rtc_cntl_reg.h"                    //disable brownout problems
#include "driver/rtc_io.h"
#include "CameraCode.h"

#include <TinyGPS++.h>                           //get library here > http://arduiniana.org/libraries/tinygpsplus/
TinyGPSPlus gps;                                 //create the TinyGPS++ object

#include <SX127XLT.h>                            //include the appropriate library  

SX127XLT LoRa;                                   //create a library class instance called LT

#include "Settings.h"
#include <ProgramLT_Definitions.h>

#include GPS_Library                             //include previously defined GPS Library

//**************************************************************************************************
// HAB tracker data - these are the variables transmitted in payload
//**************************************************************************************************
RTC_DATA_ATTR uint32_t TXSequence = 0;           //sequence number of payload, set to 0 on reset
uint8_t TXDay;                                   //Day
uint8_t TXMonth;                                 //Month
uint16_t TXYear;                                 //Year
uint8_t TXHours;                                 //Hours
uint8_t TXMinutes;                               //Minutes
uint8_t TXSeconds;                               //Seconds
float TXLat;                                     //latitude from GPS
float TXLon;                                     //longitude from GPS
uint16_t TXAlt;                                  //altitude from GPS
uint8_t TXSatellites;                            //satellites used by GPS
uint32_t TXHdop;                                 //HDOP from GPS
uint16_t TXVolts;                                //measured tracker supply volts
int8_t TXTemperature;                            //measured temperature
RTC_DATA_ATTR uint16_t TXpicture = 0;            //number of picture taken, set to 0 on reset
uint8_t TXStatus;                                //used to store current status flag bits
uint32_t TXGPSfixms;                             //fix time of GPS
//**************************************************************************************************

bool loradevicefound;

uint8_t hours, mins, secs, day, month;
uint16_t year;

uint8_t TXPacketL;                               //length of LoRa packet sent
uint8_t  TXBUFFER[TXBUFFER_SIZE];                //buffer for packet to send

#include <SPI.h>

#define uS_TO_S_FACTOR 1000000                   //Conversion factor for micro seconds to seconds

RTC_DATA_ATTR int16_t bootCount = 0;             //variables to save in RTC ram
RTC_DATA_ATTR uint16_t sleepcount = 0;


void loop()
{
  pinMode(NSS, OUTPUT);
  digitalWrite(NSS, HIGH);                       //start with LoRa module disabled

  Serial.println(F("Awake !"));
  Serial.print(F("Bootcount "));
  Serial.println(bootCount);
  Serial.print(F("Sleepcount "));
  Serial.println(sleepcount);

  TXSequence++;
  TXStatus = 0;                                  //start with TXStatus at 0, all good

  Serial.print(F("TXSequence "));
  Serial.println(TXSequence);

  Serial.println(F("Startup GPS check"));
  GPSTest();                                     //display some output from GPS
  Serial.println(F("Startup GPS check finished"));
  Serial.println();

  configureGPS();                                //configure GPS for high altitude mode

  read_GPS(WaitGPSFixSeconds);                   //read GPS, wait no more than 60 seconds for a fix

  TXVolts = readSupplyVoltage();
  Serial.print(F("SupplyVoltage "));
  Serial.print(TXVolts);
  Serial.println(F("mV"));

  if (!configInitCamera())
  {
    setStatusByte(CameraError, 1);
  }

  if (!initMicroSDCard())
  {
    setStatusByte(SDError, 1);
  }

  TXpicture = takeSavePhoto(PicturesToTake, PictureDelaymS);

  if (TXpicture == 0)
  {
    setStatusByte(SDError, 1);
  }

  //Serial.print(F("Status byte "));
  //Serial.println(TXStatus);

  pinMode(WHITELED, OUTPUT);
  digitalWrite(WHITELED, LOW);                   //Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4

  loradevicefound = setupLoRaDevice();           //setup the LoRa device

  Serial.println();

  if (loradevicefound)
  {
    TXTemperature = LoRa.getDeviceTemperature() + temperature_compensate;
    do_Transmissions();                          //do the transmissions
  }

  Serial.println(F("Sleep LoRa"));
  LoRa.setSleep(0);                              //force LoRa device into sleep.

  pinMode(16, OUTPUT);                           //PSRAM CS
  digitalWrite(16, HIGH);
  pinMode(32, OUTPUT);                           //camera power
  digitalWrite(32, HIGH);

  rtc_gpio_hold_en(GPIO_NUM_4);                  //so LED stays off in sleep


  esp_sleep_enable_timer_wakeup(SleepTimesecs * uS_TO_S_FACTOR);
  Serial.print(F("Start Sleep "));
  Serial.print(SleepTimesecs);
  Serial.println(F("s"));
  Serial.flush();

  sleepcount++;
  esp_deep_sleep_start();
  Serial.println("This should never be printed");
}


void GPSTest()
{
  uint32_t endmS;

  endmS = millis() + 2000;                     //run GPS echo for 2000mS

  while (millis() < endmS)
  {
    while (GPSserial.available() > 0)
      Serial.write(GPSserial.read());
  }
  Serial.flush();
}


bool configureGPS()
{
  Serial.println(F("GPS_Setup()"));

  GPS_Setup();                                  //initialise GPS to high altitude or balloon mode from library

  Serial.println(F("GPS_CheckConfiguration()"));

  if (GPS_CheckConfiguration())                //check that GPS is configured for high altitude mode
  {
    Serial.println();
    setStatusByte(GPSError, 0);
    setStatusByte(GPSConfigError, 0);
    Serial.println(F("GPS Config OK"));        //check tone indicates navigation model 6 set
    Serial.flush();
    return true;
  }
  else
  {
    setStatusByte(GPSConfigError, 1);
    Serial.println(F("GPS Error"));
    setStatusByte(GPSError, 1);
    setStatusByte(GPSConfigError, 1);
    return false;
  }
}


uint16_t readSupplyVoltage()
{
  uint16_t adreading;
  uint16_t volts = 0;
  byte index;

  adreading = analogRead(SupplyAD);          //start with a reading

  for (index = 0; index <= 19; index++)      //sample AD 20 times
  {
    adreading = analogRead(SupplyAD);
    volts = volts + adreading;
    delay(1);
  }
  volts = ((volts / 20) * ADMultiplier);

  pinMode (SupplyAD, INPUT);

  return volts;
}


bool read_GPS(uint8_t waitseconds)
{
  uint32_t startGetFixmS;

  Serial.print("Wait GPS fix ");
  Serial.print(waitseconds);
  Serial.println("s");

  startGetFixmS = millis();

  if (gpsWaitFix(waitseconds))
  {
    TXGPSfixms = (millis() - startGetFixmS);
    Serial.println();
    Serial.println();
    Serial.print(F("Fix time "));
    Serial.print(TXGPSfixms);
    Serial.println(F("mS"));

    TXLat = gps.location.lat();
    TXLon = gps.location.lng();
    TXAlt = gps.altitude.meters();
    TXSatellites = gps.satellites.value();
    TXHdop = gps.hdop.value();

    TXHours = gps.time.hour();
    TXMinutes = gps.time.minute();
    TXSeconds = gps.time.second();
    TXDay = gps.date.day();
    TXMonth = gps.date.month();
    TXYear = gps.date.year();
    printGPSfix();
    setStatusByte(GPSFix, 1);
    return true;
  }

  Serial.println(F("No GPS Fix"));
  Serial.println();
  setStatusByte(GPSFix, 0);

  return false;
}


bool gpsWaitFix(uint16_t waitSecs)
{
  //waits a specified number of seconds for a fix, returns true for good fix

  uint32_t endwaitmS;
  uint8_t GPSchar;

  endwaitmS = millis() + (waitSecs * 1000);

  while (millis() < endwaitmS)
  {
    if (GPSserial.available() > 0)
    {
      GPSchar = GPSserial.read();
      gps.encode(GPSchar);
      Serial.write(GPSchar);
    }

    if (gps.location.isUpdated() && gps.altitude.isUpdated())
    {
      return true;
    }
  }

  return false;
}


void printGPSfix()
{
  float tempfloat;

  Serial.print(F("New GPS Fix "));

  tempfloat = ( (float) TXHdop / 100);

  Serial.print(F("Lat,"));
  Serial.print(TXLat, 6);
  Serial.print(F(",Lon,"));
  Serial.print(TXLon, 6);
  Serial.print(F(",Alt,"));
  Serial.print(TXAlt, 1);
  Serial.print(F("m,Sats,"));
  Serial.print(TXSatellites);
  Serial.print(F(",HDOP,"));
  Serial.print(tempfloat, 2);
  Serial.print(F(",Time,"));

  if (hours < 10)
  {
    Serial.print(F("0"));
  }

  Serial.print(TXHours);
  Serial.print(F(":"));

  if (mins < 10)
  {
    Serial.print(F("0"));
  }

  Serial.print(TXMinutes);
  Serial.print(F(":"));

  if (secs < 10)
  {
    Serial.print(F("0"));
  }

  Serial.print(secs);
  Serial.print(F(",Date,"));

  Serial.print(TXDay);
  Serial.print(F("/"));
  Serial.print(TXMonth);
  Serial.print(F("/"));
  Serial.print(TXYear);

  Serial.println();
  Serial.println();
}


void do_Transmissions()
{
  //this is where all the transmisions get sent
  uint32_t startTimemS;
  uint8_t index;

  setTrackerMode();
  TXPacketL = buildHABPacket();
  Serial.print(F("HAB Packet > "));
  printBuffer(TXBUFFER, (TXPacketL + 1));              //print the buffer (the packet to send) as ASCII
  Serial.flush();
  startTimemS = millis();
  TXPacketL = LoRa.transmit(TXBUFFER, (TXPacketL + 1), 10000, TrackerTXpower, NO_WAIT); //will return packet length sent if OK, otherwise 0 if transmit error
  waitIRQTXDONE(5000);                                 //wait for IRQ_TX_DONE, timeout of 5000mS
  printTXtime(startTimemS, millis());
  reportCompletion();
  Serial.println();
  delay(1000);


  if (readConfigByte(SearchEnable))
  {
    //send location only packet - long range settings
    setSearchMode();
    Serial.print(F("LocationPacket > "));
    Serial.print(TXLat, 5);
    Serial.print(F(","));
    Serial.print(TXLon, 5);
    Serial.print(F(","));
    Serial.print(TXAlt);
    Serial.print(F(","));
    Serial.print(TXStatus);
    //Serial.println();
    Serial.flush();
    TXPacketL = buildLocationOnly(TXLat, TXLon, TXAlt, TXStatus);  //put location data in SX12xx buffer
    startTimemS = millis();
    TXPacketL = LoRa.transmitSXBuffer(0, TXPacketL, 10000, SearchTXpower, NO_WAIT);
    waitIRQTXDONE(5000);                                           //wait for IRQ_TX_DONE, timeout of 5000mS
    printTXtime(startTimemS, millis());
    reportCompletion();
    Serial.println();
    LoRa.printModemSettings();                    //reads and prints the configured LoRa settings, useful check
    Serial.println();
    delay(1000);
  }


  if (readConfigByte(FSKRTTYEnable))
  {
    LoRa.setupDirect(TrackerFrequency, Offset);
    LoRa.startFSKRTTY(FrequencyShift, NumberofPips, PipPeriodmS, PipDelaymS, LeadinmS);
    Serial.print(F("FSK RTTY > "));
    Serial.print(F("$$$$"));
    Serial.flush();
    startTimemS = millis() - LeadinmS;
    LoRa.transmitFSKRTTY('$', BaudPerioduS, REDLED);        //send a '$' as sync
    LoRa.transmitFSKRTTY('$', BaudPerioduS, REDLED);        //send a '$' as sync
    LoRa.transmitFSKRTTY('$', BaudPerioduS, REDLED);        //send a '$' as sync
    LoRa.transmitFSKRTTY('$', BaudPerioduS, REDLED);        //send a '$' as sync

    for (index = 0; index <= (TXPacketL - 1); index++)      //its  TXPacketL-1 since we dont want to send the null at the end
    {
      LoRa.transmitFSKRTTY(TXBUFFER[index], BaudPerioduS, -1);
      Serial.write(TXBUFFER[index]);
    }

    LoRa.transmitFSKRTTY(13, BaudPerioduS, REDLED);         //send carriage return
    LoRa.transmitFSKRTTY(10, BaudPerioduS, REDLED);         //send line feed
    LoRa.endFSKRTTY(); //stop transmitting carrier
    digitalWrite(REDLED, LOW);                              //LED off
    printTXtime(startTimemS, millis());
    TXPacketL += 4;                                         //add the two $ at beginning and CR/LF at end
    reportCompletion();
    Serial.println();
    delay(1000);                                            //gap between transmissions
  }
}


uint16_t waitIRQTXDONE(uint32_t txtimeout)
{
  uint32_t endtimeoutmS;

  endtimeoutmS = (millis() + txtimeout);

  while (! (LoRa.readIrqStatus() & IRQ_TX_DONE) && (millis() < endtimeoutmS));

  if ((LoRa.readIrqStatus() & IRQ_TX_DONE))
  {
    Serial.print(F(" IRQ_TX_DONE "));
    return IRQ_TX_DONE;
  }
  else
  {
    Serial.print(F(" IRQ_TX_TIMEOUT "));
    return IRQ_TX_TIMEOUT;
  }
}


void printTXtime(uint32_t startmS, uint32_t endmS)
{
  Serial.print(F(" "));
  Serial.print(endmS - startmS);
  Serial.print(F("mS"));
}


void reportCompletion()
{
  Serial.print(F(" "));
  if (TXPacketL == 0)
  {
    Serial.println();
    setStatusByte(LORAError, 1);
    reporttransmitError();
  }
  else
  {
    Serial.print(TXPacketL);
    Serial.print(F("bytes"));
    setStatusByte(LORAError, 0);
  }
}


void printBuffer(uint8_t *buffer, uint8_t size)
{
  uint8_t index;

  for (index = 0; index < size; index++)
  {
    Serial.write(buffer[index]);
  }
}


uint8_t buildHABPacket()
{
  //build the HAB tracker payload
  uint16_t index, j, CRC;
  uint8_t Count, len;
  char LatArray[12], LonArray[12];

  dtostrf(TXLat, 7, 5, LatArray);              //format is dtostrf(FLOAT,WIDTH,PRECISION,BUFFER);
  dtostrf(TXLon, 7, 5, LonArray);              //converts float to character array

  len = sizeof(TXBUFFER);
  memset(TXBUFFER, 0, len);                    //clear array to 0s
  Count = snprintf((char*) TXBUFFER,
                   TXBUFFER_SIZE,
                   "$%s,%u,%02d:%02d:%02d,%s,%s,%d,%d,%d,%u,%u,%u,%u",
                   FlightID,
                   TXSequence,
                   TXHours,
                   TXMinutes,
                   TXSeconds,
                   LatArray,
                   LonArray,
                   TXAlt,
                   TXSatellites,
                   TXVolts,
                   TXTemperature,
                   TXpicture,
                   TXStatus,
                   TXGPSfixms
                  );

  CRC = 0xffff;                                   //start value for CRC16

  for (index = 1; index < Count; index++)         //element 1 is first character after $ at start (for LoRa)
  {
    CRC ^= (((uint16_t)TXBUFFER[index]) << 8);
    for (j = 0; j < 8; j++)
    {
      if (CRC & 0x8000)
        CRC = (CRC << 1) ^ 0x1021;
      else
        CRC <<= 1;
    }
  }

  TXBUFFER[Count++] = '*';
  TXBUFFER[Count++] = Hex((CRC >> 12) & 15);      //add the checksum bytes to the end
  TXBUFFER[Count++] = Hex((CRC >> 8) & 15);
  TXBUFFER[Count++] = Hex((CRC >> 4) & 15);
  TXBUFFER[Count] = Hex(CRC & 15);
  return Count;
}


char Hex(uint8_t lchar)
{
  //used in CRC calculation in buildHABPacket
  char Table[] = "0123456789ABCDEF";
  return Table[lchar];
}


uint8_t buildLocationOnly(float Lat, float Lon, uint16_t Alt, uint8_t stat)
{
  uint8_t len;
  LoRa.startWriteSXBuffer(0);                   //initialise buffer write at address 0
  LoRa.writeUint8(LocationBinaryPacket);        //identify type of packet
  LoRa.writeUint8(Broadcast);                   //who is the packet sent too
  LoRa.writeUint8(ThisNode);                    //tells receiver where is packet from
  LoRa.writeFloat(Lat);                         //add latitude
  LoRa.writeFloat(Lon);                         //add longitude
  LoRa.writeInt16(Alt);                         //add altitude
  LoRa.writeUint8(stat);                        //add tracker status
  len = LoRa.endWriteSXBuffer();                //close buffer write
  return len;
}


void reporttransmitError()
{
  uint16_t IRQStatus;
  IRQStatus = LoRa.readIrqStatus();              //read the the interrupt register
  Serial.print(F("TXError,"));
  Serial.print(F(",IRQreg,"));
  Serial.print(IRQStatus, HEX);                  //print IRQ status
  LoRa.printIrqStatus();                         //prints the text of which IRQs set
  setStatusByte(LORAError, 1);
}


void setStatusByte(uint8_t bitnum, uint8_t bitval)
{
  //program the status byte

  if (bitval == 0)
  {
    bitClear(TXStatus, bitnum);
  }
  else
  {
    bitSet(TXStatus, bitnum);
  }
}


uint8_t readConfigByte(uint8_t bitnum)
{
  return bitRead(Default_config1, bitnum);
}


void setTrackerMode()
{
  Serial.println(F("setTrackerMode()"));
  LoRa.setupLoRa(TrackerFrequency, Offset, TrackerSpreadingFactor, TrackerBandwidth, TrackerCodeRate, TrackerOptimisation);
  LoRa.printModemSettings();
  Serial.println();
}


void setSearchMode()
{
  Serial.println(F("setSearchMode()"));
  LoRa.setupLoRa(SearchFrequency, Offset, SearchSpreadingFactor, SearchBandwidth, SearchCodeRate, SearchOptimisation);
  LoRa.printModemSettings();
  Serial.println();
}


uint8_t sendCommand(char cmd)
{
  uint32_t startTimemS;
  uint8_t len;
  Serial.print(F("Send Cmd "));
  Serial.write(cmd);

  LoRa.startWriteSXBuffer(0);                 //start the write packet to buffer process
  LoRa.writeUint8(cmd);                       //this byte defines the packet type
  LoRa.writeUint8(Broadcast);                 //destination address of the packet, the receivers address
  LoRa.writeUint8(ThisNode);                  //source address of this node
  LoRa.writeUint16(TXVolts);                  //add the battery voltage
  LoRa.writeUint8(TXStatus);                  //add the status byte
  len = LoRa.endWriteSXBuffer();              //close the packet, get the length of data to be sent

  //now transmit the packet, set a timeout of 5000mS, wait for it to complete sending
  startTimemS = millis();
  TXPacketL = LoRa.transmitSXBuffer(0, len, 5000, TrackerTXpower, NO_WAIT);

  waitIRQTXDONE(5000);                        //wait for IRQ_TX_DONE, timeout of 5000mS
  printTXtime(startTimemS, millis());
  reportCompletion();

  return TXPacketL;                           //TXPacketL will be 0 if there was an error sending
}


bool setupLoRaDevice()
{
  SPI.begin(SCK, MISO, MOSI, NSS);

  if (LoRa.begin(NSS, LORA_DEVICE))
  {
    setStatusByte(LORAError, 0);
    Serial.println(F("LoRa device found"));
    Serial.println();
  }
  else
  {
    setStatusByte(LORAError, 1);
    Serial.println(F("LoRa Device error"));
    return false;
  }
  return true;
}


void redFlash(uint16_t flashes, uint16_t ondelaymS, uint16_t offdelaymS)
{
  uint16_t index;

  pinMode(REDLED, OUTPUT);                      //setup pin as output

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(REDLED, LOW);
    delay(ondelaymS);
    digitalWrite(REDLED, HIGH);
    delay(offdelaymS);
  }
  pinMode(REDLED, INPUT);                       //setup pin as input
}


void whiteFlash(uint16_t flashes, uint16_t ondelaymS, uint16_t offdelaymS)
{
  uint16_t index;

  pinMode(4, OUTPUT);                           //setup pin as output

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(4, HIGH);
    delay(ondelaymS);
    digitalWrite(4, LOW);
    delay(offdelaymS);
  }
}


void setup()
{
  Serial.begin(DEBUGBAUD, SERIAL_8N1, RXD2, TXD2);      //debug port, format is Serial.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.println();

  Serial.println("20_ESP32CAM_LoRa_Tracker_TakePictures - Debug port - 115200baud");

  pinMode(BOOTSWITCH, INPUT_PULLUP);
  rtc_gpio_hold_dis(GPIO_NUM_4);
  whiteFlash(1, 5, 295);                                //brief white LED flash at startup

  if (bootCount == 0) //Run this only the first time
  {
    bootCount = bootCount + 1;
  }

  GPSserial.begin(GPSBAUD, SERIAL_8N1, RXD0, TXD0);     //GPS port
  GPSserial.println();                                  //wake up GPS, in case its been put to sleep
  //delay(1000);
}
