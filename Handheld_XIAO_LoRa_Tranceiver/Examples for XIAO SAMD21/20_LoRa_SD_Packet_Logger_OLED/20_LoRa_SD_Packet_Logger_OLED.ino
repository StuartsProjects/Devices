/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 18/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO SAMD21.
  
  Program Operation - The program for the Seeeduino XIAO listens for incoming packets using the LoRa
  settings in the 'Settings.h' file. The pins to access the lora device need to be defined in the
  'Settings.h' file also.

  There is a printout and save to SD card of the valid packets received in HEX format. Thus the program
  can be used to receive and record non-ASCII packets. The LED will flash for each packet received.
  The measured frequency difference between the frequency used by the transmitter and the frequency used
  by the receiver is shown. If this frequency difference gets to 25% of the set LoRa bandwidth, packet
  reception will fail. The displayed error can be reduced by using the 'offset' setting in the
  'Settings.h' file.

  There will be a limit to how fast the logger can receive packets, mainly caused by the delay in writing
  to SD card, so at high packet rates, packets will be lost.

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <SPI.h>
#include <SD.h>


File logFile;
char filename[] = "Log0000.txt";
bool SD_Found = false;                           //set if SD card found at program startup
uint8_t lognumber;


#include <SX127XLT.h>                            //get library here > https://github.com/StuartsProjects/SX12XX-LoRa 
SX127XLT LoRa;


#include <U8x8lib.h>                                      //get library here >  https://github.com/olikraus/u8g2 
U8X8_SSD1306_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);    //use this line for standard 0.96" SSD1306
//U8X8_SH1106_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);   //use this line for 1.3" OLED often sold as 1.3" SSD1306
#define DEFAULTFONT u8x8_font_chroma48medium8_r           //font used by U8X8 Library

#include "Settings.h"
#include <TimeLib.h>                             //get the library here > https://github.com/PaulStoffregen/Time

uint32_t RXpacketCount;                          //count of good packets
uint32_t RXpacketErrors;                         //count of packet errors
uint8_t RXPacketL;                               //stores length of packet received
int16_t PacketRSSI;                              //stores RSSI of received packet
int8_t  PacketSNR;                               //stores signal to noise ratio of received packet
uint16_t IRQStatus;                              //used to read the IRQ status
int32_t FreqErrror;                              //frequency error of received packet, in hz

time_t recordtime;                               //used to record the current time, preventing displayed rollover on printing

uint8_t RXBUFFER[RXBUFFER_SIZE];                 //create the buffer that received packets are copied into

#include "SD_Logger_Library.h"


void loop()
{
  RXPacketL = LoRa.receive(RXBUFFER, RXBUFFER_SIZE, 60000, WAIT_RX); //wait for a packet to arrive with 60seconds (60000mS) timeout

  digitalWrite(LED1, LOW);                       //something has happened
  recordtime = now();                             //stop the time to be displayed rolling over
  printtime();
  printtimeSD();

  PacketRSSI = LoRa.readPacketRSSI();
  PacketSNR = LoRa.readPacketSNR();
  FreqErrror = LoRa.getFrequencyErrorHz();
  IRQStatus = LoRa.readIrqStatus();

  if (RXPacketL == 0)
  {
    packet_is_Error();
    if(SD_Found)
    {
      packet_is_ErrorSD();
    }
  }
  else
  {
    packet_is_OK();
    dispscreen1();
    if(SD_Found)
    {
    packet_is_OKSD();
    }
  }

  digitalWrite(LED1, HIGH);

  Serial.println();
}


void packet_is_OK()
{

  RXpacketCount++;

  Serial.print(F(" FreqErrror,"));
  Serial.print(FreqErrror);
  Serial.print(F("hz  "));

  LoRa.printHEXPacket(RXBUFFER, RXPacketL);

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
}


void packet_is_Error()
{
  RXPacketL = LoRa.readRXPacketL();                    //get the real packet length

  if (IRQStatus & IRQ_RX_TIMEOUT)
  {
    Serial.print(F(" RXTimeout"));
  }
  else
  {
    RXpacketErrors++;
    Serial.print(F(" PacketError"));
    Serial.print(F(",RSSI,"));
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
  }
}


void printDigits(int8_t digits)
{
  //utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(F(":"));
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}


void printtime()
{
  Serial.print(hour(recordtime));
  printDigits(minute(recordtime));
  printDigits(second(recordtime));
}


void dispscreen1()
{
  disp.clearLine(0);
  disp.setCursor(0, 0);
  disp.print(F("OK "));
  disp.print(RXpacketCount);
  disp.clearLine(1);
  disp.setCursor(0, 1);
  disp.print(F("Errors "));
  disp.print(RXpacketErrors);
  disp.clearLine(2);
  disp.setCursor(0, 2);
  disp.print(F("Length  "));
  disp.print(RXPacketL);
  disp.clearLine(3);
  disp.setCursor(0, 3);
  disp.print(F("RSSI "));
  disp.print(PacketRSSI);
  disp.print(F("dBm "));
  disp.clearLine(4);
  disp.setCursor(0, 4);
  disp.print(F("SNR "));

  if (PacketSNR > 0)
  {
    disp.print(F("+"));
  }

  disp.print(PacketSNR);
  disp.print(F("dB"));

  disp.clearLine(5);
  disp.setCursor(0, 5);
  disp.print(F("FErr "));
  disp.print(FreqErrror);
  disp.print(F("hZ"));
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
    disp.print(F("Log "));
    disp.print(filename);
  }
  else
  {
    disp.print(F("No SD card"));
  }
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
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
  pinMode(LED1, OUTPUT);
  led_Flash(2, 125);

  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.println();

  Serial.println(F("20_LoRa_SD_Packet_Logger Starting"));
  Serial.println();

  SPI.begin();

  while (!LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("ERROR LoRa device not found"));
    while (1) led_Flash(10, 25);
  }

  Serial.println(F("LoRa device found"));
  led_Flash(2, 125);
  
  lognumber = setup_SDLOG() ;                   //setup SD card

  if (!lognumber)
  {
    Serial.println(F("ERROR SD card fail"));
    led_Flash(10, 25);
    SD_Found = false;
  }
  else
  {
    Serial.print(F("Lognumber "));
    Serial.println(lognumber);
    SD_Found = true;
  }

  disp.begin();
  disp.setFont(DEFAULTFONT);
  disp.clear();
  disp.setCursor(0, 0);
  dispscreen2();

  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);
  Serial.println();
  LoRa.printModemSettings();
  Serial.println();

  LoRa.printOperatingSettings();
  Serial.println();
  printtime();

  if (SD_Found)
  {
    printOperatingSettingsSD();
    logFile.println();
    printModemSettingsSD();
    logFile.println();
  }

  Serial.print(F("Receiver ready"));
  Serial.println();
}
