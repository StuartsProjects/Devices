/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 06/02/23

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a working demontration of using a XIAO to receive sensor data from a
  transmitter with a BME280 sensor.

  The transmitted packet is protected with a 'NetworkID' and a payload CRC that are automatically appended
  to the transmitted packet. This receiver needs to have a matching 'NetworkID' or the packet is rejected.
  In addition this receiver checks the payload CRC appended to the packet agianst the CRC of the received
  payload. These two checks mean that it is extremly unlikley that invalid data, such as a packet from a
  foreign transmitter is accepted.

  When the packet is received an acknowledgement is sent, the transmitter will continue to send packets until
  an acknowledgement is received or their is a timeout.

  When a valid sensor packet is received the data is displayed on the OLED display. The transmitter program
  should be 12_BME280_Sensor_Transmitter.
  
  Serial monitor baud rate should be set at 115200.
*******************************************************************************************************/

#include <SPI.h>                                //the LoRa device is SPI based so load the SPI library
#include <SX127XLT.h>                           //include the appropriate library   
#include <ProgramLT_Definitions.h>

SX127XLT LoRa;                                  //create a library class instance called LoRa

#include <U8x8lib.h>                                        //get library here > https://github.com/olikraus/u8g2 
//U8X8_SSD1306_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);    //standard 0.96" SSD1306
U8X8_SH1106_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);       //1.3" OLED often sold as 1.3" SSD1306


// *******  Setup hardware pin definitions here ! ***************

#define NSS A3                                  //select pin on LoRa device, matching D3 pins not recognised ?
#define NRESET A2                               //reset pin on LoRa device
#define DIO0 A1                                 //DIO0 pin on LoRa device, used for sensing RX and TX done 
#define LED1 13                                 //on board LED, yellow    
#define LORA_DEVICE DEVICE_SX1278               //this is the device we are using

// *******  Setup LoRa Parameters Here ! ***************

const uint32_t Frequency = 434000000;           //frequency of transmissions in hertz
const uint32_t Offset = 0;                      //offset frequency for calibration purposes

const uint8_t Bandwidth = LORA_BW_125;          //LoRa bandwidth
const uint8_t SpreadingFactor = LORA_SF7;       //LoRa spreading factor
const uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting
const int8_t TXpower = 10;                      //LoRa transmit power in dBm

const uint32_t ACKdelay = 100;                  //delay in mS before sending acknowledge                    
const uint32_t RXtimeout = 300000;              //receive timeout in mS.  

const uint16_t NetworkID = 0x3210;              //NetworkID identifies this connection, needs to match value in receiver

uint32_t ValidPackets;                          //count of packets received with valid data
uint32_t RXpacketErrors;                        //count of all packets with errors received
uint8_t RXPacketL;                              //stores length of packet received
uint8_t RXPayloadL;                             //stores length of payload received
uint8_t PacketOK;                               //set to > 0 if packetOK
int16_t PacketRSSI;                             //stores RSSI of received packet
uint16_t LocalPayloadCRC;                       //locally calculated CRC of payload
uint16_t RXPayloadCRC;                          //CRC of payload received in packet
uint16_t TransmitterNetworkID;                  //the NetworkID from the transmitted and received packet

uint8_t RXPacketType;
uint8_t RXDestination;
uint8_t RXSource;
float temperature;                               //the BME280 temperature value
float pressure;                                  //the BME280 pressure value
uint16_t humidity;                               //the BME280 humididty value
uint16_t TXVolts;                                //the battery voltage value of transmitter
uint8_t statusbyte;                              //a status byte, not currently used



void loop()
{
  PacketOK = LoRa.receiveSXReliableAutoACK(0, NetworkID, ACKdelay, TXpower, RXtimeout, WAIT_RX); //wait for a packet to arrive with 60seconds (60000mS) timeout

  RXPacketL = LoRa.readRXPacketL();                //get the received packet length
  RXPayloadL = RXPacketL - 4;

  PacketRSSI = LoRa.readPacketRSSI();              //read the received packets RSSI value

  LoRa.startReadSXBuffer(0);
  RXPacketType = LoRa.readUint8();                 //the packet type received
  RXDestination = LoRa.readUint8();                //the destination address the packet was sent to
  RXSource = LoRa.readUint8();                     //the source address, where the packet came from
  temperature = LoRa.readFloat();                  //the BME280 temperature value
  pressure = LoRa.readFloat();                     //the BME280 pressure value
  humidity = LoRa.readUint16();                    //the BME280 humididty value
  TXVolts = LoRa.readUint16();                     //the battery voltage value
  statusbyte = LoRa.readUint8();                   //a status byte, not currently used
  LoRa.endReadSXBuffer();

  if (PacketOK > 0)
  {
    //if the LoRa.receiveReliable() returns a value > 0 for PacketOK then packet was received and ack sent
    ValidPackets++;
    Serial.print(F("Payload received OK > "));
    LoRa.printSXBufferHEX(0, (RXPayloadL - 1));
    Serial.println();
    printSensorValues();
    displayscreen1();
  }
  else
  {
    //if the LoRa.receiveReliable() function detects an error PacketOK is 0
    RXpacketErrors++;
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

  IRQStatus = LoRa.readIrqStatus();                  //read the LoRa device IRQ status register
  Serial.print(F("Error "));

  if (IRQStatus & IRQ_RX_TIMEOUT)                  //check for an RX timeout
  {
    Serial.print(F(" RXTimeout "));
  }
  else
  {
    printPacketDetails();
  }
}


void printSensorValues()
{
  Serial.print(F("SourceNode,"));
  Serial.print(RXSource);
  Serial.print(F(",Temperature,"));
  Serial.print(temperature, 1);
  Serial.print(F("c,Pressure,"));
  Serial.print(pressure, 0);
  Serial.print(F("Pa,Humidity,"));
  Serial.print(humidity);
  Serial.print(F("%,Voltage,"));
  Serial.print(TXVolts);
  Serial.print(F("mV,Status,"));
  Serial.print(statusbyte, HEX);
  Serial.println();
  Serial.flush();
}


void displayscreen1()
{
  //show sensor data on display
  disp.clearLine(0);
  disp.setCursor(0, 0);
  disp.print(F("Sensor "));
  disp.print(RXSource);
  disp.clearLine(1);
  disp.setCursor(0, 1);
  disp.print(temperature, 1);
  disp.print(F("c"));
  disp.clearLine(2);
  disp.setCursor(0, 2);
  disp.print(pressure, 0);
  disp.print(F("Pa"));
  disp.clearLine(3);
  disp.setCursor(0, 3);
  disp.print(humidity);
  disp.print(F("%"));
  disp.clearLine(4);
  disp.setCursor(0, 4);
  disp.print(TXVolts);
  disp.print(F("mV"));
  disp.clearLine(6);
  disp.setCursor(0, 6);
  disp.print(F("ValidPkts "));
  disp.print(ValidPackets);
  disp.setCursor(0, 7);
  disp.print(F("Errors "));
  disp.print(RXpacketErrors);
}




void printPacketDetails()
{
  LocalPayloadCRC = LoRa.CRCCCITTReliable(0, (RXPayloadL - 1), 0xFFFF);  //calculate payload crc from the received RXBUFFER
  TransmitterNetworkID = LoRa.getRXNetworkID(RXPacketL);
  RXPayloadCRC = LoRa.getRXPayloadCRC(RXPacketL);

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



void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, LOW);
    delay(delaymS);
    digitalWrite(LED1, HIGH);
    delay(delaymS);
  }
}


void setup()
{
  pinMode(LED1, OUTPUT);                        //setup pin as output for indicator LED
  led_Flash(2, 125);                            //two quick LED flashes to indicate program start

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("23_BME280_Sensor_Receiver"));

  disp.begin();
  disp.setFont(u8x8_font_chroma48medium8_r);
  Serial.print(F("Checking LoRa device - "));         //Initialize LoRa
  disp.setCursor(0, 0);

  SPI.begin();

  if (LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("LoRa Device found"));
    led_Flash(2, 125);
  }
  else
  {
    Serial.println(F("No LoRa device responding"));
    disp.print(F("No LoRa device"));
    while (1)
    {
      led_Flash(50, 50);                               //long fast speed flash indicates device error
    }
  }


  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);   //configure frequency and LoRa settings
  pinMode(DIO0, INPUT_PULLDOWN);                                                           //prevent A1 floating if diodes in use as per TTN setup

  Serial.println(F("Receiver ready"));
  disp.print(F("Receiver ready"));
 
}
