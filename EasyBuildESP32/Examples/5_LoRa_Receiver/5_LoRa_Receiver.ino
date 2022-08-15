/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 04/08/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a program that demonstrates the receipt of LoRa packets.
  
  The program listens for incoming packets using the LoRa settings in the 'Settings.h' file. The pins
  to access the lora device need to be defined in the 'Settings.h' file also.

  There is a printout on the Arduino IDE Serial Monitor of the valid packets received, the packet is
  assumed to be in ASCII printable text, if it's not ASCII text characters from 0x20 to 0x7F, expect
  weird things to happen on the Serial Monitor. The LED will flash for each packet received.
  
  Sample serial monitor output;

  7s  Hello World 1234567890*,CRC,DAAB,RSSI,-52dBm,SNR,9dB,Length,23,Packets,5,Errors,0,IRQreg,50

  If there is a packet error it might look like this, which is showing a CRC error,

  968s PacketError,RSSI,-87dBm,SNR,-11dB,Length,23,Packets,613,Errors,2,IRQreg,70,IRQ_HEADER_VALID,IRQ_CRC_ERROR,IRQ_RX_DONE

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <SPI.h>                                 //the lora device is SPI based so load the SPI library
#include <SX127XLT.h>                            //include the appropriate library   
#include "Settings.h"                            //include the setiings file, frequencies, LoRa settings etc   

SX127XLT LoRa;                                     //create a library class instance called LoRa

uint32_t RXpacketCount;
uint32_t errors;

uint8_t RXBUFFER[RXBUFFER_SIZE];                 //create the buffer that received packets are copied into

uint8_t RXPacketL;                               //stores length of packet received
int16_t PacketRSSI;                              //stores RSSI of received packet
int8_t PacketSNR;                                //stores signal to noise ratio (SNR) of received packet


void loop()
{
  RXPacketL = LoRa.receive(RXBUFFER, RXBUFFER_SIZE, 60000, WAIT_RX); //wait for a packet to arrive with 60seconds (60000mS) timeout

  digitalWrite(LED1, HIGH);                      //something has happened

  PacketRSSI = LoRa.readPacketRSSI();            //read the recived RSSI value
  PacketSNR = LoRa.readPacketSNR();              //read the received SNR value

  if (RXPacketL == 0)                            //if the LoRa.receive() function detects an error, RXpacketL is 0
  {
    packet_is_Error();
  }
  else
  {
    packet_is_OK();
  }

  delay(10);                                     //so we can see LED flash more easily on fast processors
  digitalWrite(LED1, LOW);                       //LED off

  Serial.println();
}


void packet_is_OK()
{
  uint16_t IRQStatus, localCRC;

  IRQStatus = LoRa.readIrqStatus();                 //read the LoRa device IRQ status register

  RXpacketCount++;

  printElapsedTime();                               //print elapsed time to Serial Monitor
  Serial.print(F("  "));
  LoRa.printASCIIPacket(RXBUFFER, RXPacketL);       //print the packet as ASCII characters

  localCRC = LoRa.CRCCCITT(RXBUFFER, RXPacketL, 0xFFFF);  //calculate the CRC, this is the external CRC calculation of the RXBUFFER
  Serial.print(F(",CRC,"));                         //contents, not the LoRa device internal CRC
  Serial.print(localCRC, HEX);
  Serial.print(F(",RSSI,"));
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
  Serial.print(IRQStatus, HEX);
}


void packet_is_Error()
{
  uint16_t IRQStatus;
  IRQStatus = LoRa.readIrqStatus();                   //read the LoRa device IRQ status register

  printElapsedTime();                                 //print elapsed time to Serial Monitor

  if (IRQStatus & IRQ_RX_TIMEOUT)                     //check for an RX timeout
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
    Serial.print(LoRa.readRXPacketL());               //get the device packet length
    Serial.print(F(",Packets,"));
    Serial.print(RXpacketCount);
    Serial.print(F(",Errors,"));
    Serial.print(errors);
    Serial.print(F(",IRQreg,"));
    Serial.print(IRQStatus, HEX);
    LoRa.printIrqStatus();                            //print the names of the IRQ registers set
  }

  delay(250);                                         //gives a longer buzzer and LED flash for error

}


void printElapsedTime()
{
  float seconds;
  seconds = millis() / 1000;
  Serial.print(seconds, 0);
  Serial.print(F("s"));
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

  //SPI beginTranscation is normally part of library routines, but if it is disabled in the library
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
  //The 'Setup Lora device' list below can be replaced with a single function call;
  //LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  //***************************************************************************************************
  //Setup Lora device
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
  LoRa.setDioIrqParams(IRQ_RADIO_ALL, IRQ_RX_DONE, 0, 0);   //set for IRQ on RX done
  //***************************************************************************************************


  Serial.println();
  LoRa.printModemSettings();                                     //reads and prints the configured LoRa settings, useful check
  Serial.println();
  LoRa.printOperatingSettings();                                 //reads and prints the configured operting settings, useful check
  Serial.println();
  Serial.println();
  LoRa.printRegisters(0x00, 0x4F);                               //print contents of device registers, normally 0x00 to 0x4F
  Serial.println();
  Serial.println();

  Serial.print(F("Receiver ready - RXBUFFER_SIZE "));
  Serial.println(RXBUFFER_SIZE);
  Serial.println();
}
