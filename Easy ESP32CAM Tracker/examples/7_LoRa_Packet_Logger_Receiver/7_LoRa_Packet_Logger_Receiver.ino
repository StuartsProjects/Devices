/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 13/12/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation -  This is a LoRa receiver intended to be used with the matching high altitude
  balloon (HAB) tracker programs for ESP32CAM. This program is not intended for use on a ESP32CAM an 
  alternative Arduino, such as a DUE, should be used. The program listens for incoming packets using
  the LoRa settings in the 'Settings.h' file. The pins to access the lora device need to be defined
  in the 'Settings.h' file also.

  There is a printout of the valid packets received in HEX and ASCII. The LED will flash for each packet
  received. The measured frequency difference between the frequency used by the transmitter and the
  frequency used by the receiver is shown. If this frequency difference gets to more than 25% of the
  set LoRa bandwidth, packet reception will fail. The displayed error can be reduced by using the
  'offset' setting in the 'Settings.h' file.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <SPI.h>
#include <SX127XLT.h>                            //get the library here; https://github.com/StuartsProjects/SX12XX-LoRa                         
#include "Settings.h"
#include <TimeLib.h>                             //get the library here; https://github.com/PaulStoffregen/Time
time_t recordtime;                               //used to record the current time, preventing displayed rollover on printing

SX127XLT LoRa;

uint32_t RXpacketCount;
uint32_t errors;

uint8_t RXPacketL;                                //stores length of packet received
int8_t  PacketRSSI;                               //stores RSSI of received packet
int8_t  PacketSNR;                                //stores signal to noise ratio of received packet


void loop()
{
  RXPacketL = LoRa.receiveSXBuffer(0, 60000, WAIT_RX); //returns 0 if packet error of some sort, timeout set at 60secs\60000mS

  digitalWrite(LED1, HIGH);                       //something has happened
  recordtime = now();                             //stop the time to be displayed rolling over
  printtime();

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

  digitalWrite(LED1, LOW);

  Serial.println();
}


void packet_is_OK()
{
  uint16_t IRQStatus;
  IRQStatus = LoRa.readIrqStatus();

  RXpacketCount++;

  Serial.print(F(" FreqErrror,"));
  Serial.print(LoRa.getFrequencyErrorHz());
  Serial.print(F("hz "));

  Serial.print(F(" RSSI,"));
  Serial.print(PacketRSSI);
  Serial.print(F("dBm,SNR,"));
  Serial.print(PacketSNR);
  Serial.print(F("dB,Length,"));
  Serial.print(RXPacketL);
  Serial.print(F(",Packets,"));
  Serial.print(RXpacketCount);
  Serial.print(F(",Errors,"));
  Serial.print(errors);
  Serial.print(F(",IRQreg,"));
  Serial.println(IRQStatus, HEX);

  LoRa.printSXBufferASCII(0, (RXPacketL - 1));
  Serial.println();
  LoRa.printSXBufferHEX(0, (RXPacketL - 1));
  Serial.println();

}


void packet_is_Error()
{
  uint16_t IRQStatus;
  IRQStatus = LoRa.readIrqStatus();                    //get the IRQ status

  if (IRQStatus & IRQ_RX_TIMEOUT)
  {
    Serial.print(F(" RXTimeout"));
  }
  else
  {
    errors++;
    Serial.print(F(" PacketError"));
    Serial.print(F(",RSSI,"));
    Serial.print(PacketRSSI);
    Serial.print(F("dBm,SNR,"));
    Serial.print(PacketSNR);
    Serial.print(F("dB,Length,"));
    Serial.print(LoRa.readRXPacketL());                  //get the real packet length
    Serial.print(F(",Packets,"));
    Serial.print(RXpacketCount);
    Serial.print(F(",Errors,"));
    Serial.print(errors);
    Serial.print(F(",IRQreg,"));
    Serial.print(IRQStatus, HEX);
  }
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, HIGH);
    delay(delaymS);
    digitalWrite(LED1, LOW);
    delay(delaymS);
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


void setup()
{
  pinMode(LED1, OUTPUT);
  led_Flash(2, 125);

  Serial.begin(115200);

  Serial.println(F("7_LoRa_Packet_Logger_Receiver Starting"));
  Serial.println();

  SPI.begin();

  //SPI beginTranscation is normally part of library routines, but if it is disabled in library
  //a single instance is needed here, so uncomment the program line below
  //SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  if (LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("Radio Device found"));
    led_Flash(2, 125);
    delay(1000);
  }
  else
  {
    Serial.println(F("No device responding"));
    while (1)
    {
      led_Flash(50, 50);
    }
  }

  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  Serial.println();
  LoRa.printModemSettings();
  Serial.println();
  LoRa.printOperatingSettings();
  Serial.println();
  Serial.println();
  printtime();
  Serial.print(F(" Receiver ready"));
  Serial.println();
  Serial.println();
}

