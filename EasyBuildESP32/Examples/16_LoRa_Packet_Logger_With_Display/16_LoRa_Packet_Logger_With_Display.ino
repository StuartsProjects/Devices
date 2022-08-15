/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 14/08/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - The program listens for incoming packets using the LoRa settings in the 'Settings.h'
  file. The pins to access the LoRa device need to be defined in the 'Settings.h' file also.

  There is a printout to serial monitor of the hex values of a valid valid packet received.

  The display will show the number of packets received OK, those with errors and the RSSI and SNR of the
  last received packet. At startup the display will show the set frequency, LoRa spreading factor, LoRa
  bandwidth and the SD logfile name used, if an SD card is detected.

  The SD card in MMC mode uses pin 2 so the card will need to be removed to allow initial program upload.

  Sample serial monitor output;

  122s  RSSI,-48dBm,SNR,9dB,Length,23,Packets,1,Errors,0,IRQreg,50  48 65 6C .........

  If there is a packet error it might look like this, which is showing a CRC error,

  127s PacketError, RSSI,-116dBm,SNR,-11dB,Length,23,Packets,5,Errors,1,IRQreg,70,IRQ_HEADER_VALID,IRQ_CRC_ERROR,IRQ_RX_DONE

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <SD_MMC.h>
File logFile;

#include <SPI.h>                                   //the LoRa device is SPI based so load the SPI library
#include <SX127XLT.h>                              //include the appropriate LoRa library   
#include "Settings.h"                              //include the settings file, frequencies, LoRa settings etc   
SX127XLT LoRa;                                     //create a library class instance called LoRa

#include <TimeLib.h>                               //get the library here > https://github.com/PaulStoffregen/Time
time_t recordtime;                                 //used to record the current time

#include <U8x8lib.h>                                      //get library here >  https://github.com/olikraus/u8g2 
//U8X8_SSD1306_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);    //use this line for standard 0.96" SSD1306
U8X8_SH1106_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);   //use this line for 1.3" OLED often sold as 1.3" SSD1306
#define DEFAULTFONT u8x8_font_chroma48medium8_r           //font for U8X8 Library

#include <Adafruit_NeoPixel.h>                    //get library here > https://github.com/adafruit/Adafruit_NeoPixel
#define NPIXEL 33                                 //WS2811 neopixels on this pin
#define NUMPIXELS 3                               //How many NeoPixels are attached
#define BRIGHTNESS 25                             //LED brightness 0 to 255
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NPIXEL, NEO_GRB + NEO_KHZ800); //get library here > https://github.com/adafruit/Adafruit_NeoPixel

uint32_t RXpacketCount;
uint32_t RXpacketErrors;
uint16_t IRQStatus;

uint8_t RXBUFFER[RXBUFFER_SIZE];                   //create the buffer that received packets are copied into
uint8_t RXPacketL;                                 //stores length of packet received
int16_t PacketRSSI;                                //stores RSSI of received packet
int8_t  PacketSNR;                                 //stores signal to noise ratio of received packet

char filename[] = "/Log0000.txt";                  //base name for logfile
bool SD_Found = false;                             //variable set if SD card found at program startup
uint16_t lognumber;


void loop()
{
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));                 //all colours off
  pixels.show();

  RXPacketL = LoRa.receive(RXBUFFER, RXBUFFER_SIZE, RXtimeoutmS, WAIT_RX);

  PacketRSSI = LoRa.readPacketRSSI();              //read the recived RSSI value
  PacketSNR = LoRa.readPacketSNR();                //read the received SNR value
  IRQStatus = LoRa.readIrqStatus();                //read the LoRa device IRQ status register

  printElapsedTime();                              //print seconds to monitor

  if (RXPacketL == 0)                              //if the LoRa.receive() function detects an error, RXpacketL == 0
  {
    pixels.setPixelColor(0, pixels.Color(0, BRIGHTNESS, 0)); //Red on
    pixels.show();

    packet_is_Error();

    if (SD_Found)
    {
      printElapsedTimeSD();
      packet_is_ErrorSD();
    }

    pixels.setPixelColor(0, pixels.Color(0, 0, 0)); //all off
    pixels.show();

  }
  else
  {
    pixels.setPixelColor(0, pixels.Color(BRIGHTNESS, 0, 0)); //Green on
    pixels.show();

    packet_is_OK();

    if (SD_Found)
    {
      printElapsedTimeSD();
      packet_is_OKSD();
    }

    pixels.setPixelColor(0, pixels.Color(0, 0, 0)); //all off
    pixels.show();

  }
}


void packet_is_OK()
{
  RXpacketCount++;

  Serial.print(F(" RSSI,"));
  Serial.print(PacketRSSI);
  Serial.print(F("dBm,SNR,"));
  Serial.print(PacketSNR);
  Serial.print(F("dB,Length,"));
  Serial.print(RXPacketL);
  Serial.print(F(",Packets,"));
  Serial.print(RXpacketCount);
  Serial.print(F(",Errors,"));
  Serial.print(RXpacketErrors);
  Serial.print(F(",IRQreg,"));
  Serial.print(IRQStatus, HEX);
  Serial.print(F("  "));
  printHEXPacket(RXBUFFER, RXPacketL);              //print the HEX values of packet
  Serial.println();
  disp.clearLine(0);
  disp.setCursor(0, 0);
  disp.print(F("OK "));
  dispscreen1();
}


void printHEXPacket(uint8_t *buffer, uint8_t size)
{
  uint8_t index;

  for (index = 0; index < size; index++)
  {
    printHEXByte(buffer[index]);
    Serial.print(F(" "));
  }
}


void printHEXByte(uint8_t temp)
{
  if (temp < 0x10)
  {
    Serial.print(F("0"));
  }
  Serial.print(temp, HEX);
}


void packet_is_Error()
{
  if (IRQStatus & IRQ_RX_TIMEOUT)
  {
    Serial.println(F(" RXTimeout"));
    disp.clearLine(0);
    disp.setCursor(0, 0);
    disp.print(F("RXTimeout"));
    return;
  }

  RXpacketErrors++;
  Serial.print(F(" PacketError"));
  Serial.print(F(",RSSI,"));
  Serial.print(PacketRSSI);
  Serial.print(F("dBm,SNR,"));
  Serial.print(PacketSNR);
  Serial.print(F("dB,Length,"));
  Serial.print(LoRa.readRXPacketL());               //get the real packet length
  Serial.print(F(",Packets,"));
  Serial.print(RXpacketCount);
  Serial.print(F(",Errors,"));
  Serial.print(RXpacketErrors);
  Serial.print(F(",IRQreg,"));
  Serial.print(IRQStatus, HEX);
  LoRa.printIrqStatus();                            //print the names of the IRQ registers set
  disp.clearLine(0);
  disp.setCursor(0, 0);
  disp.print(F("Packet Error"));
  dispscreen1();
}


void printElapsedTime()
{
  float seconds;
  seconds = millis() / 1000;
  Serial.print(seconds, 0);
  Serial.print(F("s "));
}


void dispscreen1()
{
  disp.clearLine(1);
  disp.setCursor(0, 1);
  disp.print(F("Pkts "));
  disp.print(RXpacketCount);
  disp.print(F(" Ers "));
  disp.print(RXpacketErrors);
  disp.clearLine(2);
  disp.setCursor(0, 2);
  disp.print(F("Length "));
  disp.print(RXPacketL);
  disp.clearLine(3);
  disp.setCursor(0, 3);
  disp.print(F("RSSI "));
  disp.print(PacketRSSI);
  disp.print(F("dBm"));
  disp.clearLine(4);
  disp.setCursor(0, 4);

  disp.print(F("SNR "));

  if (PacketSNR > 0)
  {
    disp.print(F("+"));
  }

  disp.print(PacketSNR);
  disp.print(F("dB"));
}


void dispscreen2()
{
  disp.clear();
  disp.setCursor(0, 0);
  disp.print(F("Freq "));
  disp.print(Frequency);
  disp.setCursor(0, 1);
  disp.print(F("BW "));
  disp.print(LoRa.returnBandwidth(Bandwidth));
  disp.setCursor(0, 2);
  disp.print(F("SF "));
  disp.print(SpreadingFactor);

  disp.setCursor(0, 7);

  if (SD_Found)
  {
    //disp.print(F("Log "));
    disp.print(filename);
  }
  else
  {
    disp.print(F("No SD card"));
  }
}


//******************************************************
//SD logger routines
//******************************************************

void printElapsedTimeSD()
{
  float seconds;
  seconds = millis() / 1000;
  logFile.print(seconds, 0);
  logFile.print(F("s"));
}


void printModemSettingsSD()
{
  uint8_t regdata;
  uint8_t sf;
  uint32_t bandwidth;
  uint8_t cr;
  uint8_t opt;
  uint16_t syncword;
  uint8_t  invertIQ;
  uint16_t preamble;
  uint32_t freqint;

  if (LORA_DEVICE == DEVICE_SX1272)
  {
    regdata = (LoRa.readRegister(REG_MODEMCONFIG1) & READ_BW_AND_2);
  }
  else
  {
    regdata = (LoRa.readRegister(REG_MODEMCONFIG1) & READ_BW_AND_X);
  }

  //get al the data frome the lora device in one go to avoid swapping
  //devices on the SPI bus all the time

  if (LORA_DEVICE == DEVICE_SX1272)
  {
    regdata = (LoRa.readRegister(REG_MODEMCONFIG1) & READ_BW_AND_2);
  }
  else
  {
    regdata = (LoRa.readRegister(REG_MODEMCONFIG1) & READ_BW_AND_X);
  }

  bandwidth = LoRa.returnBandwidth(regdata);
  freqint = LoRa.getFreqInt();
  sf = LoRa.getLoRaSF();
  cr = LoRa.getLoRaCodingRate();
  opt = LoRa.getOptimisation();
  syncword = LoRa.getSyncWord();
  invertIQ = LoRa.getInvertIQ();
  preamble = LoRa.getPreamble();

  printDeviceSD();
  logFile.print(F(", "));
  logFile.print(freqint);
  logFile.print(F("hz, SF"));
  logFile.print(sf);

  logFile.print(F(", BW"));
  logFile.print(bandwidth);

  logFile.print(F(", CR4: "));
  logFile.print(cr);
  logFile.print(F(", LDRO_"));

  if (opt)
  {
    logFile.print(F("On"));
  }
  else
  {
    logFile.print(F("Off"));
  }

  logFile.print(F(", SyncWord_0x"));
  logFile.print(syncword, HEX);

  if (invertIQ == LORA_IQ_INVERTED)
  {
    logFile.print(F(", IQInverted"));
  }
  else
  {
    logFile.print(F(", IQNormal"));
  }
  logFile.print(F(", Preamble_"));
  logFile.print(preamble);
  logFile.flush();
}


void printOperatingSettingsSD()
{
  //get al the data frome the lora device in one go to avoid swapping
  //devices on the SPI bus all the time

  uint8_t ver = LoRa.getVersion();
  uint8_t pm = LoRa.getPacketMode();
  uint8_t hm = LoRa.getHeaderMode();
  uint8_t crcm = LoRa.getCRCMode();
  uint8_t agc = LoRa.getAGC();
  uint8_t lnag = LoRa.getLNAgain();
  uint8_t boosthf = LoRa.getLNAboostHF();
  uint8_t boostlf = LoRa.getLNAboostLF();
  uint8_t opmode = LoRa.getOpmode();

  printDeviceSD();
  logFile.print(F(", "));

  printOperatingModeSD(opmode);

  logFile.print(F(", Version_"));
  logFile.print(ver, HEX);

  logFile.print(F(", PacketMode_"));

  if (pm)
  {
    logFile.print(F("LoRa"));
  }
  else
  {
    logFile.print(F("FSK"));
  }

  if (hm)
  {
    logFile.print(F(", Implicit"));
  }
  else
  {
    logFile.print(F(", Explicit"));
  }

  logFile.print(F(", CRC_"));
  if (crcm)
  {
    logFile.print(F("On"));
  }
  else
  {
    logFile.print(F("Off"));
  }

  logFile.print(F(", AGCauto_"));
  if (agc)
  {
    logFile.print(F("On"));
  }
  else
  {
    logFile.print(F("Off"));
  }

  logFile.print(F(", LNAgain_"));
  logFile.print(lnag);

  logFile.print(F(", LNAboostHF_"));
  if (boosthf)
  {
    logFile.print(F("On"));
  }
  else
  {
    logFile.print(F("Off"));
  }

  logFile.print(F(", LNAboostLF_"));
  if (boostlf)
  {
    logFile.print(F("On"));
  }
  else
  {
    logFile.print(F("Off"));
  }
  logFile.flush();
}


void printOperatingModeSD(uint8_t opmode)
{
  switch (opmode)
  {
    case 0:
      logFile.print(F("SLEEP"));
      break;

    case 1:
      logFile.print(F("STDBY"));
      break;

    case 2:
      logFile.print(F("FSTX"));
      break;

    case 3:
      logFile.print(F("TX"));
      break;

    case 4:
      logFile.print(F("FSRX"));
      break;

    case 5:
      logFile.print(F("RXCONTINUOUS"));
      break;

    case 6:
      logFile.print(F("RXSINGLE"));
      break;

    case 7:
      logFile.print(F("CAD"));
      break;

    default:
      logFile.print(F("NOIDEA"));
      break;
  }
}


uint16_t setupSDLOG(char *buff)
{
  //creats a new filename

  uint16_t index;

  File dir;

  Serial.print(F("SD card..."));

  for (index = 1; index <= 9999; index++)
  {
    buff[4] = index / 1000 + '0';
    buff[5] = ((index % 1000) / 100) + '0';
    buff[6] = ((index % 100) / 10) + '0';
    buff[7] = index % 10 + '0' ;

    if (! SD_MMC.exists(filename))
    {
      // only open a new file if it doesn't exist
      dir = SD_MMC.open(buff, FILE_WRITE);
      break;
    }
  }

  dir.rewindDirectory();                          //stops SD_MMC.exists() command causing issues with directory listings
  dir.close();

  Serial.print(F("Writing to "));
  Serial.println(buff);

  logFile = SD_MMC.open(filename, FILE_APPEND);

  return index;                                   //return number of log file created
}



void printDeviceSD()
{

  switch (LORA_DEVICE)
  {
    case DEVICE_SX1272:
      logFile.print(F("SX1272"));
      break;

    case DEVICE_SX1276:
      logFile.print(F("SX1276"));
      break;

    case DEVICE_SX1277:
      logFile.print(F("SX1277"));
      break;

    case DEVICE_SX1278:
      logFile.print(F("SX1278"));
      break;

    case DEVICE_SX1279:
      logFile.print(F("SX1279"));
      break;

    default:
      logFile.print(F("Unknown Device"));
  }
}


void printHEXPacketSD(uint8_t *buffer, uint8_t size)
{
  uint8_t index;

  for (index = 0; index < size; index++)
  {
    printHEXByteSD(buffer[index]);
    logFile.print(F(" "));
  }
}


void printHEXByteSD(uint8_t temp)
{
  if (temp < 0x10)
  {
    logFile.print(F("0"));
  }
  logFile.print(temp, HEX);
}


void packet_is_OKSD()
{
  IRQStatus = LoRa.readIrqStatus();

  logFile.print(F(" RSSI,"));
  logFile.print(PacketRSSI);
  logFile.print(F("dBm,SNR,"));
  logFile.print(PacketSNR);
  logFile.print(F("dB,Length,"));
  logFile.print(RXPacketL);
  logFile.print(F(",Packets,"));
  logFile.print(RXpacketCount);
  logFile.print(F(",Errors,"));
  logFile.print(RXpacketErrors);
  logFile.print(F(",IRQreg,"));
  logFile.print(IRQStatus, HEX);
  logFile.print(F(",FreqErrror,"));
  logFile.print(LoRa.getFrequencyErrorHz());
  logFile.print(F("hz  "));
  printHEXPacketSD(RXBUFFER, RXPacketL);
  logFile.println();
  logFile.flush();
}


void packet_is_ErrorSD()
{
  IRQStatus = LoRa.readIrqStatus();                    //get the IRQ status
  RXPacketL = LoRa.readRXPacketL();                    //get the real packet length

  if (IRQStatus & IRQ_RX_TIMEOUT)
  {
    logFile.println(F(" RXTimeout"));
  }
  else
  {
    logFile.print(F(" PacketError"));
    logFile.print(F(",RSSI,"));
    logFile.print(PacketRSSI);
    logFile.print(F("dBm,SNR,"));
    logFile.print(PacketSNR);
    logFile.print(F("dB,Length,"));
    logFile.print(RXPacketL);
    logFile.print(F(",Packets,"));
    logFile.print(RXpacketCount);
    logFile.print(F(",Errors,"));
    logFile.print(RXpacketErrors);
    logFile.print(F(",IRQreg,"));
    logFile.println(IRQStatus, HEX);
  }
  logFile.flush();
}


void printDigitsSD(int8_t digits)
{
  //utility function for digital clock display: prints preceding colon and leading 0
  logFile.print(F(": "));
  if (digits < 10)
    logFile.print('0');
  logFile.print(digits);
}


void printtimeSD()
{
  logFile.print(hour(recordtime));
  printDigitsSD(minute(recordtime));
  printDigitsSD(second(recordtime));
}


void setup()
{
  pinMode(2, INPUT_PULLUP);                                  //needed for MMC card to work

  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.println();
  Serial.println(__TIME__);
  Serial.println();
  Serial.println(__DATE__);
  Serial.println();

  if (SD_MMC.begin("/sdcard", true))                          //mount card in 1 bit MMC mode
  {
    SD_Found = true;
    Serial.println("MMC Card Mount OK");
    lognumber = setupSDLOG(filename);
  }
  else
  {
    Serial.println();
    Serial.println("ERROR MMC Card Mount Failed");
    Serial.println();
    lognumber = 0;
  }

  disp.begin();
  disp.setFont(DEFAULTFONT);
  disp.clear();
  disp.setCursor(0, 0);
  disp.print(F("Check LoRa"));

  SPI.begin(SCK, MISO, MOSI, NSS);

  //SPI beginTranscation is normally part of library routines, but if it is disabled in library
  //a single instance is needed here, so uncomment the program line below
  //SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  //setup hardware pins used by device, then check if device is found
  if (LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    disp.setCursor(0, 1);
    disp.print(F("LoRa OK"));
    Serial.println(F("LoRa Device found"));
  }
  else
  {
    disp.print(F("No LoRa device"));
    Serial.println(F("No LoRa device responding"));
    while (1)
    {
      disp.print(F("Device error"));
    }
  }

  //this function call sets up the device for LoRa using the settings from settings.h
  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  Serial.println();
  LoRa.printModemSettings();                                   //reads and prints the configured LoRa settings, useful check
  Serial.println();
  LoRa.printOperatingSettings();                               //reads and prints the configured operating settings, useful check
  Serial.println();
  Serial.println();

  dispscreen2();

  if (SD_Found)
  {
    printModemSettingsSD();
    logFile.println();
    printOperatingSettingsSD();
    logFile.println();
    logFile.println();
    logFile.flush();
  }

  disp.clearLine(0);
  disp.clearLine(1);
  disp.clearLine(2);
  disp.setCursor(0, 0);
  disp.print(F("Receiver ready"));

  Serial.print(F("Receiver ready - RXBUFFER_SIZE "));
  Serial.println(RXBUFFER_SIZE);
  Serial.println();

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));                 //all colours off
  pixels.show();
}
