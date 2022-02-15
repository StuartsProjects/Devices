/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 15/02/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This is a minimum setup LoRa test transmitter for the ESP32CAM. A packet containing
  the ASCII text "Hello World 1234567890" is sent using the frequency and LoRa settings specified in the
  LoRa.setupLoRa() command. The pins to access the lora device need to be defined at the top of the program
  also.

  The details of the packet sent and any errors are shown on the Arduino IDE Serial Monitor, together with
  the transmit power used and the packet length. Sample Serial Monitor output;

  10dBm Packet> Hello World 1234567890*  BytesSent,23  PacketsSent,6

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <SPI.h>                                //the lora device is SPI based so load the SPI library                                         
#include <SX127XLT.h>                           //include the appropriate library  

SX127XLT LoRa;                                  //create a library class instance called LoRa

#define NSS 12                                  //select on LoRa device
#define NRESET 15                               //reset pin on LoRa device
#define SCK 4                                   //SCK on SPI3
#define MISO 13                                 //MISO on SPI3 
#define MOSI 2                                  //MOSI on SPI3
#define REDLED 33                               //pin number for ESP32CAM on board red LED, set logic level low for on

#define LORA_DEVICE DEVICE_SX1278               //this is the device we are using

#define TXpower 2                               //LoRa transmit power in dBm

uint8_t TXPacketL;
uint32_t TXPacketCount;

uint8_t buff[] = "Hello World 1234567890";      //the message to send  


void loop()
{
  Serial.print(TXpower);                        //print the transmit power defined
  Serial.print(F("dBm "));
  Serial.print(F("Packet> "));
  Serial.flush();

  TXPacketL = sizeof(buff);                     //set TXPacketL to length of array
  buff[TXPacketL - 1] = '*';                    //replace null character at buffer end so its visible on receiver
 
  LoRa.printASCIIPacket(buff, TXPacketL);       //print the buffer (the sent packet) as ASCII

  digitalWrite(REDLED, LOW);
  
  if (LoRa.transmitIRQ(buff, TXPacketL, 10000, TXpower, WAIT_TX))   //will return packet length sent if OK, otherwise 0 if transmit error
  {
    TXPacketCount++;
    packet_is_OK();
  }
  else
  {
    packet_is_Error();                           //transmit packet returned 0, there was an error
  }

  digitalWrite(REDLED, HIGH);

  Serial.println();
  delay(1000);                                   //have a delay between packets
}


void packet_is_OK()
{
  //if here packet has been sent OK
  Serial.print(F("  BytesSent,"));
  Serial.print(TXPacketL);                       //print transmitted packet length
  Serial.print(F("  PacketsSent,"));
  Serial.print(TXPacketCount);                   //print total of packets sent OK
}


void packet_is_Error()
{
  //if here there was an error transmitting packet
  uint16_t IRQStatus;
  IRQStatus = LoRa.readIrqStatus();              //read the the interrupt register
  Serial.print(F(" SendError,"));
  Serial.print(F("Length,"));
  Serial.print(TXPacketL);                       //print transmitted packet length
  Serial.print(F(",IRQreg,"));
  Serial.print(IRQStatus, HEX);                  //print IRQ status
  LoRa.printIrqStatus();                         //prints the text of which IRQs set
}


void redFlash(uint16_t flashes, uint16_t ondelaymS, uint16_t offdelaymS)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(REDLED, LOW);
    delay(ondelaymS);
    digitalWrite(REDLED, HIGH);
    delay(offdelaymS);
  }
}


void setup()
{
  pinMode(REDLED, OUTPUT);                    //setup pin as output
  redFlash(4, 125, 125);
  
  Serial.begin(115200);
  Serial.println();
  Serial.println(F(__FILE__));

  SPI.begin(SCK, MISO, MOSI, NSS);

  if (LoRa.begin(NSS, NRESET, LORA_DEVICE))
  {
    Serial.println(F("LoRa Device found"));
    delay(1000);
  }
  else
  {
    Serial.println(F("No device responding"));
    while (1);
  }

  LoRa.setupLoRa(434000000, 0, LORA_SF7, LORA_BW_125, LORA_CR_4_5, LDRO_AUTO); //configure frequency and LoRa settings

  Serial.print(F("Test Transmitter ready"));
  Serial.println();
}
