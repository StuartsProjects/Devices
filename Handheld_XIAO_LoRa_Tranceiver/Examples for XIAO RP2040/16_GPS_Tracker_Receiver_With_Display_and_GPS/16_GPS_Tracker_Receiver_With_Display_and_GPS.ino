/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 03/11/22
  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO RP2040.

  Program Operation -  This program is an example of a functional GPS tracker receiver for the XIAO Rp2040
  using LoRa. It is capable of picking up the trackers location packets from many kilometres away with only
  basic antennas.

  The program receives the location packets from the remote tracker transmitter and writes them on an OLED
  display and also prints the information to the Arduino IDE serial monitor. The program can read a locally
  attached GPS and when that has a fix, will display the distance and direction to the remote tracker.

  The program writes direct to the LoRa devices internal buffer, no memory buffer is used. The LoRa settings
  are configured in the Settings.h file.

  Most of the tracker information is shown on the OLED display. If there has been a tracker transmitter GPS fix
  the mumber\identifier of that tracker (the 'ThisNode' identifier in the transmitter settings) is shown on row 0
  right of screen, such as 'T1'. If the tracker transmitter does not have an upto date fix T? is shown. If there
  is a recent local (receiver) GPS fix then 'RG' is displayed row 1 right of screen. if there is no fix from the
  receivers GPS (or no GPS at all) then R? is shown.

  The program has the option of using a pin to control the power to the GPS, if the GPS module being used has this
  feature. To use the option change the define in Settings.h;

  '#define GPSPOWER -1' from -1 to the pin number being used. Also set the GPSONSTATE and GPSOFFSTATE defines to
  the appropriate logic levels.

  Uses the earlephilhower/arduino-pico: Raspberry Pi Pico Arduino core, for all RP2040 boards.
  Get it here > https://github.com/earlephilhower/arduino-pico
  Board selected is 'Seeed XIAO RP2040'

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <SPI.h>
#include <SX127XLT.h>                                       //get library here > https://github.com/StuartsProjects/SX12XX-LoRa
SX127XLT LoRa;

#include "Settings.h"
#include <ProgramLT_Definitions.h>

#include <U8x8lib.h>                                        //get library here > https://github.com/olikraus/u8g2 
U8X8_SSD1306_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);      //standard 0.96" SSD1306
//U8X8_SH1106_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);     //1.3" OLED often sold as 1.3" SSD1306


#include <TinyGPS++.h>                                      //get library here > http://arduiniana.org/libraries/tinygpsplus/
TinyGPSPlus gps;                                            //create the TinyGPS++ object


uint32_t RXpacketCount;        //count of received packets
uint8_t RXPacketL;             //length of received packet
int8_t  PacketRSSI;            //signal strength (RSSI) dBm of received packet
int8_t  PacketSNR;             //signal to noise ratio (SNR) dB of received packet
uint8_t PacketType;            //for packet addressing, identifies packet type
uint8_t Destination;           //for packet addressing, identifies the destination (receiving) node
uint8_t Source;                //for packet addressing, identifies the source (transmiting) node
uint8_t TXStatus;              //status byte from tracker transmitter
uint8_t TXSats;                //number of sattelites in use
float TXLat;                   //latitude
float TXLon;                   //longitude
float TXAlt;                   //altitude
float RXLat;                   //latitude
float RXLon;                   //longitude
float RXAlt;                   //altitude
uint32_t TXHdop;               //HDOP, indication of fix quality, horizontal dilution of precision, low is good
uint32_t TXGPSFixTime;         //time in mS for fix
uint16_t TXVolts;              //supply\battery voltage
uint16_t RXVolts;              //supply\battery voltage
float TXdistance;              //calculated distance to tracker
uint16_t TXdirection;          //calculated direction to tracker
uint16_t RXerrors;             //number of packets received with errors
uint32_t TXupTimemS;           //up time of TX in mS

uint32_t LastRXGPSfixCheck;    //used to record the time of the last GPS fix

bool TXLocation = false;       //set to true when at least one tracker location packet has been received
bool RXGPSfix = false;         //set to true if the local GPS has a recent fix

uint8_t FixCount = DisplayRate;  //used to keep track of number of GPS fixes before display updated


void loop()
{
  RXPacketL = LoRa.receiveSXBuffer(0, 0, NO_WAIT);   //returns 0 if packet error of some sort

  while (!digitalRead(DIO0))
  {
    readGPS();                                       //If the DIO pin is low, no packet arrived, so read the GPS
  }


  digitalWrite(RED1, LOW);                           //something has happened in receiver LED on

  RXPacketL = LoRa.readRXPacketL();
  PacketRSSI = LoRa.readPacketRSSI();
  PacketSNR = LoRa.readPacketSNR();

  if (RXPacketL == 0)
  {
    packet_is_Error();
  }
  else
  {
    packet_is_OK();
  }

  digitalWrite(RED1, HIGH);                           //LED off
  Serial.println();
}


void readGPS()
{
  uint8_t gpschar;
  if (Serial1.available() > 0)
  {
    gpschar = Serial1.read();
    gps.encode(gpschar);
    Serial.write(gpschar);
  }

  if ( (uint32_t) (millis() - LastRXGPSfixCheck) > NoRXGPSfixms)
  {
    RXGPSfix = false;
    LastRXGPSfixCheck = millis();
    dispscreen1();
  }

  if (gps.location.isUpdated() && gps.altitude.isUpdated())
  {
    RXGPSfix = true;
    RXLat = gps.location.lat();
    RXLon = gps.location.lng();
    RXAlt = gps.altitude.meters();
    printRXLocation();
    LastRXGPSfixCheck = millis();

    if ( FixCount == 1)                           //update screen when FIX count counts down from DisplayRate to 1
    {
      FixCount = DisplayRate;
      dispscreen1();
    }
    FixCount--;
  }
}


bool readTXStatus(byte bitnum)
{
  return bitRead(TXStatus, bitnum);
}


void printRXLocation()
{
  Serial.println();
  Serial.println();
  Serial.print(F("LocalGPS "));
  Serial.print(RXLat, 5);
  Serial.print(F(","));
  Serial.print(RXLon, 5);
  Serial.print(F(","));
  Serial.print(RXAlt, 1);
  Serial.println();
}


void readPacketAddressing()
{
  LoRa.startReadSXBuffer(0);
  PacketType = LoRa.readUint8();
  Destination = LoRa.readUint8();
  Source = LoRa.readUint8();
  LoRa.endReadSXBuffer();
}


void packet_is_OK()
{
  float tempfloat;

  RXpacketCount++;
  readPacketAddressing();

  if (PacketType == PowerUp)
  {
    LoRa.startReadSXBuffer(0);
    LoRa.readUint8();                                      //read byte from SXBuffer, not used
    LoRa.readUint8();                                      //read byte from SXBuffer, not used
    LoRa.readUint8();                                      //read byte from SXBuffer, not used
    TXVolts = LoRa.readUint16();                           //read tracker transmitter voltage
    LoRa.endReadSXBuffer();
    Serial.println();
    Serial.print(F("Tracker Powerup - Battery "));
    Serial.print(TXVolts);
    Serial.println(F("mV"));
    Serial.println();
    dispscreen2();
  }

  if (PacketType == LocationPacket)
  {
    //packet has been received, now read from the SX12XX FIFO in the correct order.
    Serial.println();
    Serial.println();
    Serial.print(F("LocationPacket "));
    TXLocation = true;
    LoRa.startReadSXBuffer(0);                //start the read of received packet
    PacketType = LoRa.readUint8();            //read in the PacketType
    Destination = LoRa.readUint8();           //read in the Packet destination address
    Source = LoRa.readUint8();                //read in the Packet source address
    TXLat = LoRa.readFloat();                 //read in the tracker latitude
    TXLon = LoRa.readFloat();                 //read in the tracker longitude
    TXAlt = LoRa.readFloat();                 //read in the tracker altitude
    TXSats = LoRa.readUint8();                //read in the satellites in use by tracker GPS
    TXHdop = LoRa.readUint32();               //read in the HDOP of tracker GPS
    TXStatus = LoRa.readUint8();              //read in the tracker status byte
    TXGPSFixTime = LoRa.readUint32();         //read in the last fix time of tracker GPS
    TXVolts = LoRa.readUint16();              //read in the tracker supply\battery volts
    TXupTimemS = LoRa.readUint32();           //read in the TX uptime in mS
    RXPacketL = LoRa.endReadSXBuffer();       //end the read of received packet


    if (RXGPSfix)                           //if there has been a local GPS fix do the distance and direction calculation
    {
      TXdirection = (int16_t) TinyGPSPlus::courseTo(RXLat, RXLon, TXLat, TXLon);
      TXdistance = TinyGPSPlus::distanceBetween(RXLat, RXLon, TXLat, TXLon);
    }
    else
    {
      TXdistance = 0;
      TXdirection = 0;
    }

    Serial.write(PacketType);
    Serial.write(Destination);
    Serial.write(Source);
    Serial.print(F(","));
    Serial.print(TXLat, 6);
    Serial.print(F(","));
    Serial.print(TXLon, 6);
    Serial.print(F(","));
    Serial.print(TXAlt, 1);
    Serial.print(F(","));
    Serial.print(TXSats);
    Serial.print(F(","));

    tempfloat = ( (float) TXHdop / 100);           //need to convert Hdop read from GPS as uint32_t to a float for display
    Serial.print(tempfloat, 2);

    Serial.print(F(","));
    Serial.print(TXStatus);
    Serial.print(F(","));

    Serial.print(TXGPSFixTime);
    Serial.print(F("mS,"));
    Serial.print(TXVolts);
    Serial.print(F("mV,"));
    Serial.print((TXupTimemS / 1000));
    Serial.print(F("s,"));

    Serial.print(TXdistance, 0);
    Serial.print(F("m,"));
    Serial.print(TXdirection);
    Serial.print(F("d"));
    printpacketDetails();
    Serial.println();
    dispscreen1();                                  //and show the packet detail it on screen
    return;
  }

  if (PacketType == NoFix)
  {
    Serial.println();
    Serial.print(F("No tracker GPS fix"));
    Serial.println();
  }
}


void printpacketDetails()
{
  uint16_t IRQStatus;
  Serial.print(F(",RSSI,"));
  Serial.print(PacketRSSI);
  Serial.print(F("dBm,SNR,"));
  Serial.print(PacketSNR);
  Serial.print(F("dB,Packets,"));
  Serial.print(RXpacketCount);

  Serial.print(F(",Length,"));
  Serial.print(RXPacketL);
  IRQStatus = LoRa.readIrqStatus();
  Serial.print(F(",IRQreg,"));
  Serial.print(IRQStatus, HEX);
}


void packet_is_Error()
{
  uint16_t IRQStatus;

  IRQStatus = LoRa.readIrqStatus();                    //get the IRQ status
  RXerrors++;
  Serial.print(F("PacketError,RSSI"));

  Serial.print(PacketRSSI);
  Serial.print(F("dBm,SNR,"));
  Serial.print(PacketSNR);

  Serial.print(F("dB,Length,"));
  Serial.print(LoRa.readRXPacketL());                  //get the real packet length
  Serial.print(F(",IRQreg,"));
  Serial.print(IRQStatus, HEX);
  LoRa.printIrqStatus();
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(RED1, LOW);
    delay(delaymS);
    digitalWrite(RED1, HIGH);
    delay(delaymS);
  }
}


void dispscreen1()
{
  //show received packet data on display
  float tempfloat;
  disp.clearLine(0);
  disp.setCursor(0, 0);
  disp.print(TXLat, 6);
  disp.setCursor(14, 0);

  if (readTXStatus(GPSFix))
  {
    disp.print(F("T"));
    disp.write(Source);
  }
  else
  {
    disp.print(F("T?"));
  }

  disp.clearLine(1);
  disp.setCursor(0, 1);
  disp.print(TXLon, 6);

  disp.setCursor(14, 1);
  if (RXGPSfix)
  {
    disp.print(F("RG"));
  }
  else
  {
    disp.print(F("R?"));
  }

  disp.clearLine(2);
  disp.setCursor(0, 2);
  disp.print(TXAlt, 0);
  disp.print(F("m"));
  disp.clearLine(3);
  disp.setCursor(0, 3);

  disp.print(F("RSSI "));
  disp.print(PacketRSSI);
  disp.print(F("dBm"));
  disp.clearLine(4);
  disp.setCursor(0, 4);
  disp.print(F("SNR  "));

  if (PacketSNR > 0)
  {
    disp.print(F("+"));
  }

  if (PacketSNR == 0)
  {
    disp.print(F(" "));
  }

  if (PacketSNR < 0)
  {
    disp.print(F("-"));
  }

  disp.print(PacketSNR);
  disp.print(F("dB"));

  if (PacketType == LocationPacket)
  {
    disp.clearLine(5);
    disp.setCursor(0, 5);
    tempfloat = ((float) TXVolts / 1000);
    disp.print(F("Batt "));
    disp.print(tempfloat, 2);
    disp.print(F("v"));
  }

  disp.clearLine(6);
  disp.setCursor(0, 6);
  disp.print(F("Packets "));
  disp.print(RXpacketCount);

  disp.clearLine(7);

  if (RXGPSfix && TXLocation)           //only display distance and direction if have received tracker packet and have local GPS fix
  {
    disp.clearLine(7);
    disp.setCursor(0, 7);
    disp.print(TXdistance, 0);
    disp.print(F("m "));
    disp.print(TXdirection);
    disp.print(F("d"));
  }
}


void dispscreen2()
{
  //show tracker powerup data on display
  float tempfloat;
  disp.clear();
  disp.setCursor(0, 0);
  disp.print(F("Tracker Powerup"));
  disp.setCursor(0, 1);
  disp.print(F("Battery "));
  tempfloat = ((float) TXVolts / 1000);
  disp.print(tempfloat, 2);
  disp.print(F("v"));
}


void GPSTest()
{
  uint32_t startmS;
  startmS = millis();

  while ( (uint32_t) (millis() - startmS) < 2000)       //allows for millis() overflow
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


void setup()
{
  pinMode(RED1, OUTPUT);                           //Red LED pin to output
  pinMode(GREEN1, OUTPUT);                         //Green LED pin to output
  pinMode(BLUE1, OUTPUT);                          //Blue LED pin to output

  digitalWrite(RED1, HIGH);                        //Red LED to off
  digitalWrite(GREEN1, HIGH);                      //Green LED to off
  digitalWrite(BLUE1, HIGH);                       //Blue LED to off

  led_Flash(2, 125);                               //two quick LED flashes to indicate program start

  Serial.begin(115200);
  Serial.println();
  Serial.print(F(__FILE__));
  Serial.println();

  Serial.println(F("16_GPS_Tracker_Receiver_With_Display_and_GPS Starting"));

  SPI.begin();

  disp.begin();
  disp.setFont(u8x8_font_chroma48medium8_r);

  Serial.print(F("Checking LoRa device - "));         //Initialize LoRa
  disp.setCursor(0, 0);

  if (LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("Receiver ready"));
    disp.print(F("Receiver ready"));
    led_Flash(2, 125);
    delay(1000);
  }
  else
  {
    Serial.println(F("No LoRa device responding"));
    disp.print(F("No LoRa device"));
    while (1)
    {
      led_Flash(50, 50);                               //long fast speed flash indicates device error
    }
  }

  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  Serial.println();
  Serial.println(F("Startup GPS check"));

  if (GPSPOWER >= 0)
  {
    pinMode(GPSPOWER, OUTPUT);
    digitalWrite(GPSPOWER, GPSONSTATE);                 //power up GPS
  }

  Serial1.begin(GPSBaud);                               //serial for GPS
  GPSTest();

  Serial.println();
  Serial.println();

  Serial.println(F("Receiver ready"));
  Serial.println();
}
