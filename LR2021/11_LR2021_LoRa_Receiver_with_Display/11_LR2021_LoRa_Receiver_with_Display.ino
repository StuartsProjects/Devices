/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 03/11/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*************************************************************************************************************
  This is a program that uses the LR2021 to receive packets. The packet is in ASCII and the count of packets
  sent is in the packet. 
  
  The file defining the pins for the used microcontroller board such as SEEED_XIAO_Board.h contains the pin 
  definitions used for the LoRa module. You need to connect the LR2021 up to the SPI pins also. 

  Sample serial port output;

  19s,LoRa00011,RSSI,-36dBm,SNR,15dB,Length,10,Packets,11,Errors,0
  20s,LoRa00012,RSSI,-36dBm,SNR,15dB,Length,10,Packets,12,Errors,0
  22s,LoRa00013,RSSI,-36dBm,SNR,15dB,Length,10,Packets,13,Errors,0

*************************************************************************************************************/

/*
  For default module settings, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration#lr2021---lora-modem

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// include the LR2021 library
#include <RadioLib.h>
#include "Settings.h"

#include "SEEED_XIAO_Board.h"  //include if processor board is XIAO
//#include "Arduino_DUE_Board.h"  //include if processor board is Arduino DUE

LR2021 radio = new Module(LORA_NSS, LORA_DIO, LORA_RESET, LORA_BUSY);

uint8_t RXbuffer[10];    //buffer to receive packet, transmitted packet is 10 char array
uint8_t RXPacketL;       //stores length of packet received
int8_t PacketRSSI;       //stores RSSI of received packet
int8_t PacketSNR;        //stores signal to noise ratio (SNR) of received packet
uint32_t RXpacketCount;  //keep count of packets received
uint32_t RXerrors;       //keep count of any packets with errors
int16_t state;           //reported state of radio. functions
bool DISPLAYOK = true;

#ifdef USE_OLED128X64
//libraries for I2C SH1106 OLED
#include <Wire.h>
#include <U8x8lib.h>                                   //get library here >  https://github.com/olikraus/u8g2
U8X8_SH1106_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);  //use this line for 1.3" OLED often sold as 1.3" SSD1306
//U8X8_SSD1306_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE); //use this line for 0.96" SSD1306
#define DEFAULTFONT u8x8_font_chroma48medium8_r  //font for U8X8 Library
#include "OLED128X64.h"                          //library of display functions
#endif

#ifdef USE_LCD20X4
//libraries for I2C LCD via PCF8574 interface, HD44780 type
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>           //get the library here > https://github.com/mathertel/LiquidCrystal_PCF8574
LiquidCrystal_PCF8574 disp(DISPLAYADDRESS);  //set the LCD address to address provided in Settings.h
#include "LCD20X4.h"                         //library of display functions
#endif


void loop() {

  state = radio.receive(RXbuffer, sizeof(RXbuffer), 60000);  //put receive timeout at 60 seconds

  PacketRSSI = radio.getRSSI();         //read the received packets RSSI value
  PacketSNR = radio.getSNR();           //read the received packets SNR value
  RXPacketL = radio.getPacketLength();  //read the length of packet received

  digitalWrite(LED1, LEDON);

  if (state == RADIOLIB_ERR_NONE) {
    packet_is_OK(state);
  } else {
    packet_is_Error(state);
  }

  digitalWrite(LED1, LEDOFF);
}


void packet_is_OK(int16_t state) {
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
  if (DISPLAYOK) {
    display_LoRaRXpacket(RXbuffer, RXPacketL, PacketRSSI, PacketSNR, RXpacketCount, RXerrors, state);
  }
}


void packet_is_Error(int16_t state) {

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
  if (DISPLAYOK) {
    display_LoRaRXpacket(RXbuffer, RXPacketL, PacketRSSI, PacketSNR, RXpacketCount, RXerrors, state);
  }
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


void print_LoRaModemSettings() {
  mySerial.println(F("LoRa Modem Settings"));

  mySerial.print(F("Frequency "));
  mySerial.print(Frequency, 3);
  mySerial.println(F("Khz"));

  mySerial.print(F("Bandwith "));
  mySerial.print(Bandwidth, 3);
  mySerial.println(F("Khz"));

  mySerial.print(F("SpreadingFactor "));
  mySerial.println(SpreadingFactor);

  mySerial.print(F("CodingRate 4:"));
  mySerial.println(CodingRate);

  mySerial.print(F("LoRaSyncWord 0x"));
  mySerial.println(LoRaSyncWord, HEX);

  mySerial.print(F("TXPower "));
  mySerial.print(TXPower);
  mySerial.println(F("dBm"));

  mySerial.print(F("PreambleLength "));
  mySerial.println(PreambleLength);
}


void print_Device_Settings() {
  mySerial.println(F("Device Settings"));

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

  mySerial.print(F("TCXOVoltage "));
  mySerial.print(TCXOVoltage, 1);
  mySerial.println(F("V"));
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
  config.codingRate = CodingRate;
  config.syncWord = LoRaSyncWord;
  config.power = TXPower;
  config.preambleLength = PreambleLength;
  state = radio.begin(config);

  if (state == RADIOLIB_ERR_NONE) {
    mySerial.println(F("success!"));
  } else {
    while (1) {
      led_Flash(5, 25);
      mySerial.print(F("Error Initializing, state "));
      mySerial.println(state);
    };
  }


#ifdef USE_OLED128X64
  if (setup_Display(DISPLAYADDRESS)) {
    mySerial.println("OLED128X64 Display init OK");
    DISPLAYOK = true;
  } else {
    mySerial.println("OLED128X64 Display init Failed");
    DISPLAYOK = false;
  }
#endif

#ifdef USE_LCD20X4
  if (setup_Display(DISPLAYADDRESS)) {
    mySerial.println("LCD20X4 Display init OK");
    DISPLAYOK = true;
  } else {
    mySerial.println("LCD20X4 Display init Failed");
    DISPLAYOK = false;
  }
#endif

  mySerial.println();
  print_Device_Settings();
  mySerial.println();
  print_LoRaModemSettings();
  mySerial.println();

  if (DISPLAYOK) {
    display_LoRaModemSettings(Frequency, Bandwidth, SpreadingFactor, CodingRate, LoRaSyncWord, TXPower, PreambleLength);
    delay(2000);
  }
  mySerial.println(F("Receiver ready"));
}