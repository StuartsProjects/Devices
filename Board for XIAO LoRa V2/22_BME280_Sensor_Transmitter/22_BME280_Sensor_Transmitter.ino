/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 06/02/23

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This is a working demontration of using a XIAO to read a BME280 sensor and transmit
  the sensor readings in a LoRa packet.

  The transmitted packets contain two protection methods so that the receiver will know if they are
  geniune packets from a known transmitter. The transmitted packet is protected with a 'NetworkID' and a
  payload CRC that are automatically appended to the sent packet. The receiver needs to have a matching
  'NetworkID' or the packet is rejected. In addition the receiver checks the payload CRC appended to the
  packet agianst the CRC of the received payload. These two checks mean that it is extremly unlikley that
  invalid data, such as a packet from a foreign transmitter is accepted.

  When the packet is transmitted it will continue to be sent until an acknowledgement is received. The
  number of transmit attempts is set by the constant TXattempts.

  When the transmission of the sensor data is complete, or the number of transmit attempts exceeds the set
  value the XIAO is put to sleep to wake up using the RTC timer for a number of seconds set by the constant
  SleepSeconds. Sleep current with the power LED fitted, 1436uA, with the LED removed, 10uA.

  Use program 23_BME280_Sensor_Receiver_With_Display and another XIAO LoRa board as the receiver. 

  Serial monitor baud rate should be set at 115200.

  *******************************************************************************************************/

#include <SPI.h>                                //the LoRa device is SPI based so load the SPI library                                         
#include <SX127XLT.h>                           //get library here > https://github.com/StuartsProjects/SX12XX-LoRa 
#include <ProgramLT_Definitions.h>

SX127XLT LoRa;                                  //create a library class instance called LoRa

#include <Wire.h>
#include <Seeed_BME280.h>                       //get library here; https://github.com/Seeed-Studio/Grove_BME280
BME280 bme280;                                  //create an instance of the BME280 senosor

#include "ArduinoLowPower.h"                    //get library here > https://github.com/arduino-libraries/ArduinoLowPower

// *******  Setup hardware pin definitions here ! ***************

#define NSS A3                                  //select pin on LoRa device, matching D3 pins not recognised ?
#define NRESET A2                               //reset pin on LoRa device
#define DIO0 A1                                 //DIO0 pin on LoRa device, used for sensing RX and TX done 
#define LED1 13                                 //on board LED, yellow    
#define ADMultiplier 1.623                     //adjustment to convert AD value read into mV of battery voltage 
#define DiodemV 181  
#define SupplyAD A0                             //Resitor divider for battery voltage read connected here 

#define LORA_DEVICE DEVICE_SX1278               //this is the device we are using

// *******  Setup LoRa Parameters Here ! ***************

const uint32_t Frequency = 434000000;           //frequency of transmissions in hertz
const uint32_t Offset = 0;                      //offset frequency for calibration purposes

const uint8_t Bandwidth = LORA_BW_125;          //LoRa bandwidth
const uint8_t SpreadingFactor = LORA_SF7;       //LoRa spreading factor
const uint8_t CodeRate = LORA_CR_4_5;           //LoRa coding rate
const uint8_t Optimisation = LDRO_AUTO;         //low data rate optimisation setting
const int8_t TXpower = 10;                      //LoRa transmit power in dBm

const uint16_t TXattempts = 5;                  //number of times to transmit packet and wait for an acknowledge 
const uint32_t SleepSeconds = 60;               //number of seconds spent asleep
const uint32_t ACKtimeout = 1000;               //Acknowledge timeout in mS                      
const uint32_t TXtimeout = 5000;                //transmit timeout in mS. If 0 return from transmit function after send.  

const uint8_t BME280ADDRESS = 0x76;             //I2C bus address of BME280, common default 
const uint8_t BME280_REGISTER_CONTROL = 0xF4;   //BME280 register number for power control

const uint16_t NetworkID = 0x3210;              //NetworkID identifies this connection, needs to match value in receiver
const uint8_t ThisNode = 49;                    //source node 49 = '1'

uint16_t PayloadCRC;                            //CRC of the payload
uint8_t TXPacketL;                              //length of transmitted packet
uint8_t TXPayloadL;                             //this is the payload length sent
uint16_t attempts;                              //number of times to attempt to TX and get an Ack before failing
uint8_t TXDestination;                          //* is a broadcast

float temperature;                              //the BME280 temperature value
float pressure;                                 //the BME280 pressure value
uint16_t humidity;                              //the BME280 humididty value
uint16_t TXVolts;                               //the battery voltage value
uint8_t statusbyte;                             //a status byte, not currently used


void loop()
{
  uint16_t attempts;

  readSensors();                                //read the sensor values
  printSensorValues();                          //print the sensor values
  attempts = TXattempts;                        //set the number of times to TX and wait for ACK, see settings.h  

  //keep transmitting the packet until an ACK is received
  do
  {
    TXPayloadL = build_Packet();

    Serial.print(F("Transmit SX buffer > "));
    LoRa.printSXBufferHEX(0, TXPayloadL - 1);   //print the sent SX buffer as HEX
    Serial.println();
    Serial.flush();

    digitalWrite(LED1, LOW);
    TXPacketL = LoRa.transmitSXReliableAutoACK(0, TXPayloadL, NetworkID, ACKtimeout, TXtimeout, TXpower, WAIT_TX);
    digitalWrite(LED1, HIGH);
    attempts--;

    if (TXPacketL > 0)
    {
      //if transmitSXReliableAutoACK() returns > 0 then transmit and ack was OK
      PayloadCRC = LoRa.getTXPayloadCRC(TXPacketL);    //read the actual transmitted CRC from the LoRa device buffer
      packet_is_OK();
      Serial.println();
    }
    else
    {
      //if transmitReliable() returns 0 there was an error
      packet_is_Error();
      Serial.println();
    }
    delay(500);                                        //small delay between tranmission attampts
  }
  while ((TXPacketL == 0) && (attempts != 0));

  Serial.println();
  Serial.flush();

  sleepBME280();                                       //sleep the BME280
  LoRa.setSleep(CONFIGURATION_RETENTION);              //preserve LoRa register settings in sleep.

  Serial.println(F("Sleeping"));
  Serial.flush();
  digitalWrite(LED1, HIGH);                            //LED off

  LowPower.sleep(SleepSeconds * 1000);

  digitalWrite(LED1, LOW);                             //LED on while awake
  Serial.println(F("Wakeup !"));
  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);
  normalBME280();                                      //BME280 sensor to normal mode
}


void normalBME280()
{
  //write this register value to BME280 to put it to read mode
  writeBME280reg(BME280_REGISTER_CONTROL, B01111111);
}


void sleepBME280()
{
  //write this register value to BME280 to put it to sleep
  writeBME280reg(BME280_REGISTER_CONTROL, B01111100);
}


void writeBME280reg(byte reg, byte regvalue)
{
  //write a register value to the BME280
  Wire.beginTransmission((uint8_t) BME280_ADDRESS);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)regvalue);
  Wire.endTransmission();
}


void readSensors()
{
  //read the sensor values into the global variables
  temperature = bme280.getTemperature();
  pressure = bme280.getPressure();
  humidity = bme280.getHumidity();

  TXVolts = readSupplyVoltage();             //read resistor divider across battery
  statusbyte = 0x55;                         //manually set this for now, its a test
}


uint16_t readSupplyVoltage()
{
  //returns supply in mV @ 10mV per AD bit read
  uint16_t temp;
  uint16_t volts = 0;
  byte index;

  analogReadResolution(ADC_RESOLUTION);
  pinMode(SupplyAD, INPUT);
  //Serial.printf("Set the Analog to digital converter resolution to %d bits\n", ADC_RESOLUTION);
  //Serial.printf("Initialized pin A%d as an input.\n", SupplyAD);

  temp = analogRead(SupplyAD);              //do a null read to start

  for (index = 0; index <= 4; index++)      //sample AD 5 times
  {
    temp = analogRead(SupplyAD);
    volts = volts + temp;
  }
  volts = ((volts / 5) * ADMultiplier);

  return volts + DiodemV;
}


void printSensorValues()
{
  Serial.print(F("Temperature,"));
  Serial.print(temperature, 1);
  Serial.print(F("c,Pressure,"));
  Serial.print(pressure, 0);
  Serial.print(F("Pa,Humidity,"));
  Serial.print(humidity);
  Serial.print(F("%,Voltage,"));
  Serial.print(TXVolts);
  Serial.print(F("mV,Status,"));
  Serial.print(statusbyte, HEX);
  Serial.print(F("  "));
  Serial.flush();
}


uint8_t build_Packet()
{
  uint8_t len;
  LoRa.startWriteSXBuffer(0);                 //start the write packet to buffer process
  LoRa.writeUint8(Sensor1);                   //this byte defines the packet type
  LoRa.writeUint8(TXDestination);             //destination address of the packet, the receivers address
  LoRa.writeUint8(ThisNode);                  //source address of this node
  LoRa.writeFloat(temperature);               //add the BME280 temperature
  LoRa.writeFloat(pressure);                  //add the BME280 pressure
  LoRa.writeUint16(humidity);                 //add the BME280 humididty
  LoRa.writeUint16(TXVolts);                  //add the battery voltage
  LoRa.writeUint8(statusbyte);                //add the status byte
  len = LoRa.endWriteSXBuffer();              //close the packet, get the length of data to be sent

  return len;
}


void packet_is_OK()
{
  Serial.println(F("Packet acknowledged"));
  Serial.print(F("LocalNetworkID,0x"));
  Serial.print(NetworkID, HEX);
  Serial.print(F(",TransmittedPayloadCRC,0x"));        //print CRC of transmitted payload
  Serial.print(PayloadCRC, HEX);
}


void packet_is_Error()
{
  Serial.print(F("No Packet acknowledge"));
  LoRa.printIrqStatus();                               //prints the text of which IRQs set
  LoRa.printReliableStatus();                          //print the reliable status
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
  pinMode(LED1, OUTPUT);
  led_Flash(2, 125);

  bme280.init(BME280ADDRESS);
  readSensors();

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("22_BME280_Sensor_Transmitter"));

  pinMode(A9, INPUT_PULLUP);                    //pin to INPUT_PULLUP to reduce sleep current

  SPI.begin();

  if (LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("LoRa Device found"));
    led_Flash(2, 125);
  }
  else
  {
    Serial.println(F("No LoRa device responding"));
    while (1) led_Flash(10, 25);
  }

  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  Serial.println(F("Transmitter ready"));
  Serial.println();
  digitalWrite(LED1, LOW);                     //LED on while awake
}
