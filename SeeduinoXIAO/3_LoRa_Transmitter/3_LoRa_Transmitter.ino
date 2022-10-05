/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This is a minimum setup LoRa test transmitter for the Seeeduino XIAO. 
  
  A packet containing the ASCII text "Hello World 1234567890" is sent using the frequency and LoRa settings
  specified in the setupLoRa() command. The pins to access the lora device need to be defined at the
  top of the program also.

  The details of the packet sent and any errors are shown on the Arduino IDE Serial Monitor, together with
  the transmit power used and the packet length. The matching receiver program, '4_LoRa_Receiver' can be used
  to check the packets are being sent correctly, the frequency and LoRa settings (in the LT.setupLoRa()
  commands) must be the same for the transmitter and receiver programs. Sample Serial Monitor output;

  10dBm Packet> Hello World 1234567890*  BytesSent,23  PacketsSent,6

    Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <SPI.h>                                //the lora device is SPI based so load the SPI library                                         
#include <SX127XLT.h>                           //include the appropriate library  

SX127XLT LoRa;                                  //create a library class instance called LoRa

#define NSS A3                                  //select pin on LoRa device
#define NRESET A2                               //reset pin on LoRa device
#define DIO0 A1                                 //DIO0 pin on LoRa device, used for sensing RX and TX done 
#define LED1 13                                 //on board LED, yellow    
#define LORA_DEVICE DEVICE_SX1278               //we need to define the device we are using
#define TXpower 10                              //LoRa transmit power in dBm

uint8_t TXPacketL;
uint32_t TXPacketCount;

uint8_t buff[] = "Hello World 1234567890";      //the message to send  


void loop()
{
  Serial.print(TXpower);                                       //print the transmit power defined
  Serial.print(F("dBm "));
  Serial.print(F("Packet> "));
  Serial.flush();

  TXPacketL = sizeof(buff);                                    //set TXPacketL to length of array
  buff[TXPacketL - 1] = '*';                                   //replace null character at buffer end so its visible on receiver
 
  LoRa.printASCIIPacket(buff, TXPacketL);                      //print the buffer (the sent packet) as ASCII

  digitalWrite(LED1, LOW);                                     //XIAO board LED on
  if (LoRa.transmit(buff, TXPacketL, 10000, TXpower, WAIT_TX)) //will return packet length sent if OK, otherwise 0 if transmit error
  {
    TXPacketCount++;
    packet_is_OK();
  }
  else
  {
    packet_is_Error();                                         //transmit packet returned 0, there was an error
  }
  digitalWrite(LED1, HIGH);                                    //XIAO board LED off
  Serial.println();
  
  delay(1000);                                                 //have a delay between packets
}


void packet_is_OK()
{
  //if here packet has been sent OK
  Serial.print(F("  BytesSent,"));
  Serial.print(TXPacketL);                             //print transmitted packet length
  Serial.print(F("  PacketsSent,"));
  Serial.print(TXPacketCount);                         //print total of packets sent OK
}


void packet_is_Error()
{
  //if here there was an error transmitting packet
  uint16_t IRQStatus;
  IRQStatus = LoRa.readIrqStatus();                     //read the the interrupt register
  Serial.print(F(" SendError,"));
  Serial.print(F("Length,"));
  Serial.print(TXPacketL);                              //print transmitted packet length
  Serial.print(F(",IRQreg,"));
  Serial.print(IRQStatus, HEX);                         //print IRQ status
  LoRa.printIrqStatus();                                //prints the text of which IRQs set
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  //general purpose routine for flashing LED as indicator
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, LOW);                      //XIAO board LED on
    delay(delaymS);
    digitalWrite(LED1, HIGH);                     //XIAO board LED off
    delay(delaymS);
  }
}


void setup()
{
  pinMode(LED1, OUTPUT);
  led_Flash(10, 125);                             //10 quick LED flashes to indicate program start
  
  Serial.begin(115200);
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

  LoRa.setupLoRa(434000000, 0, LORA_SF7, LORA_BW_125, LORA_CR_4_5, LDRO_AUTO); //configure frequency and LoRa settings

  Serial.print(F("Transmitter ready"));
  Serial.println();
}
