/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 06/02/23

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation -  This program is an example of a basic GPS tracker. The program reads the GPS,
  waits for an updated fix and transmits location, altitude, number of satellites in view.
  
  At startup there should be at least a couple of seconds of recognisable text from the GPS printed to
  the serial monitor. If you see garbage or funny characters its likley the GPS baud rate is wrong.
  Outside with a good view of the sky most GPSs should produce a location fix in around 45 seconds.

  The program writes direct to the LoRa devices internal buffer, no memory buffer is used.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <SPI.h>
#include <SX127XLT.h>                              //get library here > https://github.com/StuartsProjects/SX12XX-LoRa

//#include "Settings.h"
#include <ProgramLT_Definitions.h>

SX127XLT LoRa;

#include <TinyGPS++.h>                             //get library here > http://arduiniana.org/libraries/tinygpsplus/
TinyGPSPlus gps;                                   //create the TinyGPS++ object


//*******  Setup hardware pin definitions here ! ***************
//These are the pin definitions for the Seeeduino XIAO SAMD21. 

#define NSS A3                                  //select pin on LoRa device
#define NRESET A2                               //reset pin on LoRa device
#define DIO0 A1                                 //DIO0 pin on LoRa device, used for sensing RX and TX done 
#define LED1 13                                 //on board LED, yellow  
#define ADMultiplier 1.628                      //adjustment to convert AD value read into mV of battery voltage 
#define SupplyAD A0                             //Resistor divider for battery voltage read connected here 

#define LORA_DEVICE DEVICE_SX1278               //this is the device we are using 

//*******  Setup LoRa Parameters Here ! ***************

//LoRa Modem Parameters
const uint32_t Frequency = 434000000;           //frequency of transmissions
const uint32_t Offset = 0;                      //offset frequency for calibration purposes

const uint8_t Bandwidth = LORA_BW_125;          //LoRa bandwidth
const uint8_t SpreadingFactor = LORA_SF7;       //LoRa spreading factor
const uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting
const int8_t TXpower = 10;                      //LoRa transmit power

#define ThisNode '1'                            //a character that identifies this tracker

//**************************************************************************************************
// GPS Settings
//**************************************************************************************************

#define GPSBaud 9600                               //GPS Baud rate   

#define WaitGPSFixSeconds 30                       //time in seconds to wait for a new GPS fix 
#define WaitFirstGPSFixSeconds 120                 //time to seconds to wait for the first GPS fix at startup
#define Sleepsecs 15                               //seconds between transmissions, this delay is used to set overall transmission cycle time

uint8_t TXStatus = 0;                              //used to store current status flag bits of Tracker transmitter (TX)
uint8_t TXPacketL;                                 //length of LoRa packet (TX)
float TXLat;                                       //Latitude from GPS on Tracker transmitter (TX)
float TXLon;                                       //Longitude from GPS on Tracker transmitter (TX)
float TXAlt;                                       //Altitude from GPS on Tracker transmitter (TX)
uint8_t TXSats;                                    //number of GPS satellites seen (TX)
uint16_t TXVolts;                                  //Volts (battery) level on this Tracker transmitter (TX)
uint32_t TXPacketCount, TXErrorsCount;             //keep count of OK packets and send errors


void loop()
{

  if (gpsWaitFix(WaitGPSFixSeconds))
  {
    sendLocation(TXLat, TXLon, TXAlt, TXSats);
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
  uint8_t GPSchar;

  GPSonTime = millis();

  Serial.print(F("Wait GPS Fix "));
  Serial.print(waitSecs);
  Serial.println(F("s"));
  Serial.flush();

  waitmS = waitSecs * 1000;                              //convert seconds wait into mS
  startmS = millis();

  digitalWrite(LED1, LOW);                               //LED on
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
      digitalWrite(LED1, HIGH);                          //LED off
      GPSfix = true;
      Serial.flush();
      Serial.println();
      Serial.print(F("Have GPS Fix "));
      Serial.print(millis() - GPSonTime);
      Serial.print(F("mS > "));
      TXLat = gps.location.lat();
      TXLon = gps.location.lng();
      TXAlt = gps.altitude.meters();
      TXSats = gps.satellites.value();
 
      Serial.print(TXLat, 5);
      Serial.print(F(","));
      Serial.print(TXLon, 5);
      Serial.print(F(","));
      Serial.print(TXAlt, 1);
      Serial.print(F(","));
      Serial.print(TXSats);
      Serial.println();

      break;                                  //exit while loop reading GPS
    }
  }

  //if here then there has either been a fix or no fix causing a timeout
  digitalWrite(LED1, HIGH);                   //LED off

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


void sendLocation(float Lat, float Lon, float Alt, uint8_t Sats)
{
  uint8_t len;
  uint16_t IRQStatus;

  Serial.print(F("Send Location"));

  TXVolts = readSupplyVoltage();

  LoRa.startWriteSXBuffer(0);                   //initialise buffer write at address 0
  LoRa.writeUint8(LocationPacket);              //identify type of packet
  LoRa.writeUint8(Broadcast);                   //who is the packet sent too
  LoRa.writeUint8(ThisNode);                    //tells receiver where is packet from
  LoRa.writeFloat(Lat);                         //add latitude
  LoRa.writeFloat(Lon);                         //add longitude
  LoRa.writeFloat(Alt);                         //add altitude
  LoRa.writeUint8(TXSats);                      //add number of satellites
  LoRa.writeUint8(TXStatus);                    //add tracker status
  LoRa.writeUint16(TXVolts);                    //add tracker supply volts
  len = LoRa.endWriteSXBuffer();                //close buffer write

  digitalWrite(LED1, LOW);
  TXPacketL = LoRa.transmitSXBuffer(0, len, 10000, TXpower, WAIT_TX);
  digitalWrite(LED1, HIGH);

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
    IRQStatus = LoRa.readIrqStatus();                //read the the interrupt register
    Serial.print(F(" SendError,"));
    Serial.print(F("Length,"));
    Serial.print(TXPacketL);                         //print transmitted packet length
    Serial.print(F(",IRQreg,"));
    Serial.print(IRQStatus, HEX);                    //print IRQ status
    LoRa.printIrqStatus();                           //prints the text of which IRQs set
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

  digitalWrite(LED1, LOW);
  SendOK = LoRa.transmitSXBuffer(0, len, 10000, TXpower, WAIT_TX);   //timeout set at 10 seconds
  digitalWrite(LED1, HIGH);

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


uint16_t readSupplyVoltage()
{
  //returns supply in mV @ 10mV per AD bit read
  uint16_t temp;
  uint16_t volts = 0;
  byte index;

  analogReadResolution(ADC_RESOLUTION);
  pinMode(SupplyAD, INPUT);
  temp = analogRead(SupplyAD);              //do a null read to start

  for (index = 0; index <= 4; index++)      //sample AD 5 times
  {
    temp = analogRead(SupplyAD);
    volts = volts + temp;
  }
  volts = ((volts / 5) * ADMultiplier);

  return volts;
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  //flash LED to show tracker is alive
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, LOW);
    delay(delaymS);
    digitalWrite(LED1, HIGH);
    delay(delaymS);
  }
}


void setup()
{
  uint16_t index, checkcount;

  pinMode(LED1, OUTPUT);                                      //setup pin as output for indicator LED
  led_Flash(2, 125);                                          //two quick LED flashes to indicate program start

  Serial.begin(115200);
  Serial.println();
  Serial.print(F("20_GPS_Tracker_Transmitter"));
  Serial.println();

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
      led_Flash(50, 50);                            //long fast speed flash indicates device error
    }
  }

  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);
  pinMode(DIO0, INPUT_PULLDOWN);    //prevent A1 floating if diodes in use as per TTN setup
  
  Serial.println();
  LoRa.printModemSettings();                        //reads and prints the configured LoRa settings, useful check
  Serial.println();
  LoRa.printOperatingSettings();                    //reads and prints the configured operating settings, useful check
  Serial.println();

  Serial.println(F("Startup GPS check"));
  Serial1.begin(9600);
  GPSTest(5000);
  Serial.println();

  TXVolts = readSupplyVoltage();
  Serial.print(F("Supply Volts "));
  Serial.print(readSupplyVoltage());
  Serial.println(F("mV"));
  Serial.println();

  send_Command(PowerUp);                           //send power up command, includes supply mV

  Serial.print(F("Wait for first GPS fix "));
  Serial.print(WaitFirstGPSFixSeconds);
  Serial.println(F("seconds"));

  checkcount = WaitFirstGPSFixSeconds / 5;

  for (index = 1; index <= checkcount; index++)
  {
    gpsWaitFix(5);
    delay(250);                                               //so we can see LED blink
  }

  sendLocation(TXLat, TXLon, TXAlt, TXSats);
}
