/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 04/08/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This is a program that demonstrates the transmission of LoRa packets.

  A packet containing ASCII text is sent according to the frequency and LoRa settings specified in the
  'Settings.h' file. The pins to access the lora device need to be defined in the 'Settings.h' file also.

  The details of the packet sent and any errors are shown on the Arduino IDE Serial Monitor, together with
  the transmit power used, the packet length and the CRC of the packet. The matching receive program,
  '5_LoRa_Receiver' can be used to check the packets are being sent correctly, the frequency and LoRa
  settings (in Settings.h) must be the same for the transmitter and receiver programs. Sample Serial
  Monitor output;

  10dBm Packet> Hello World 1234567890*  BytesSent,23  CRC,DAAB  TransmitTime,64mS  PacketsSent,2

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <SPI.h>                                               //the lora device is SPI based so load the SPI library
#include <SX127XLT.h>                                          //include the appropriate library  
#include "Settings.h"                                          //include the setiings file, frequencies, LoRa settings etc

SX127XLT LoRa;                                                 //create a library class instance called LT

uint8_t TXPacketL;
uint32_t TXPacketCount, startmS, endmS;

uint8_t buff[] = "Hello World 1234567890";

void loop()
{
  Serial.print(TXpower);                                       //print the transmit power defined
  Serial.print(F("dBm "));
  Serial.print(F("Packet> "));
  Serial.flush();

  TXPacketL = sizeof(buff);                                    //set TXPacketL to length of array
  buff[TXPacketL - 1] = '*';                                   //replace null character at buffer end so its visible on reciver

  LoRa.printASCIIPacket(buff, TXPacketL);                      //print the buffer (the sent packet) as ASCII

  startmS =  millis();                                         //start transmit timer
  if (LoRa.transmit(buff, TXPacketL, 10000, TXpower, WAIT_TX)) //will return packet length sent if OK, otherwise 0 if transmit error
  {
    endmS = millis();                                          //packet sent, note end time
    TXPacketCount++;
    packet_is_OK();
  }
  else
  {
    packet_is_Error();                                 //transmit packet returned 0, there was an error
  }

  Serial.println();
  delay(packet_delay);                                 //have a delay between packets
}


void packet_is_OK()
{
  //if here packet has been sent OK
  uint16_t localCRC;

  Serial.print(F("  BytesSent,"));
  Serial.print(TXPacketL);                             //print transmitted packet length
  localCRC = LoRa.CRCCCITT(buff, TXPacketL, 0xFFFF);
  Serial.print(F("  CRC,"));
  Serial.print(localCRC, HEX);                         //print CRC of transmitted packet
  Serial.print(F("  TransmitTime,"));
  Serial.print(endmS - startmS);                       //print transmit time of packet
  Serial.print(F("mS"));
  Serial.print(F("  PacketsSent,"));
  Serial.print(TXPacketCount);                         //print total of packets sent OK
}


void packet_is_Error()
{
  //if here there was an error transmitting packet
  uint16_t IRQStatus;
  IRQStatus = LoRa.readIrqStatus();                    //read the the interrupt register
  Serial.print(F(" SendError,"));
  Serial.print(F("Length,"));
  Serial.print(TXPacketL);                             //print transmitted packet length
  Serial.print(F(",IRQreg,"));
  Serial.print(IRQStatus, HEX);                        //print IRQ status
  LoRa.printIrqStatus();                               //prints the text of which IRQs set
}


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.println();
  Serial.println(__TIME__);
  Serial.println();
  Serial.println(__DATE__);
  Serial.println();

  SPI.begin(SCK, MISO, MOSI, NSS);

  //SPI beginTranscation is normally part of library routines, but if it is disabled in library
  //a single instance is needed here, so uncomment the program line below
  //SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  //setup hardware pins used by device, then check if device is found
  if (LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("LoRa Device found"));
  }
  else
  {
    Serial.println(F("No LoRa device responding - program halted"));
    while (1);
  }

  //The function call list below shows the complete setup for the LoRa device using the information defined in the
  //Settings.h file.
  //The 'Setup LoRa device' list below can be replaced with a single function call;
  //LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  //***************************************************************************************************
  //Setup LoRa device
  //***************************************************************************************************
  LoRa.setMode(MODE_STDBY_RC);                              //got to standby mode to configure device
  LoRa.setPacketType(PACKET_TYPE_LORA);                     //set for LoRa transmissions
  LoRa.setRfFrequency(Frequency, Offset);                   //set the operating frequency
  LoRa.calibrateImage(0);                                   //run calibration after setting frequency
  LoRa.setModulationParams(SpreadingFactor, Bandwidth, CodeRate, LDRO_AUTO);  //set LoRa modem parameters
  LoRa.setBufferBaseAddress(0x00, 0x00);                    //where in the SX buffer packets start, TX and RX
  LoRa.setPacketParams(8, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL);  //set packet parameters
  LoRa.setSyncWord(LORA_MAC_PRIVATE_SYNCWORD);              //syncword, LORA_MAC_PRIVATE_SYNCWORD = 0x12, or LORA_MAC_PUBLIC_SYNCWORD = 0x34
  LoRa.setHighSensitivity();                                //set for highest sensitivity at expense of slightly higher LNA current
  LoRa.setDioIrqParams(IRQ_RADIO_ALL, IRQ_TX_DONE, 0, 0);   //set for IRQ on RX done
  //***************************************************************************************************

  Serial.println();
  LoRa.printModemSettings();                               //reads and prints the configured LoRa settings, useful check
  Serial.println();
  LoRa.printOperatingSettings();                           //reads and prints the configured operating settings, useful check
  Serial.println();
  Serial.println();
  LoRa.printRegisters(0x00, 0x4F);                         //print contents of device registers, normally 0x00 to 0x4F
  Serial.println();
  Serial.println();

  Serial.print(F("Transmitter ready"));
  Serial.println();
}
