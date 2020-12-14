/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 12/12/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This is a demonstration of using the ESP32CAM as a GPS tracker. From restart\reset
  the program sets up the GPS for high altitude mode, reads the position from the GPS, sets up the LoRa
  device then builds a HAB style payload and transmits it as LoRa according to the settings in the 
  Settings.h file. You can select and option to send a long range search mode packet, with lat,long and
  altitude, there is also the option to send the payload as FSK RTTY. Once the transmissions are sent the
  processor goes into a timed deep sleep wakes up after the set time and the process starts again.
  
  The temperature sensor inside the LoRa device (SX127X) is read and sent out in the HAB style payload,
  do check the calibration adjustment in Settings.h.

  The format of the HAB style payload is;

  PayloadID,Sequence,Time,Lat,Lon,Alt,Satellites,Volts,Temperature,PictureNo,Status,TXGPSfixms,Checksum
  Field 0      1      2    3   4   5      6        7       8          9        10      11        12


  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <TinyGPS++.h>                           //get library here > http://arduiniana.org/libraries/tinygpsplus/
TinyGPSPlus gps;                                 //create the TinyGPS++ object

#include <Arduino.h>
#include <SX127XLT.h>                            //get library here > https://github.com/StuartsProjects/SX12XX-LoRa  

SX127XLT LoRa;                                   //create a library class instance called LoRa

#include "Settings.h"
#include <ProgramLT_Definitions.h>

#include GPS_Library                             //include GPS Library as defined in 'Settings.h'

//**************************************************************************************************
// HAB tracker data - these are the variables transmitted in payload
//**************************************************************************************************
RTC_DATA_ATTR uint32_t TXSequence;               //sequence number of payload, , set to 1 on reset
uint8_t TXDay;
uint8_t TXMonth;
uint16_t TXYear;
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
RTC_DATA_ATTR uint16_t TXPicture = 0;            //number of picture taken, set to 0 on reset
uint8_t TXStatus;                                //used to store current status flag bits
uint32_t TXGPSfixms;                             //fix time of GPS
//**************************************************************************************************

bool loradevicefound;

uint8_t hours, mins, secs, day, month;
uint16_t year;

uint8_t TXPacketL;                               //length of LoRa packet sent
uint8_t  TXBUFFER[TXBUFFER_SIZE];                //buffer for packet to send

//#include Memory_Library                        //memory library to use

#include <SPI.h>

#define uS_TO_S_FACTOR 1000000                   //Conversion factor for micro seconds to seconds


void loop()
{

  Serial.println(F("Awake !"));

  TXSequence++;
  TXStatus = 0;                                  //start with TXStatus at 0, all good

  Serial.print(F("TXSequence "));
  Serial.println(TXSequence);

  Serial.println(F("Startup GPS check"));
  GPSTest();                                                        //display some output from GPS
  Serial.println(F("Startup GPS check finished"));
  Serial.println();

  configureGPS();                                                   //configure GPS for high altitude mode

  readGPS(WaitGPSFixSeconds);                                      //read GPS, wait no more than 60 seconds for a fix

  TXVolts = readSupplyVoltage();
  Serial.print(F("SupplyVoltage "));
  Serial.print(TXVolts);
  Serial.println(F("mV"));

  loradevicefound = setupLoRaDevice();                              //setup the LoRa device

  Serial.println();

  if (loradevicefound)
  {
    TXTemperature = LoRa.getDeviceTemperature() + temperature_compensate;
    do_Transmissions();                                             //do the transmissions
  }

  Serial.println(F("Sleep LoRa"));
  LoRa.setSleep(0);                                                   //force LoRa device into sleep.

  pinMode(16, OUTPUT);                                              //PSRAM CS
  digitalWrite(16, HIGH);
  pinMode(32, OUTPUT);                                              //Camera power
  digitalWrite(32, HIGH);

  esp_sleep_enable_timer_wakeup(SleepTimesecs * uS_TO_S_FACTOR);
  Serial.print(F("Start Sleep "));
  Serial.print(SleepTimesecs);
  Serial.println(F("s"));
  Serial.flush();

  esp_deep_sleep_start();
  Serial.println("This should never be printed");
}


void GPSTest()
{
  uint32_t endmS;
  uint8_t GPSbyte;

  endmS = millis() + 2000;                     //run GPS echo for 2000mS

  while (millis() < endmS)
  {
    GPSbyte = GPS_GetByte();
    if (GPSbyte < 0xFF)                   //this function in GPS library will return 0xFF if no characters available from GPS.
    {
      Serial.write(GPSbyte);
    }
  }
  Serial.flush();
}


bool configureGPS()
{
  Serial.println(F("GPS_Setup()"));

  GPS_Setup();                                  //initialise GPS to highaltitude\balloon mode from library

  Serial.println(F("GPS_CheckConfiguration()"));

  if (GPS_CheckConfiguration())                //Check that GPS is configured for high altitude mode
  {
    Serial.println();
    setStatusByte(GPSError, 0);
    setStatusByte(GPSConfigError, 0);

    Serial.println(F("GPS Config OK"));        //check tone indicates navigation model 6 set
    Serial.println();
    Serial.flush();
    return true;
  }
  else
  {
    setStatusByte(GPSConfigError, 1);
    Serial.println(F("GPS Error"));
    Serial.println();
    return false;
  }
}


uint16_t readSupplyVoltage()
{
  //relies on 3V3 supply volts reference and 100K & 47K resistor divider
  uint16_t temp;
  uint16_t volts = 0;
  byte index;

  temp = analogRead(SupplyAD);

  for (index = 0; index <= 19; index++)      //sample AD 20 times
  {
    temp = analogRead(SupplyAD);
    volts = volts + temp;
    delay(1);
  }
  volts = ((volts / 20) * ADMultiplier);

  return volts;
}



bool readGPS(uint8_t waitseconds)
{
  uint8_t numberwaits;
  uint8_t index;
  uint32_t startGetFixmS, endFixmS;

  numberwaits = waitseconds / 5;

  Serial.print("Wait GPS fix ");
  Serial.print(waitseconds);
  Serial.println("s");

  startGetFixmS = millis();

  for (index = 1; index < numberwaits; index++)
  {
    if (gpsWaitFix(5))
    {
      endFixmS = millis();
      TXGPSfixms = (endFixmS - startGetFixmS);
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
      whiteFlash(2, 5, 120);
      return true;
    }
    else
    {
      whiteFlash(1, 5, 120);
    }
    Serial.println(F("No GPS Fix"));
    Serial.println();
    setStatusByte(GPSFix, 0);
  }
  return false;
}


bool gpsWaitFix(uint16_t waitSecs)
{
  //waits a specified number of seconds for a fix, returns true for good fix

  uint32_t endwaitmS;
  uint8_t GPSByte;

  endwaitmS = millis() + (waitSecs * 1000);

  while (millis() < endwaitmS)
  {
    GPSByte = GPS_GetByte();
    if (GPSByte < 0xFF)                   //this function in GPS library will return 0xFF if no characters available from GPS.
    {
      gps.encode(GPSByte);
      Serial.write(GPSByte);              //debug output, flashes RED led also
    }

    if (gps.location.isUpdated() && gps.altitude.isUpdated() && gps.date.isUpdated())
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

  LoRa.toneFM(500, 1000, deviation, adjustfreq, ToneTXpower); //Transmit an FM tone, 1000hz, 1000ms
  delay(1000);

  setTrackerMode();

  TXPacketL = buildHABPacket();
  Serial.print(F("HAB Packet > "));
  printBuffer(TXBUFFER, (TXPacketL + 1));              //print the buffer (the packet to send) as ASCII

  startTimemS = millis();

  TXPacketL = LoRa.transmit(TXBUFFER, (TXPacketL + 1), 10000, TrackerTXpower, NO_WAIT); //will return packet length sent if OK, otherwise 0 if transmit error

  wait_IRQ_TX_DONE(5000);                              //wait for IRQ_TX_DONE, timeout of 5000mS

  printTXtime(startTimemS, millis());
  reportCompletion();
  Serial.println();

  if (readConfigByte(FSKRTTYEnable))
  {
    setTrackerMode();
    LoRa.setupDirect(TrackerFrequency, Offset);
    LoRa.startFSKRTTY(FrequencyShift, NumberofPips, PipPeriodmS, PipDelaymS, LeadinmS);

    startTimemS = millis() - LeadinmS;

    Serial.print(F("FSK RTTY > $$"));
    Serial.flush();
    LoRa.transmitFSKRTTY('$', BaudPerioduS, REDLED);          //send a '$' as sync
    LoRa.transmitFSKRTTY('$', BaudPerioduS, REDLED);          //send a '$' as sync
    LoRa.transmitFSKRTTY('$', BaudPerioduS, REDLED);          //send a '$' as sync
    LoRa.transmitFSKRTTY('$', BaudPerioduS, REDLED);          //send a '$' as sync


    for (index = 0; index <= (TXPacketL - 1); index++)      //its  TXPacketL-1 since we dont want to send the null at the end
    {
      LoRa.transmitFSKRTTY(TXBUFFER[index], BaudPerioduS, -1);
      Serial.write(TXBUFFER[index]);
    }

    LoRa.transmitFSKRTTY(13, BaudPerioduS, REDLED);              //send carriage return
    LoRa.transmitFSKRTTY(10, BaudPerioduS, REDLED);              //send line feed
    LoRa.endFSKRTTY(); //stop transmitting carrier
    digitalWrite(REDLED, LOW);                                 //LED off
    printTXtime(startTimemS, millis());
    TXPacketL += 4;                                          //add the two $ at beginning and CR/LF at end
    reportCompletion();
    Serial.println();
    delay(1000);                                            //gap between transmissions
  }

  if (readConfigByte(SearchEnable))
  {
    setSearchMode();
    TXPacketL = buildLocationOnly(TXLat, TXLon, TXAlt, TXStatus);  //put location data in SX12xx buffer
    Serial.print(F("Search packet > "));
    Serial.print(TXLat, 5);
    Serial.print(F(","));
    Serial.print(TXLon, 5);
    Serial.print(F(","));
    Serial.print(TXAlt);
    Serial.print(F(","));
    Serial.print(TXStatus);
    digitalWrite(REDLED, HIGH);
    startTimemS = millis();
    TXPacketL = LoRa.transmitSXBuffer(0, TXPacketL, 10000, SearchTXpower, WAIT_TX);
    printTXtime(startTimemS, millis());
    reportCompletion();
    Serial.println();
    delay(1000);                                        //gap between transmissions
  }
}


uint16_t wait_IRQ_TX_DONE(uint32_t txtimeout)
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

  dtostrf(TXLat, 7, 5, LatArray);                                //format is dtostrf(FLOAT,WIDTH,PRECISION,BUFFER);
  dtostrf(TXLon, 7, 5, LonArray);                                //converts float to character array

  len = sizeof(TXBUFFER);
  memset(TXBUFFER, 0, len);                                      //clear array to 0s
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
                   TXPicture,
                   TXStatus,
                   TXGPSfixms
                  );

  CRC = 0xffff;                                   //start value for CRC16

  for (index = 1; index < Count; index++)         //element 2 is first character after $ at start (for LoRa)
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
  Serial.print(IRQStatus, HEX);                //print IRQ status
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
  Serial.println(F("setTrackerMode"));
  LoRa.setupLoRa(TrackerFrequency, Offset, TrackerSpreadingFactor, TrackerBandwidth, TrackerCodeRate, TrackerOptimisation);
}


void setSearchMode()
{
  Serial.println(F("setSearchMode"));
  LoRa.setupLoRa(SearchFrequency, Offset, SearchSpreadingFactor, SearchBandwidth, SearchCodeRate, SearchOptimisation);
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

  wait_IRQ_TX_DONE(5000);                    //wait for IRQ_TX_DONE, timeout of 5000mS
  printTXtime(startTimemS, millis());
  reportCompletion();

  return TXPacketL;                         //TXPacketL will be 0 if there was an error sending
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

  setTrackerMode();
  LoRa.printModemSettings();                    //reads and prints the configured LoRa settings, useful check
  Serial.println();

  TXStatus = 0;                               //clear all TX status bits
  return true;
}


void checkMemoryClear()
{
  uint32_t endmS;

  endmS = millis() + 5000;

  while (millis() < endmS)
  {
    if (digitalRead(BOOTSWITCH))
    {
      break;
    }
  }

  if (millis() >= endmS)
  {
    //bootswitch was held down for at least 5 seconds so clear RTC memory.
    clearRTCmemory();
    whiteFlash(10, 5, 95);                 //10 rapid flashes as a indication of memory clear
  }
}


void clearRTCmemory()
{
  TXSequence = 1;
  TXPicture = 1;
}


void beginSerial()
{
  //the beginSerial() and endSerial() routines can be used to free up pin33 for LED indications
  Serial.begin(DEBUGBAUD, SERIAL_8N1, RXD2, TXD2);      //Debug port on pin33, format is Serial.begin(baud-rate, protocol, RX pin, TX pin);
}


void endSerial()
{
  //the beginSerial() and endSerial() routines can be used to free up pin33 for LED indications
  Serial.end();
}


void redFlash(uint16_t flashes, uint16_t ondelaymS, uint16_t offdelaymS)
{
  uint16_t index;

  pinMode(REDLED, OUTPUT);                            //setup pin as output

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(REDLED, LOW);
    delay(ondelaymS);
    digitalWrite(REDLED, HIGH);
    delay(offdelaymS);
  }

  pinMode(REDLED, INPUT);                             //setup pin as input
}


void whiteFlash(uint16_t flashes, uint16_t ondelaymS, uint16_t offdelaymS)
{
  uint16_t index;

  pinMode(WHITELED, OUTPUT);                          //setup pin as output

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(WHITELED, HIGH);
    delay(ondelaymS);
    digitalWrite(WHITELED, LOW);
    delay(offdelaymS);
  }
}


void setup()
{
  pinMode(BOOTSWITCH, INPUT_PULLUP);
  pinMode(REDLED, OUTPUT);
  digitalWrite(REDLED, LOW);                            //red LEDon
  whiteFlash(5, 5, 295);

  //Serial is set up to send on the debug port conencted to pin33, shared with the on board LED.
  //in this way the debug routines in the SX12XX library can be used, these print to Serial.
  Serial.begin(DEBUGBAUD, SERIAL_8N1, RXD2, TXD2);      //Debug port on pin33, format is Serial.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.println();
  Serial.println();
  Serial.println("10_ESP32CAM_LoRa_Tracker_Basic");
  Serial.println("Debug port - 115200baud");

  GPSserial.begin(GPSBAUD, SERIAL_8N1, RXD0, TXD0);     //GPS port on programming port
  GPSserial.println();                                  //wake up GPS

  delay(1000);

  //now test if boot switch is held down for 5 seconds, if so clear RTC memories.

  if (!digitalRead(BOOTSWITCH))                         //if the boot switch is pressed go anf check for memory clear
  {
    checkMemoryClear();
  }

  digitalWrite(REDLED, HIGH);                           //red LED off
}

