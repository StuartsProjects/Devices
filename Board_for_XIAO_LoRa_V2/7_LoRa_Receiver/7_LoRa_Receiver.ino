/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/02/23

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO SAMD21.

  Program Operation - This is a minimum setup LoRa test receiver for the Seeeduino XIAO SAMD21. The program
  listens for incoming packets using the frequency and LoRa settings in the LoRa.setupLoRa() command. The
  pins to access the LoRa device need to be defined at the top of the program also.

  There is a printout on the Arduino IDE serial monitor of the valid packets received, the packet is assumed
  to be in ASCII printable text, if it's not ASCII text characters from 0x20 to 0x7F, expect weird things to
  happen on the Serial Monitor. Sample serial monitor output;

  8s  007 Hello World*,RSSI,-44dBm,SNR,9dB,Length,16,Packets,7,Errors,0,IRQreg,50

  If there is a packet error it might look like this, which is showing a CRC error;

  137s PacketError,RSSI,-89dBm,SNR,-8dB,Length,16,Packets,8,Errors,1,IRQreg,70,IRQ_HEADER_VALID,IRQ_CRC_ERROR,IRQ_RX_DONE

  If there are no packets received in a 10 second period then you should see a message like this;

  112s RXTimeout

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <SPI.h>                                //the LoRa device is SPI based so load the SPI library
#include <SX127XLT.h>                           //get library here > https://github.com/StuartsProjects/SX12XX-LoRa 

SX127XLT LoRa;                                  //create a library class instance called LoRa

#define NSS A3                                  //select pin on LoRa device
#define NRESET A2                               //reset pin on LoRa device
#define DIO0 A1                                 //DIO0 pin on LoRa device, used for sensing RX and TX done 
#define LED1 13                                 //on board LED, yellow    
#define LORA_DEVICE DEVICE_SX1278               //we need to define the device we are using
#define RXBUFFER_SIZE 255                       //RX buffer size

//*******  Setup LoRa Parameters Here ! ***************

const uint32_t Frequency = 434000000;           //frequency of transmissions in hertz
const uint32_t Offset = 0;                      //offset frequency for calibration purposes

const uint8_t Bandwidth = LORA_BW_125;          //LoRa bandwidth
const uint8_t SpreadingFactor = LORA_SF7;       //LoRa spreading factor
const uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting, normally set to auto
const uint8_t TXpower = 10;                     //LoRa transmit power in dBm

uint32_t RXpacketCount;                         //count of packets received
uint32_t errors;                                //count of packet errors
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
  pinMode(LED1, OUTPUT);                          //setup board LED pin as output
  led_Flash(10, 500);                              //2 LED flashes to indicate program start

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("7_LoRa_Receiver"));
  Serial.println();

  SPI.begin();

  while (!LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("ERROR - No LoRa device is responding"));
    led_Flash(10, 25);                            //10 fast LED flashes to indicate LoRa device not responding
  }

  Serial.println(F("LoRa Device responding"));
  led_Flash(2, 125);                              //2 LED flashes to indicate LoRa device is responding

  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);
  pinMode(DIO0, INPUT_PULLDOWN);                  //prevent A1 floating if diodes in use as per TTN setup

  Serial.print(F("Receiver ready - RXBUFFER_SIZE "));
  Serial.println(RXBUFFER_SIZE);
  Serial.println();
}
