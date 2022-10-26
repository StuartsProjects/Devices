/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 18/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This is a working demontration of using a XIAO to read a BME280 sensor and transmit
  the reading with a LoRa packet.

  The transmitted packet is protected with a 'NetworkID' and a payload CRC that are automatically appended
  to the transmitted packet. The receiver needs to have a matching 'NetworkID' or the packet is rejected.
  In addition the receiver checks the payload CRC appended to the packet agianst the CRC of the received
  payload. These two checks mean that it is extremly unlikley that invalid data, such as a packet from a
  foreign transmitter is accepted.

  When the packet is transmitted it will continue to be sent until an acknowledgement is received. The
  number of transmit attempts is set by the constant TXattempts.

  When the transmission of the sensor data is complete, or the number of transmit attempts exceeds the set
  value the XIAO is put to sleep to wake up usin the RTC alarm for a number of seconds set by the constant
  SleepSeconds.

  Use program 18_BME280_Sensor_Receiver and another XIAO LoRa board as the receiver. Sleep current with the
  power LED fitted, 1436uA, withe the LED removed, 8uA.

  Serial monitor baud rate should be set at 115200.

  *******************************************************************************************************/

#include <SPI.h>                                //the LoRa device is SPI based so load the SPI library                                         
#include <SX127XLT.h>                           //get library here > https://github.com/StuartsProjects/SX12XX-LoRa 
#include <ProgramLT_Definitions.h>
#include "Settings.h"

SX127XLT LoRa;                                  //create a library class instance called LoRa

#include <Seeed_BME280.h>                       //get library here; https://github.com/Seeed-Studio/Grove_BME280
BME280 bme280;                                  //create an instance of the BME280 senosor
#include <Wire.h>

#include <RTCZero.h>                            //get the library here > https://github.com/arduino-libraries/RTCZero  
#include "ArduinoLowPower.h"                    //get library here > https://github.com/arduino-libraries/ArduinoLowPower
RTCZero rtc;                                    //Create an RTCZero object

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
bool alarm = false;


void loop()
{
  uint16_t attempts;

  readSensors();                                //read the sensor values
  printSensorValues();                          //print the sensor values
  attempts = TXattempts;

  //keep transmitting the packet until an ACK is received
  do
  {
    TXPayloadL = build_Packet();

    Serial.print(F("Transmit SX buffer > "));
    LoRa.printSXBufferHEX(0, TXPayloadL - 1);   //print the sent SX buffer as HEX
    Serial.println();
    Serial.flush();

    TXPacketL = LoRa.transmitSXReliableAutoACK(0, TXPayloadL, NetworkID, ACKtimeout, TXtimeout, TXpower, WAIT_TX);
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

  set_Alarm(SleepSeconds);                             //set sleep seconds
  LowPower.deepSleep();

  digitalWrite(LED1, LOW);                             //LED on while awake
  alarm = false;
  rtc.disableAlarm();
  Serial.println(F("Alarm !"));
  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);
  normalBME280();                                      //BME280 sensor to normal mode
}


void set_Alarm(uint32_t alarmseconds)
{
  uint8_t hours, mins, seconds;
  uint32_t secondsleft;

  hours = alarmseconds / 3600;
  secondsleft = alarmseconds - (hours * 3600);
  mins = secondsleft / 60;
  seconds = secondsleft - (mins * 60);

  Serial.print(F("Alarm > Hours,"));
  Serial.print(hours);
  Serial.print(F(",Minutes,"));
  Serial.print(mins);
  Serial.print(F(",Seconds,"));
  Serial.println(seconds);

  rtc.setTime(0, 0, 0);
  rtc.setAlarmTime(hours, mins, seconds);                   //set alarm for alarmseconds
  rtc.attachInterrupt(alarmMatch);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  return;
}


void alarmMatch()
{
  alarm = true;
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

  return volts;
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

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("17_BME280_Sensor_Transmitter Starting"));

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

  bme280.init();
  rtc.begin();                                 //initialize RTC 24H format

  Serial.println(F("Transmitter ready"));
  Serial.println();
  digitalWrite(LED1, LOW);                     //LED on while awake
}
