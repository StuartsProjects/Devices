/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 03/11/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO RP2040.

  Program Operation -  This program is an example of a basic GPS tracker. The program reads the GPS,
  waits for an updated fix and transmits location, altitude, number of satellites in view, the HDOP
  value, the fix time of the GPS. At startup there should be at least a couple of seconds of recognisable
  text from the GPS printed to the serial monitor. If you see garbage or funny characters its likley the
  GPS baud rate is wrong. Outside with a good view of the sky most GPSs should produce a location fix in
  around 45 seconds. The number of satellites and HDOP are good indications to how well a GPS is working.

  The program writes direct to the LoRa devices internal buffer, no memory buffer is used.

  The LoRa settings are configured in the Settings.h file.

  Uses the earlephilhower/arduino-pico: Raspberry Pi Pico Arduino core, for all RP2040 boards.
  Get it here > https://github.com/earlephilhower/arduino-pico
  Board selected is 'Seeed XIAO RP2040'

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <SPI.h>
#include <SX127XLT.h>                              //get library here > https://github.com/StuartsProjects/SX12XX-LoRa

#include "Settings.h"
#include <ProgramLT_Definitions.h>

SX127XLT LoRa;

#include <TinyGPS++.h>                             //get library here > http://arduiniana.org/libraries/tinygpsplus/
TinyGPSPlus gps;                                   //create the TinyGPS++ object

uint8_t TXStatus = 0;                              //used to store current status flag bits of Tracker transmitter (TX)
uint8_t TXPacketL;                                 //length of LoRa packet (TX)
float TXLat;                                       //Latitude from GPS on Tracker transmitter (TX)
float TXLon;                                       //Longitude from GPS on Tracker transmitter (TX)
float TXAlt;                                       //Altitude from GPS on Tracker transmitter (TX)
uint8_t TXSats;                                    //number of GPS satellites seen (TX)
uint32_t TXHdop;                                   //HDOP from GPS on Tracker transmitter (TX)
uint16_t TXVolts;                                  //Volts (battery) level on this Tracker transmitter (TX)
uint32_t TXGPSFixTime;                             //GPS fix time in hot fix mode of GPS on Tracker transmitter (TX)
uint32_t TXPacketCount, TXErrorsCount;             //keep count of OK packets and send errors


void loop()
{

  if (gpsWaitFix(WaitGPSFixSeconds))
  {
    sendLocation(TXLat, TXLon, TXAlt, TXHdop, TXGPSFixTime);
    Serial.println();
    Serial.print(F("Waiting "));
    Serial.print(Sleepsecs);
    Serial.println(F("s"));
    delay(Sleepsecs * 1000);                        //this sleep is used to set overall transmission cycle time
  }
  else
  {
    send_Command(NoFix);                            //send notification of no GPS fix.
  }
}


bool gpsWaitFix(uint32_t waitSecs)
{
  //waits a specified number of seconds for a fix, returns true for good fix
  uint32_t startmS, waitmS, GPSonTime;
  bool GPSfix = false;
  float tempfloat;
  uint8_t GPSchar;

  GPSonTime = millis();

  Serial.print(F("Wait GPS Fix "));
  Serial.print(waitSecs);
  Serial.println(F("s"));
  Serial.flush();

  waitmS = waitSecs * 1000;                              //convert seconds wait into mS
  startmS = millis();

  digitalWrite(RED1, LOW);                               //LED on
  while ((uint32_t) (millis() - startmS) < waitmS)
  {
    if (Serial1.available() > 0)
    {
      GPSchar = Serial1.read();
      gps.encode(GPSchar);
      Serial.write(GPSchar);
    }

    if (gps.location.isUpdated() && gps.altitude.isUpdated())
    {
      digitalWrite(RED1, HIGH);                   //LED off
      GPSfix = true;
      Serial.flush();
      Serial.println();
      Serial.print(F("Have GPS Fix "));
      TXGPSFixTime = millis() - GPSonTime;
      Serial.print(TXGPSFixTime);
      Serial.print(F("mS > "));
      TXLat = gps.location.lat();
      TXLon = gps.location.lng();
      TXAlt = gps.altitude.meters();
      TXSats = gps.satellites.value();
      TXHdop = gps.hdop.value();
      tempfloat = ( (float) TXHdop / 100);

      Serial.print(TXLat, 5);
      Serial.print(F(","));
      Serial.print(TXLon, 5);
      Serial.print(F(","));
      Serial.print(TXAlt, 1);
      Serial.print(F(","));
      Serial.print(TXSats);
      Serial.print(F(","));
      Serial.print(tempfloat, 2);
      Serial.println();

      break;                                  //exit while loop reading GPS
    }
  }

  //if here then there has either been a fix or no fix and a timeout
  digitalWrite(RED1, HIGH);                   //LED off

  if (GPSfix)
  {
    setStatusByte(GPSFix, 1);                 //set status bit to flag a GPS fix
  }
  else
  {
    setStatusByte(GPSFix, 0);                 //set status bit to flag no fix
    Serial.println();
    Serial.println(F("Timeout - No GPSFix"));
    Serial.println();
    GPSfix = false;
  }

  return GPSfix;
}


void sendLocation(float Lat, float Lon, float Alt, uint32_t Hdop, uint32_t fixtime)
{
  uint8_t len;
  uint16_t IRQStatus;

  Serial.print(F("Send Location"));

  TXVolts = 3999;                               //test value

  LoRa.startWriteSXBuffer(0);                   //initialise buffer write at address 0
  LoRa.writeUint8(LocationPacket);              //identify type of packet
  LoRa.writeUint8(Broadcast);                   //who is the packet sent too
  LoRa.writeUint8(ThisNode);                    //tells receiver where is packet from
  LoRa.writeFloat(Lat);                         //add latitude
  LoRa.writeFloat(Lon);                         //add longitude
  LoRa.writeFloat(Alt);                         //add altitude
  LoRa.writeUint8(TXSats);                      //add number of satellites
  LoRa.writeUint32(Hdop);                       //add hdop
  LoRa.writeUint8(TXStatus);                    //add tracker status
  LoRa.writeUint32(fixtime);                    //add GPS fix time in mS
  LoRa.writeUint16(TXVolts);                    //add tracker supply volts
  LoRa.writeUint32(millis());                   //add uptime in mS
  len = LoRa.endWriteSXBuffer();                //close buffer write

  digitalWrite(RED1, LOW);
  TXPacketL = LoRa.transmitSXBuffer(0, len, 10000, TXpower, WAIT_TX);
  digitalWrite(RED1, HIGH);

  if (TXPacketL)
  {
    TXPacketCount++;
    Serial.println(F(" - Done "));
    Serial.print(F("SentOK,"));
    Serial.print(TXPacketCount);
    Serial.print(F(",Errors,"));
    Serial.println(TXErrorsCount);
  }
  else
  {
    //if here there was an error transmitting packet, will need to reset LoRa device
    TXErrorsCount++;
    IRQStatus = LoRa.readIrqStatus();                  //read the the interrupt register
    Serial.print(F(" SendError,"));
    Serial.print(F("Length,"));
    Serial.print(TXPacketL);                         //print transmitted packet length
    Serial.print(F(",IRQreg,"));
    Serial.print(IRQStatus, HEX);                    //print IRQ status
    LoRa.printIrqStatus();                             //prints the text of which IRQs set
    Serial.println();
    LoRa.resetDevice();
    LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);
    setStatusByte(LORAError, 1);
  }
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


void send_Command(char cmd)
{
  bool SendOK;
  uint8_t len;

  Serial.print(F("Send Cmd "));
  Serial.write(cmd);

  LoRa.startWriteSXBuffer(0);
  LoRa.writeUint8(cmd);                         //packet addressing used indentify type of packet
  LoRa.writeUint8(Broadcast);                   //who is the packet sent to
  LoRa.writeUint8(ThisNode);                    //where is packet from
  LoRa.writeUint16(TXVolts);
  len = LoRa.endWriteSXBuffer();

  digitalWrite(RED1, LOW);
  SendOK = LoRa.transmitSXBuffer(0, len, 10000, TXpower, WAIT_TX);   //timeout set at 10 seconds
  digitalWrite(RED1, HIGH);

  if (SendOK)
  {
    Serial.println(F(" - Done"));
  }
  else
  {
    Serial.println(F(" - Error"));
  }
}


void GPSTest(uint32_t testtime)
{
  uint32_t startmS;
  startmS = millis();

  while ( (uint32_t) (millis() - startmS) < testtime)       //allows for millis() overflow
  {
    if (Serial1.available() > 0)
    {
      Serial.write(Serial1.read());
    }
  }
  Serial.println();
  Serial.println();
  Serial.flush();
}



void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  //flash LED to show tracker is alive
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(RED1, LOW);
    delay(delaymS);
    digitalWrite(RED1, HIGH);
    delay(delaymS);
  }
}


void setup()
{
  uint16_t index, checkcount;

  pinMode(RED1, OUTPUT);                           //Red LED pin to output
  pinMode(GREEN1, OUTPUT);                         //Green LED pin to output
  pinMode(BLUE1, OUTPUT);                          //Blue LED pin to output

  digitalWrite(RED1, HIGH);                        //Red LED to off
  digitalWrite(GREEN1, HIGH);                      //Green LED to off
  digitalWrite(BLUE1, HIGH);                       //Blue LED to off

  led_Flash(2, 125);                                          //two quick LED flashes to indicate program start

  Serial.begin(115200);
  Serial.println();
  Serial.print(F(__FILE__));
  Serial.println();

  Serial.println(F("15_GPS_Tracker_Transmitter Starting"));

  SPI.begin();

  if (LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("LoRa Device found"));
    led_Flash(2, 125);
    delay(1000);
  }
  else
  {
    Serial.println(F("No device responding"));
    while (1)
    {
      led_Flash(50, 50);                          //long fast speed flash indicates device error
    }
  }

  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  Serial.println();
  LoRa.printModemSettings();                         //reads and prints the configured LoRa settings, useful check
  Serial.println();
  LoRa.printOperatingSettings();                     //reads and prints the configured operating settings, useful check
  Serial.println();

  Serial.println(F("Startup GPS check"));
  Serial1.begin(9600);
  GPSTest(5000);
  Serial.println();

  send_Command(PowerUp);                             //send power up command, includes supply mV

  Serial.print(F("Wait for first GPS fix "));
  Serial.print(WaitFirstGPSFixSeconds);
  Serial.println(F("seconds"));

  checkcount = WaitFirstGPSFixSeconds / 5;

  for (index = 1; index <= checkcount; index++)
  {
    gpsWaitFix(5);
    delay(250);                                               //so we can see LED blink
  }

  sendLocation(TXLat, TXLon, TXAlt, TXHdop, TXGPSFixTime);
}
