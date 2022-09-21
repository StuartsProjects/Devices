/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 13/09/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a test program to demonstrate reception of 'Reliable' packets as used in
  the cycle alarm extender.

  A reliable packet has 4 bytes automatically appended to the end of the buffer\array that is the data
  payload. The first two bytes appended are a 16bit 'NetworkID'. This receiver needs to have the same
  NetworkID as configured for the transmitter. The receiver program uses the NetworkID to check that
  the received packet is from a known source.  The third and fourth bytes appended are a 16 bit CRC of
  the payload. The receiver will carry out its own CRC check on the received payload and can then verify
  this against the CRC appended to the packet. The receiver is thus able to check if the payload is valid.

  For a packet to be accepted by the receiver, the networkID and payload CRC appended to the packet by the
  transmitter need to match those from the receiver which gives a high level of assurance that the packet
  is valid.

  Sample serial monitor output;

  Payload received OK > Hello
  LocalNetworkID,0x3210,TransmitterNetworkID,0x3210,LocalPayloadCRC,0xB037,RXPayloadCRC,0xB037

  Serial monitor baud rate should be set at 9600.
*******************************************************************************************************/

#include <SPI.h>                                //the LoRa device is SPI based so load the SPI library
#include <SX127XLT.h>                           //include the appropriate library   

SX127XLT LoRa;                                  //create a library class instance called LoRa

#define NSS 10                                  //select pin on LoRa device
#define NRESET 9                                //reset pin on LoRa device
#define DIO0 3                                  //DIO0 pin on LoRa device, used for RX and TX done 
#define LORA_DEVICE DEVICE_SX1278               //we need to define the device we are using
#define LED1 8                                  //LED on board

#define RXtimeout 60000                         //receive timeout in mS.  

uint8_t RXPacketL;                              //stores length of packet received
uint8_t RXPayloadL;                             //stores length of payload received
uint8_t PacketOK;                               //set to > 0 if packetOK
int16_t PacketRSSI;                             //stores RSSI of received packet
uint16_t LocalPayloadCRC;                       //locally calculated CRC of payload
uint16_t RXPayloadCRC;                          //CRC of payload received in packet
uint16_t TransmitterNetworkID;                  //the NetworkID from the transmitted and received packet
uint8_t buff[255];                              //buffer for receiving packet

const uint16_t NetworkID = 0x3210;              //NetworkID identifies this connection, needs to match value in transmitter


void loop()
{
  PacketOK = LoRa.receiveReliable(buff, 251, NetworkID, RXtimeout, WAIT_RX); //wait for a packet to arrive with 60seconds (60000mS) timeout

  RXPacketL = LoRa.readRXPacketL();                       //get the received packet length, get this before reading packet
  RXPayloadL = RXPacketL - 4;
  PacketRSSI = LoRa.readPacketRSSI();                     //read the received packets RSSI value

  if (PacketOK > 0)
  {
    //if the LoRa.receiveReliable() returns a value > 0 for PacketOK then packet was received OK
    //then only action payload if destinationNode = thisNode

    Serial.print(F("Payload received OK > "));
    LoRa.printASCIIPacket(buff, RXPayloadL);              //print the packet as ASCII characters
    Serial.println();

    packet_is_OK();

  }
  else
  {
    //if the LoRa.receiveReliable() function detects an error PacketOK is 0
    packet_is_Error();
  }

  Serial.println();
}


void packet_is_OK()
{
  printPacketDetails();
  Serial.println();
}


void packet_is_Error()
{
  uint16_t IRQStatus;

  IRQStatus = LoRa.readIrqStatus();                 //read the LoRa device IRQ status register
  Serial.print(F("Error "));

  if (IRQStatus & IRQ_RX_TIMEOUT)                   //check for an RX timeout
  {
    Serial.print(F(" RXTimeout "));
  }
  else
  {
    printPacketDetails();
  }
}




void printPacketDetails()
{
  LocalPayloadCRC = LoRa.CRCCCITTSX(0, RXPayloadL - 1, 0xFFFF);  //calculate payload crc from the received payload in LoRa device buffer
  TransmitterNetworkID = LoRa.getRXNetworkID(RXPacketL);
  RXPayloadCRC = LoRa.getRXPayloadCRC(RXPacketL);                //used the saved packet value to retrieve the payload CRC

  Serial.print(F("LocalNetworkID,0x"));
  Serial.print(NetworkID, HEX);
  Serial.print(F(",TransmitterNetworkID,0x"));
  Serial.print(TransmitterNetworkID, HEX);
  Serial.print(F(",LocalPayloadCRC,0x"));
  Serial.print(LocalPayloadCRC, HEX);
  Serial.print(F(",RXPayloadCRC,0x"));
  Serial.print(RXPayloadCRC, HEX);
  LoRa.printReliableStatus();
}


void setup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("4_LoRa_Receiver Starting"));

  SPI.begin();

  if (LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("LoRa Device found"));
    delay(1000);
  }
  else
  {
    Serial.println(F("No LoRa device responding"));
    while (1);
  }

  LoRa.setupLoRa(434000000, 0, LORA_SF12, LORA_BW_062, LORA_CR_4_5, LDRO_AUTO);   //configure frequency and LoRa settings

  Serial.println(F("Receiver ready"));
  Serial.println();

}
