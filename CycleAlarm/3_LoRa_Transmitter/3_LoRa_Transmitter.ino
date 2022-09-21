/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 13/09/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This is a test program to demonstrate transmission of a 'Reliable' packets as used
  in the cycle alarm extender.

  A reliable packet has 4 bytes automatically appended to the end of the buffer\array that is the data
  payload. The first two bytes appended are a 16bit 'NetworkID'. This receiver needs to have the same
  NetworkID as configured for the transmitter. The receiver program uses the NetworkID to check that
  the received packet is from a known source.  The third and fourth bytes appended are a 16 bit CRC of
  the payload. The receiver will carry out its own CRC check on the received payload and can then verify
  this against the CRC appended to the packet. The receiver is thus able to check if the payload is valid.

  For a packet to be accepted by the receiver, the networkID and payload CRC appended to the packet by the
  transmitter need to match those from the receiver which gives a high level of assurance that the packet
  is valid.

  Sample Serial Monitor output;

  10dBm Packet> Hello BytesSent,6  PacketsSent,6

  Serial monitor baud rate is set at 9600
*******************************************************************************************************/

#include <SPI.h>                                //the LoRa device is SPI based so load the SPI library                                         
#include <SX127XLT.h>                           //include the appropriate library  

SX127XLT LoRa;                                  //create a library class instance called LT

#define NSS 10                                  //select pin on LoRa device
#define NRESET 9                                //reset pin on LoRa device
#define DIO0 3                                  //DIO0 pin on LoRa device, used for sensing RX and TX done
#define LED1 8                                  //pin number for PCB LED, set logic level high for on

#define LORA_DEVICE DEVICE_SX1278               //we need to define the device we are using
#define TXpower 10                              //LoRa transmit power in dBm
#define TXtimeout 2500                          //transmit timeout in mS. If 0 return from transmit function after send.  

uint8_t TXPacketL;
uint32_t TXPacketCount;
uint8_t buff[] = "Hello";                       //the message to send

const uint16_t NetworkID = 0x3210;              //NetworkID identifies this connection, needs to match value in receiver


void loop()
{
  Serial.print(TXpower);                        //print the transmit power defined
  Serial.print(F("dBm "));
  Serial.print(F("Packet> "));
  Serial.flush();

  TXPacketL = sizeof(buff);                     //set TXPacketL to length of array

  LoRa.printASCIIPacket(buff, TXPacketL);       //print the buffer (the sent packet) as ASCII

  digitalWrite(LED1, HIGH);                     //board LED on

  if (LoRa.transmitReliable(buff, TXPacketL, NetworkID, TXtimeout, TXpower, WAIT_TX))   //will return packet length sent if OK, otherwise 0 if transmit error
  {
    TXPacketCount++;
    packet_is_OK();
  }
  else
  {
    packet_is_Error();                           //transmit packet returned 0, there was an error
  }

  digitalWrite(LED1, LOW);                       //board LED off

  Serial.println();
  delay(1000);                                   //have a delay between packets
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  //general purpose routine for flashing LED as indicator
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, HIGH);                     //board LED on
    delay(delaymS);
    digitalWrite(LED1, LOW);                      //board LED off
    delay(delaymS);
  }
}


void packet_is_OK()
{
  //if here packet has been sent OK
  Serial.print(F("  BytesSent,"));
  Serial.print(TXPacketL);                        //print transmitted packet length
  Serial.print(F("  PacketsSent,"));
  Serial.print(TXPacketCount);                    //print total of packets sent OK
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
  pinMode(LED1, OUTPUT);                               //setup pin as output for indicator LED
  digitalWrite(LED1, LOW);

  led_Flash(2, 125);                                   //two quick LED flashes to indicate program start

  Serial.begin(9600);
  Serial.println();
  Serial.println(F("3_LoRa_Transmitter Starting"));

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

  LoRa.setupLoRa(434000000, 0, LORA_SF12, LORA_BW_062, LORA_CR_4_5, LDRO_AUTO); //configure frequency and LoRa settings

  Serial.print(F("Transmitter ready"));
  Serial.println();
}
