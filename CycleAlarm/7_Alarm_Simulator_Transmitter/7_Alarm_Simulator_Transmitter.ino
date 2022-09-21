/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 19/09/21

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a program that simulates the operation of the cycle motion\disturb alarm.
  It can be problem, and somewhat deafening, to use a live alarm for testing a receiver. This program will
  transmit beacon signals at a default rate of one every 15 seconds and send an alarm signal every 4 beacon
  signals. The beacon packet is 5 bytes long and the alarm packet is 6 bytes long so the receiver knows
  which one is which. An alarm packet and a beacon packet is sent as soon as the program starts.

  Use with matching program 6_Alarm_Receiver.

  Serial monitor baud rate should be set at 9600.
*******************************************************************************************************/

#include <SPI.h>                                  //the LoRa device is SPI based so load the SPI library                                         
#include <SX127XLT.h>                             //include the appropriate library  

SX127XLT LoRa;                                    //create a library class instance called LoRa

#define NSS 10                                    //select pin on LoRa device
#define NRESET 9                                  //reset pin on LoRa device
#define DIO0 3                                    //DIO0 pin on LoRa device, used for sensing RX and TX done 
#define LED1 8                                    //pin number for LED, set logic level high for on

#define LORA_DEVICE DEVICE_SX1278                 //we need to define the device we are using
#define TXpower 10                                //LoRa transmit power in dBm

#define ACKtimeout 3000                           //Wait acknowledge timeout in mS, set to 0 if ACK not used.                      
#define TXtimeout 2000                            //transmit timeout in mS. If 0 return from transmit function after send.  
#define TXattempts 10                             //number of times to attempt to TX and get an Ack before failing
#define BeaconPeriodmS 15000;                     //mS between beacons 

uint8_t TXPacketL;                                //length of transmitted packet
uint8_t TXPayloadL;                               //this is the payload length sent
uint8_t RXPacketL;                                //length of received acknowledge
uint16_t PayloadCRC;
uint8_t BatteryVolts;
uint8_t beaconcount = 4;                          //send this number of beacons, then alarm

const uint16_t NetworkID = 0x3210;                //NetworkID identifies this connection, needs to match value in receiver

uint8_t ThisNode = '1';                           //number of this node, '1' displays as number 49 on receiver.


void loop()
{
  //beacon packet is 5 bytes long
  //will send an alarm every 4 beacons
  uint32_t nextBeaconmS;

  if (beaconcount == 0)
  {
    SendAlarm();
    beaconcount = 4;
  }

  nextBeaconmS = millis() + BeaconPeriodmS;
  while (millis() < nextBeaconmS);
  beaconcount--;
  SendBeacon();
  nextBeaconmS = millis() + BeaconPeriodmS;
}


void SendBeacon()
{
  Serial.println(F("Sending Beacon"));
  LoRa.setupLoRa(434000000, 0, LORA_SF12, LORA_BW_062, LORA_CR_4_5, LDRO_AUTO); //configure frequency and LoRa settings
  LoRa.startWriteSXBuffer(0);                        //start the write at SX12XX internal buffer location 0
  LoRa.writeUint8(ThisNode);                         //node sending the packet
  TXPayloadL = LoRa.endWriteSXBuffer();              //closes packet write and returns the length of the payload to send
  digitalWrite(LED1, HIGH);
  TXPacketL = LoRa.transmitSXReliable(0, TXPayloadL, NetworkID, TXtimeout, TXpower, WAIT_TX);
  digitalWrite(LED1, LOW);
}




void SendAlarm()
{
  //alarm packet is 6 bytes long
  BatteryVolts = (4250 - 2500) / 10;                 //convert battery volts into units of 10mV above 2500mV

  LoRa.setupLoRa(434000000, 0, LORA_SF12, LORA_BW_062, LORA_CR_4_5, LDRO_AUTO); //configure frequency and LoRa settings

  //now transmit the packet and keep transmitting until the acknowledge is received

  uint8_t attempts = TXattempts;

  do
  {
    LoRa.startWriteSXBuffer(0);                       //start the write at SX12XX internal buffer location 0
    LoRa.writeUint8(ThisNode);                        //node sending the packet
    LoRa.writeUint8(BatteryVolts);                    //battery volts in units of 10mV above 2500mV
    TXPayloadL = LoRa.endWriteSXBuffer();             //closes packet write and returns the length of the payload to send

    Serial.print(F("Sending Alarm > "));
    LoRa.printSXBufferHEX(0, TXPayloadL - 1);         //print the sent SX array as HEX
    Serial.print(F("Send attempt "));
    Serial.println(TXattempts - attempts + 1);
    Serial.println();
    Serial.flush();

    digitalWrite(LED1, HIGH);
    TXPacketL = LoRa.transmitSXReliable(0, TXPayloadL, NetworkID, TXtimeout, TXpower, WAIT_TX);  //will return packet length > 0 if sent OK, otherwise 0 if transmit error
    attempts--;
    digitalWrite(LED1, LOW);

    PayloadCRC = LoRa.getTXPayloadCRC(TXPacketL);      //read the payload CRC sent

    RXPacketL = LoRa.waitReliableACK(NetworkID, PayloadCRC, ACKtimeout);

    if (RXPacketL > 0)
    {
      //if waitReliableACK() returns > 0 then valid ack was received
      packet_is_OK();
      Serial.println();
      Serial.println(F("Ack Received"));
      led_Flash(1, 125);
    }
    else
    {
      //if transmitReliable() returns 0 there is an error
      packet_is_Error();
      Serial.println();
      Serial.println(F("No Ack Received"));
    }
    delay(250);                                                   //small delay between tranmission attampts
  }
  while ((RXPacketL == 0) && (attempts != 0));                    //loop attempts times waiting for an ACK
  Serial.println();

}


void packet_is_OK()
{
  Serial.print(F("LocalNetworkID,0x"));
  Serial.print(NetworkID, HEX);
  Serial.print(F(",TransmittedPayloadCRC,0x"));                   //print CRC of transmitted packet
  Serial.print(PayloadCRC, HEX);
}


void packet_is_Error()
{
  Serial.print(F("SendError"));
  LoRa.printIrqStatus();                                 //prints the text of which IRQs set
  LoRa.printReliableStatus();                            //print the reliable status
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  //general purpose routine for flashing LED as indicator
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, HIGH);                  //LED on
    delay(delaymS);
    digitalWrite(LED1, LOW);                   //LED off
    delay(delaymS);
  }
}


void setup()
{
  pinMode(LED1, OUTPUT);
  led_Flash(2, 125);                            //two quick LED flashes to indicate program start

  Serial.begin(9600);
  Serial.println();
  Serial.println(F("7_Alarm_Simulator_Transmitter Starting"));

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

  LoRa.setupLoRa(434000000, 0, LORA_SF12, LORA_BW_062, LORA_CR_4_5, LDRO_AUTO); //configure frequency and LoRa settings

  Serial.println(F("Alarm Simulator ready"));
  Serial.println();

  SendAlarm();
  delay(500);
  SendBeacon();
  delay(5000);
}
