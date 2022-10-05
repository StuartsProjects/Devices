/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a minimum setup LoRa test receiver for the Seeeduino XIAO. The program listens
  for incoming packets using the frequency and LoRa settings in the LoRa.setupLoRa() command. The pins to
  access the lora device need to be defined at the top of the program also.

  There is a printout on the Arduino IDE serial monitor of the valid packets received, the packet is assumed
  to be in ASCII printable text, if it's not ASCII text characters from 0x20 to 0x7F, expect weird things to
  happen on the Serial Monitor. Sample serial monitor output;

  8s  Hello World 1234567890*,RSSI,-44dBm,SNR,9dB,Length,23,Packets,7,Errors,0,IRQreg,50

  If there is a packet error it might look like this, which is showing a CRC error;

  137s PacketError,RSSI,-89dBm,SNR,-8dB,Length,23,Packets,37,Errors,2,IRQreg,70,IRQ_HEADER_VALID,IRQ_CRC_ERROR,IRQ_RX_DONE

  If there are no packets received in a 10 second period then you should see a message like this;

  112s RXTimeout

  For an example of a more detailed configuration for a receiver, see program 104_LoRa_Receiver.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <SPI.h>                                //the lora device is SPI based so load the SPI library
#include <SX127XLT.h>                           //include the appropriate library   

SX127XLT LoRa;                                  //create a library class instance called LoRa

#define NSS A3                                  //select pin on LoRa device
#define NRESET A2                               //reset pin on LoRa device
#define DIO0 A1                                 //DIO0 pin on LoRa device, used for sensing RX and TX done 
#define LED1 13                                 //on board LED, yellow    
#define TXpower 10                              //LoRa transmit power in dBm
#define LORA_DEVICE DEVICE_SX1278               //we need to define the device we are using

#define RXBUFFER_SIZE 255                       //RX buffer size

uint32_t RXpacketCount;
uint32_t errors;

uint8_t RXBUFFER[RXBUFFER_SIZE];                //create the buffer that received packets are copied into

uint8_t RXPacketL;                              //stores length of packet received
int16_t PacketRSSI;                             //stores RSSI of received packet
int8_t  PacketSNR;                              //stores signal to noise ratio (SNR) of received packet


void loop()
{
  RXPacketL = LoRa.receive(RXBUFFER, RXBUFFER_SIZE, 60000, WAIT_RX); //wait for a packet to arrive with 60seconds (60000mS) timeout

  PacketRSSI = LoRa.readPacketRSSI();            //read the received packets RSSI value
  PacketSNR = LoRa.readPacketSNR();              //read the received packets SNR value

  digitalWrite(LED1, LOW);                       //XIAO board LED on
  if (RXPacketL == 0)                            //if the LoRa.receive() function detects an error RXpacketL is 0
  {
    packet_is_Error();
  }
  else
  {
    packet_is_OK();
  }
  delay(100);                                     //so we can see on board LED, yellow
  digitalWrite(LED1, HIGH);                       //XIAO board LED off
  Serial.println();
}


void packet_is_OK()
{
  uint16_t IRQStatus;

  RXpacketCount++;
  IRQStatus = LoRa.readIrqStatus();                  //read the LoRa device IRQ status register
  printElapsedTime();                              //print elapsed time to Serial Monitor

  Serial.print(F("  "));
  LoRa.printASCIIPacket(RXBUFFER, RXPacketL);        //print the packet as ASCII characters

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

  printElapsedTime();                               //print elapsed time to Serial Monitor

  if (IRQStatus & IRQ_RX_TIMEOUT)                   //check for an RX timeout
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
    Serial.print(LoRa.readRXPacketL());               //get the real packet length
    Serial.print(F(",Packets,"));
    Serial.print(RXpacketCount);
    Serial.print(F(",Errors,"));
    Serial.print(errors);
    Serial.print(F(",IRQreg,"));
    Serial.print(IRQStatus, HEX);
    LoRa.printIrqStatus();                            //print the names of the IRQ registers set
  }
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
  led_Flash(10, 125);                            //10 quick LED flashes to indicate program start

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("4_LoRa_Receiver Starting"));
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
    while (1);
  }

  LoRa.setupLoRa(434000000, 0, LORA_SF7, LORA_BW_125, LORA_CR_4_5, LDRO_AUTO);   //configure frequency and LoRa settings

  Serial.print(F("Receiver ready - RXBUFFER_SIZE "));
  Serial.println(RXBUFFER_SIZE);
  Serial.println();
}
