/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/02/23

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This is a minimum setup LoRa test transmitter for the Seeeduino XIAO.

  A packet containing the ASCII text "XXX Hello World" is sent using the frequency and LoRa settings
  specified in the setupLoRa() command. XXX is a number that increases with each transmission.

  The details of the packet sent and any errors are shown on the Arduino IDE Serial Monitor, together with
  the transmit power used and the packet length. The matching receiver program, '6_LoRa_Receiver' can be used
  to check the packets are being sent correctly, the frequency and LoRa settings (in the LoRa.setupLoRa()
  commands) must be the same for the transmitter and receiver programs. Sample Serial Monitor output;

  10dBm Packet> 005 Hello World*  BytesSent,16  PacketsSent,5

    Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <SPI.h>                                //the LoRa device is SPI based so load the SPI library                                         
#include <SX127XLT.h>                           //get library here > https://github.com/StuartsProjects/SX12XX-LoRa 

SX127XLT LoRa;                                  //create a library class instance called LoRa

#define NSS A3                                  //select pin on LoRa device
#define NRESET A2                               //reset pin on LoRa device
#define DIO0 A1                                 //DIO0 pin on LoRa device, used for sensing RX and TX done 
#define LED1 13                                 //on board LED, yellow    
#define LORA_DEVICE DEVICE_SX1278               //we need to define the device we are using


//*******  Setup LoRa Parameters Here ! ***************

const uint32_t Frequency = 434000000;           //frequency of transmissions in hertz
const uint32_t Offset = 0;                      //offset frequency for calibration purposes

const uint8_t Bandwidth = LORA_BW_125;          //LoRa bandwidth
const uint8_t SpreadingFactor = LORA_SF7;       //LoRa spreading factor
const uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting, normally set to auto
const uint8_t TXpower = 2;                      //LoRa transmit power in dBm


uint8_t buff[] = "    Hello World";             //the message to send
uint8_t TXPacketL;
uint8_t counter;
uint32_t TXPacketCount;


void loop()
{
  counter++;

  buff[0] = ((counter % 1000) / 100) + '0';
  buff[1] = ((counter % 100) / 10) + '0';
  buff[2] = counter % 10 + '0' ;

  Serial.print(TXpower);                                      //print the transmit power defined
  Serial.print(F("dBm "));
  Serial.print(F("Packet> "));

  TXPacketL = sizeof(buff);                                    //set TXPacketL to length of array
  buff[TXPacketL - 1] = '*';                                   //replace null character at buffer end so its visible on receiver

  LoRa.printASCIIPacket(buff, TXPacketL);                      //print the buffer (the sent packet) as ASCII
  Serial.flush();

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
  led_Flash(10, 500);                             //10 LED flashes to indicate program start

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("5_LoRa_Transmitter"));
  Serial.println();
  SPI.begin();

  if (LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("LoRa Device found"));
    delay(1000);
  }
  else
  {
    Serial.println(F("No LoRa device responding"));
    while (1) led_Flash(10, 25);
  }

  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  pinMode(DIO0, INPUT_PULLDOWN);                     //prevent A1 floating if diodes in use as per TTN setup

  Serial.print(F("Transmitter ready"));
  Serial.println();
}
