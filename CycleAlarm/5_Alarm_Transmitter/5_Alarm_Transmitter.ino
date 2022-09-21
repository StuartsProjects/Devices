/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 13/09/21

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a program for a LoRa transmitter thats used as a range extender for a cycle
  motion\disturb alarm. The program was tested on an ATmega328P processor.

  When the horn on the Alarm sounds a pulse is detected on pin A2 of the ATMega328P micro controller which
  wakes it up from low current deep sleep.

  The transmitter then sends a reliable packet from the SX127XLT library, this packet has 4 bytes automatically
  appended to the end of the buffer\array that is the data payload. The first two bytes appended are a 16bit
  'NetworkID'. The receiver needs to have the same NetworkID as configured for the transmitter since
  the receiver program uses the NetworkID to check that the received packet is from a known source.
  The third and fourth bytes appended are a 16 bit CRC of the payload. The receiver will carry out its own
  CRC check on the received payload and can then verify this against the CRC appended in the packet.
  The receiver is thus able to check if the payload is valid. This gives a gives a high level of assurance
  that the packet is valid.

  If the received packet is valid then the networkID and payload CRC are returned in a 4 byte packet as an
  acknowledgement that the transmitter listens for. If the transmitter does not receive the acknowledgement
  within the ACKtimeout period, the original packet is re-transmitted until a valid acknowledgement is
  received.

  If when the Alarm sounds the micro controller detects that a switch on pin A4 is shorted to ground the
  micro controller will go into Beacon mode. Whilst still monitoring the horn for sounds the transmitter
  will send a beacon packet every 15 seconds. The receiver knows its a beacon packet and can click the
  buzzer or flash an LED. If the beacon mode is activated on the transmitter but the receiver does not
  indicate receipt of the beacon packets then the receiver is out of range. The transmitter should not be
  left in beacon mode permanently as this will drain the battery. To turn off beacon mode release the switch
  on pin A4 and use the Alarm remote to sound the Alarm horn.

  Use with matching program 6_Alarm_Receiver.

  Serial monitor baud rate should be set at 9600.
*******************************************************************************************************/

#include <avr/sleep.h>
#include "PinChangeInterrupt.h"                   //get the library here; https://github.com/NicoHood/PinChangeInterrupt

#include <SPI.h>                                  //the LoRa device is SPI based so load the SPI library                                         
#include <SX127XLT.h>                             //include the appropriate library  

SX127XLT LoRa;                                    //create a library class instance called LoRa

#define NSS 10                                    //select pin on LoRa device
#define NRESET 9                                  //reset pin on LoRa device
#define DIO0 3                                    //DIO0 pin on LoRa device, used for sensing RX and TX done 
#define LED1 8                                    //pin number for LED, set logic level high for on
#define BATVREADON 8                              //used to turn on the resistor divider to measure voltage
#define SWITCH1 A2                                //pin number for wakeup 
#define SWITCH2 A4                                //pin number beacon mode switch
#define SupplyAD A7                               //AD pin to read supply voltage  

#define LORA_DEVICE DEVICE_SX1278                 //we need to define the device we are using
#define TXpower 10                                //LoRa transmit power in dBm

#define ADMultiplier 10.36                        //adjustment to convert AD value read into mV of battery voltage 
#define DiodemV 180                               //typical voltage drop across diode in battery lead

#define ACKtimeout 3000                           //Wait acknowledge timeout in mS, set to 0 if ACK not used.                      
#define TXtimeout 2000                            //transmit timeout in mS. If 0 return from transmit function after send.  
#define TXattempts 10                             //number of times to attempt to TX and get an Ack before failing
#define BeaconPeriodmS 15000;                     //mS between beacons 

uint8_t TXPacketL;                                //length of transmitted packet
uint8_t TXPayloadL;                               //this is the payload length sent
uint8_t RXPacketL;                                //length of received acknowledge
uint16_t PayloadCRC;
uint8_t BatteryVolts;

const uint16_t NetworkID = 0x3210;                //NetworkID identifies this connection, needs to match value in receiver

uint8_t ThisNode = '1';                           //number of this node
bool BeaconMode = false;
bool Alarm = false;


void loop()
{
  if (digitalRead(SWITCH2))
  {
    BeaconMode = false;
    Serial.println(F("Normal Mode"));
  }
  else
  {
    BeaconMode = true;
    SendBeacon();
  }

  LoRa.setSleep(CONFIGURATION_RETENTION);                      //preserve register settings in sleep.
  Serial.println(F("Sleeping zzzzz...."));
  Serial.flush();
  digitalWrite(LED1, LOW);
  sleep_permanent();                                           //goto sleep till woken up by switch press

  Serial.println(F("Awake !"));
  Serial.flush();
  sendAlarm();

}


void sendAlarm()
{
  //alarm packet is 6 bytes long
  BatteryVolts = readSupplyVoltage();

  Serial.print(F("Battery "));
  Serial.print((BatteryVolts * 10) + 2500);
  Serial.println(F("mV"));

  LoRa.wake();
  LoRa.setupLoRa(434000000, 0, LORA_SF12, LORA_BW_062, LORA_CR_4_5, LDRO_AUTO); //configure frequency and LoRa settings

  //now transmit the packet and keep transmitting until the acknowledge is received

  uint8_t attempts = TXattempts;

  do
  {
    LoRa.startWriteSXBuffer(0);                        //start the write at SX12XX internal buffer location 0
    LoRa.writeUint8(ThisNode);                         //node sending the packet
    LoRa.writeUint8(BatteryVolts);                     //battery volts in units of 10mV above 2500mV
    TXPayloadL = LoRa.endWriteSXBuffer();              //closes packet write and returns the length of the payload to send

    Serial.print(F("Sending Alarm > "));
    LoRa.printSXBufferHEX(0, TXPayloadL - 1);          //print the sent SX array as HEX
    Serial.print(F("Send attempt "));
    Serial.println(TXattempts - attempts + 1);
    Serial.println();
    Serial.flush();

    digitalWrite(LED1, HIGH);
    TXPacketL = LoRa.transmitSXReliable(0, TXPayloadL, NetworkID, TXtimeout, TXpower, WAIT_TX);  //will return packet length > 0 if sent OK, otherwise 0 if transmit error
    attempts--;
    digitalWrite(LED1, LOW);

    PayloadCRC = LoRa.getTXPayloadCRC(TXPacketL);       //read the payload CRC sent

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
  Alarm = false;
  Serial.println();

}


void SendBeacon()
{
  //beacon packet is 5 bytes long
  uint32_t nextBeaconmS;
  nextBeaconmS = millis() + BeaconPeriodmS;
  attachPCINT(digitalPinToPCINT(SWITCH1), wakeUp, HIGH);         //This is a hardware interrupt
  Serial.println(F("BeaconMode"));

  do
  {
    if (Alarm)                                                   //check for alarm condition
    {
      sendAlarm();
      Alarm = false;
    }

    if (millis() > nextBeaconmS)
    {
      Serial.println(F("Sending Beacon"));
      LoRa.setupLoRa(434000000, 0, LORA_SF12, LORA_BW_062, LORA_CR_4_5, LDRO_AUTO); //configure frequency and LoRa settings
      LoRa.startWriteSXBuffer(0);                        //start the write at SX12XX internal buffer location 0
      LoRa.writeUint8(ThisNode);                         //node sending the packet
      TXPayloadL = LoRa.endWriteSXBuffer();              //closes packet write and returns the length of the payload to send
      digitalWrite(LED1, HIGH);
      TXPacketL = LoRa.transmitSXReliable(0, TXPayloadL, NetworkID, TXtimeout, TXpower, WAIT_TX);
      digitalWrite(LED1, LOW);
      nextBeaconmS = millis() + BeaconPeriodmS;
    }

  }
  while (!digitalRead(SWITCH1));                                     //repeat loop if beacn switch active
  return;
}


void sleep_permanent()
{
  attachPCINT(digitalPinToPCINT(SWITCH1), wakeUp, HIGH);  //This is a hardware interrupt

  ADCSRA = 0;                                             //disable ADC
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  noInterrupts ();                                        //timed sequence follows
  sleep_enable();

  //turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);                       //turn on brown-out enable select
  MCUCR = bit (BODS);                                     //this must be done within 4 clock cycles of above
  interrupts ();                                          //guarantees next instruction executed

  sleep_cpu ();                                           //sleep within 3 clock cycles of above

  /* wake up here */

  sleep_disable();
  ADCSRA = 0x86;                                          //restore ADCSRA register so that AD works
  detachPCINT(digitalPinToPCINT(SWITCH1));
}


void wakeUp()
{
  //handler for the interrupt
  Alarm = true;
}


void packet_is_OK()
{
  Serial.print(F("LocalNetworkID,0x"));
  Serial.print(NetworkID, HEX);
  Serial.print(F(",TransmittedPayloadCRC,0x"));          //print CRC of transmitted packet
  Serial.print(PayloadCRC, HEX);
}


void packet_is_Error()
{
  Serial.print(F("SendError"));
  LoRa.printIrqStatus();                                 //prints the text of which IRQs set
  LoRa.printReliableStatus();                            //print the reliable status
}


uint8_t readSupplyVoltage()
{
  //relies on internal reference and 91K & 11K resistor divider
  //returns supply in mV @ 10mV per AD bit read
  uint16_t temp;
  uint16_t volts = 0;
  uint8_t index;

  digitalWrite(BATVREADON, HIGH);            //turn on MOSFET connection resistor divider in circuit

  analogReference(INTERNAL);
  temp = analogRead(SupplyAD);

  for (index = 0; index <= 4; index++)       //sample AD 5 times
  {
    temp = analogRead(SupplyAD);
    volts = volts + temp;
  }
  volts = ((volts / 5) * ADMultiplier);

  //Serial.print(F("Voltage read "));
  //Serial.print(volts);
  //Serial.println(F("mV"));

  digitalWrite(BATVREADON, LOW);              //turn off MOSFET connection resistor divider in circuit

  index = (uint8_t) (((volts - 2500) + DiodemV) / 10);   //convert into units of 10mV above 2.5V

  return index;

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
  pinMode(SWITCH1, INPUT);                     //setup switch pin, to VCC to activate
  pinMode(SWITCH2, INPUT_PULLUP);

  pinMode(LED1, OUTPUT);
  led_Flash(2, 125);                           //two quick LED flashes to indicate program start

  Serial.begin(9600);
  Serial.println();
  Serial.println(F("5_Alarm_Transmitter Starting"));

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

  Serial.println(F("Transmitter ready"));
  Serial.println();
}
