/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 16/10/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  This is a program that uses the LR2021 to receive packets. The packet is in ASCII and the count of packets
  sent is in the packet. 

  The program was tested on version 7.8.1 of the RadioLib library for the LR2021. The program will not work
  on the recently released version 8 of the RadioLib library.
  
  The file defining the pins for the used microcontroller board such as SEEED_XIAO_Pins.h contains the pin 
  definitions used for the LoRa module. You need to connect the LR2021 up to the XIAO SPI pins also. 

  Sample serial port output;

  19s,LoRa00011,RSSI,-36dBm,SNR,15dB,Length,10,Packets,11,Errors,0
  20s,LoRa00012,RSSI,-36dBm,SNR,15dB,Length,10,Packets,12,Errors,0
  22s,LoRa00013,RSSI,-36dBm,SNR,15dB,Length,10,Packets,13,Errors,0

*******************************************************************************************************/

/*
  For default module settings, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration#lr2021---lora-modem

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// include the LR2021 library
#include <RadioLib.h>
#include "Settings.h"
#include "SEEED_XIAO_Board.h"  //include if processor board is XIAO SAMD
//#include "Arduino_DUE_Board.h"  //include if processor board is Arduino DUE

LR2021 radio = new Module(LORA_NSS, LORA_DIO, LORA_RESET, LORA_BUSY);

uint8_t RXbuffer[10];    //buffer to receive packet, transmitted packet is 10 char array
uint8_t RXPacketL;       //stores length of packet received
int8_t PacketRSSI;       //stores RSSI of received packet
int8_t PacketSNR;        //stores signal to noise ratio (SNR) of received packet
uint32_t RXpacketCount;  //keep count of packets received
uint32_t RXerrors;       //keep count of any packets with errors
int16_t state;           //reported state of radio. functions


void loop() {

  state = radio.receive(RXbuffer, sizeof(RXbuffer), 60000);  //put receive timeout at 60 seconds

  PacketRSSI = radio.getRSSI();         //read the received packets RSSI value
  PacketSNR = radio.getSNR();           //read the received packets SNR value
  RXPacketL = radio.getPacketLength();  //read the length of packet received

  digitalWrite(LED1, LEDON);

  if (state == RADIOLIB_ERR_NONE) {
    packet_is_OK();
  } else {
    packet_is_Error();
  }

  digitalWrite(LED1, LEDOFF);
}


void packet_is_OK() {
  RXpacketCount++;
  printElapsedTime();
  mySerial.print(F(","));
  mySerial.print((char*)RXbuffer);

  mySerial.print(F(",RSSI,"));
  mySerial.print(PacketRSSI);
  mySerial.print(F("dBm,SNR,"));
  mySerial.print(PacketSNR);
  mySerial.print(F("dB,Length,"));
  mySerial.print(RXPacketL);
  mySerial.print(F(",Packets,"));
  mySerial.print(RXpacketCount);
  mySerial.print(F(",Errors,"));
  mySerial.print(RXerrors);
  mySerial.println();
}


void packet_is_Error() {

  printElapsedTime();

  if (state == RADIOLIB_ERR_RX_TIMEOUT) {
    mySerial.print(F("RX Timeout"));
  } else {
    RXerrors++;
    mySerial.print(F(" PacketError"));
    mySerial.print(F(",RSSI,"));
    mySerial.print(PacketRSSI);
    mySerial.print(F("dBm,SNR,"));
    mySerial.print(PacketSNR);
    mySerial.print(F("dB,Length,"));
    mySerial.print(RXPacketL);
    mySerial.print(F(",Packets,"));
    mySerial.print(RXpacketCount);
    mySerial.print(F(",Errors,"));
    mySerial.print(RXerrors);
    mySerial.print(F(",State,"));
    mySerial.print(state);
  }
  mySerial.println();
}


void printElapsedTime() {
  float seconds;
  seconds = millis() / 1000;
  mySerial.print(seconds, 0);
  mySerial.print(F("s"));
}


void led_Flash(uint16_t flashes, uint16_t delaymS) {
  uint16_t index;

  for (index = 1; index <= flashes; index++) {
    digitalWrite(LED1, LEDON);
    delay(delaymS);
    digitalWrite(LED1, LEDOFF);
    delay(delaymS);
  }
}


void print_Device_Settings() {
  mySerial.println(F("LoRa Device Settings"));

  mySerial.print(F("Frequency "));
  mySerial.print(Frequency, 3);
  mySerial.println(F("Khz"));

  mySerial.print(F("Bandwith "));
  mySerial.print(Bandwidth, 3);
  mySerial.println(F("Khz"));

  mySerial.print(F("SpreadingFactor "));
  mySerial.println(SpreadingFactor);

  mySerial.print(F("CodeRate 4:"));
  mySerial.println(CodeRate);

  mySerial.print(F("LoRaSyncWord 0x"));
  mySerial.println(LoRaSyncWord, HEX);

  mySerial.print(F("PreambleLength "));
  mySerial.println(PreambleLength);

  mySerial.print(F("TXPower "));
  mySerial.print(TXPower);
  mySerial.println(F("dBm"));

  mySerial.print(F("TCXOVoltage "));
  mySerial.print(TCXOVoltage, 1);
  mySerial.println(F("V"));

  mySerial.print(F("LORA_RESET pin "));
  mySerial.println(LORA_RESET);

  mySerial.print(F("LORA_DIO pin "));
  mySerial.println(LORA_DIO);

  mySerial.print(F("LORA_BUSY pin "));
  mySerial.println(LORA_BUSY);

  mySerial.print(F("LORA_NSS pin "));
  mySerial.println(LORA_NSS);

  mySerial.print(F("IRQDIONUM "));
  mySerial.println(IRQDIONUM);
}


void setup() {
  pinMode(LED1, OUTPUT);     //initialize digital pin LED1 as an output.
  digitalWrite(LED1, HIGH);  //LED on

  mySerial.begin(115200);

  if (!Serial) {};

  led_Flash(10, 250);

  mySerial.println();
  mySerial.println(__FILE__);

  // LR2021 allows to use any of the DIO pins as the interrupt
  // as an example, we set DIO11 to be the IRQ
  // this has to be done prior to calling begin()!
  radio.irqDioNum = IRQDIONUM;

  // also before calling begin(), correct crystal has to be selected
  // some LR2021 have a TCXO which needs 1.6V reference
  // set to 0 if your radio has an XTAL
  radio.tcxoVoltage = TCXOVoltage;

  mySerial.print(F("Initializing Module for LoRa ... "));
  ConfigLoRa_t config;
  config.frequency = Frequency;
  config.bandwidth = Bandwidth;
  config.spreadingFactor = SpreadingFactor;
  config.codingRate = CodeRate;
  config.syncWord = LoRaSyncWord;
  config.power = TXPower;
  config.preambleLength = PreambleLength;
  int state = radio.begin(config);

  //Alternative begin, listed in API but now depracated apparently
  //begin (float freq = 868.0, float bw = 125.0, uint8_t sf = 7, uint8_t cr = 5, uint8_t syncWord = RADIOLIB_LR2021_LORA_SYNC_WORD_PRIVATE, int8_t power = 10, uint16_t preambleLength = 8, float tcxoVoltage = 1.60;
  //int state = radio.begin(Frequency, Bandwidth, SpreadingFactor, CodeRate, SyncWord, TXPower, PreambleLength, TCXOVoltage);
  //int state = radio.begin(868.0, 125.0, 7, 5, 0x12, 10, 8, 1.6);

  if (state == RADIOLIB_ERR_NONE) {
    mySerial.println(F("success!"));
  } else {
    while (1) {
      led_Flash(5, 25);
      mySerial.print(F("No device responding, code "));
      mySerial.println(state);
    };
  }

  print_Device_Settings();
  mySerial.println();
}