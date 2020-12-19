/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 19/12/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This is a minimum setup LoRa test transmitter. A packet containing the ASCII text
  "Hello World 1234567890" is sent using the frequency and LoRa settings specified in the LoRa.setupLoRa()
  command. The pins to access the LoRa device need to be defined at the top of the program also.

  The details of the packet sent and any errors are shown on the Arduino IDE Serial Monitor, together with
  the transmit power used and the packet length. The matching receiver program, 'RFM9x_LoRa_Receiver' can
  be used to check the packets are being sent correctly, the frequency and LoRa settings (in the 
  LoRa.setupLoRa() commands) must be the same for the transmitter and receiver programs. 
  Sample Serial Monitor output;

  10dBm Packet> Hello World 1234567890*  BytesSent,23  PacketsSent,6

  Serial monitor baud rate is set at 9600
*******************************************************************************************************/

#include <SPI.h>                                //the LoRa device is SPI based so load the SPI library                                         
#include <SX127XLT.h>                           //get library here > https://github.com/StuartsProjects/SX12XX-LoRa  

SX127XLT LoRa;                                  //create a library class instance called LoRa

//define the pin connections for the LoRa module, these are the connections for the Easy Pro Mini kit, link below
//https://www.tindie.com/products/20845/

#define NSS 10                                  //select pin on LoRa device
#define NRESET 9                                //reset pin on LoRa device
#define DIO0 3                                  //DIO0 pin on LoRa device, used for sensing RX and TX done 
#define LORA_DEVICE DEVICE_SX1278               //we need to define the device we are using
#define TXpower 10                              //LoRa transmit power in dBm 


//define LoRa module setting here, be sure frequency is appropriate for the particular module
#define Frequency 434000000
#define Offset 0
#define SpreadingFactor LORA_SF7
#define Bandwidth LORA_BW_125
#define CodeingRate LORA_CR_4_5


uint8_t TXPacketL;
uint32_t TXPacketCount;

uint8_t buff[] = "Hello World 1234567890";      //the message to send  


void setup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("RFM9x_LoRa_Transmitter Starting"));

  SPI.begin();

  if (LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("LoRa Device found"));
    delay(1000);
  }
  else
  {
    Serial.println(F("No device responding"));
    while (1);
  }

  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeingRate, LDRO_AUTO);   //configure frequency and LoRa settings

  Serial.print(F("Transmitter ready"));
  Serial.println();
}


void loop()
{
  Serial.print(TXpower);                                         //print the transmit power defined
  Serial.print(F("dBm "));
  Serial.print(F("Packet> "));
  Serial.flush();

  TXPacketL = sizeof(buff);                                      //set TXPacketL to length of array
  buff[TXPacketL - 1] = '*';                                     //replace null character at buffer end so its visible on receiver
 
  LoRa.printASCIIPacket(buff, TXPacketL);                        //print the buffer (the sent packet) as ASCII

  if (LoRa.transmit(buff, TXPacketL, 10000, TXpower, WAIT_TX))   //will return packet length sent if OK, otherwise 0 if transmit error
  {
    TXPacketCount++;
    packet_is_OK();
  }
  else
  {
    packet_is_Error();                                           //transmit packet returned 0, there was an error
  }

  Serial.println();
  delay(1000);                                                   //have a delay between packets
}


void packet_is_OK()
{
  //if here packet has been sent OK
  Serial.print(F("  BytesSent,"));
  Serial.print(TXPacketL);                                       //print transmitted packet length
  Serial.print(F("  PacketsSent,"));
  Serial.print(TXPacketCount);                                   //print total of packets sent OK
}


void packet_is_Error()
{
  //if here there was an error transmitting packet
  uint16_t IRQStatus;
  IRQStatus = LoRa.readIrqStatus();                               //read the the interrupt register
  Serial.print(F(" SendError,"));
  Serial.print(F("Length,"));
  Serial.print(TXPacketL);                                        //print transmitted packet length
  Serial.print(F(",IRQreg,"));
  Serial.print(IRQStatus, HEX);                                   //print IRQ status
  LoRa.printIrqStatus();                                          //prints the text of which IRQs set
}


