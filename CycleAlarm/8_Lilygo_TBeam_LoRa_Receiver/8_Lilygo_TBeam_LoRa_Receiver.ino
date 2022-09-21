/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 20/09/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a LoRa receiver thats used as an extender for a cycle motion\disturb alarm.
  The program is for a Lilygo TBEAM V1.1, this is a ready built portable and battery powered LoRa reciever
  with display. A 5V buzzer (which works on 3.3v) is wired to pin 15.

  The LoRa transmitter fitted to the alarm sends a reliable packet from the SX127XLT library, this packet
  has 4 bytes automatically appended to the end of the buffer\array that is the data payload. The first
  two bytes appended are a 16bit 'NetworkID'. The receiver needs to have the same NetworkID as configured
  for the transmitter since the receiver program uses the NetworkID to check that the received packet is
  from a known source. The third and fourth bytes appended are a 16 bit CRC of the payload. The receiver
  will carry out its own CRC check on the received payload and can then verify this against the CRC
  appended in the packet. The receiver is thus able to check if the payload is valid. This gives a gives
  a high level of assurance that the packet is valid.

  If the received packet is valid then the networkID and payload CRC are returned in a 4 byte packet as an
  acknowledgement that the transmitter listens for. If the transmitter does not receive the acknowledgement
  within the ACKtimeout period, the original packet is re-transmitted until a valid acknowledgement is
  received.

  The receiver will flash an LED briefly whilst its listening so you can check that the receiver is working.
  The transmitter, see program '5_Alarm_Transmitter', sends two types of packet, and alarm packet which will
  cause the receiver to sound the buzzer and an optional beacon packet which can be used to monitor the
  effectiveness of the communications from transmitter to receiver.

  Serial monitor baud rate should be set at 9600.
*******************************************************************************************************/

#include <SPI.h>                                //the LoRa device is SPI based so load the SPI library
#include <SX127XLT.h>                           //include the appropriate library   

SX127XLT LoRa;                                  //create a library class instance called LoRa


#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NSS 18                                  //select pin on LoRa device
#define NRESET 23                               //reset pin on LoRa device
#define DIO0 26                                 //DIO0 pin on LoRa device, used for RX and TX done 
#define LORA_DEVICE DEVICE_SX1278               //we need to define the device we are using
#define LED1 4                                  //on board LED
#define BUZZ 15                                 //external buzzer 

#define ACKdelay 0                              //delay in mS before sending acknowledge, not used here so set to 0                      
#define RXtimeout 60000                         //receive timeout in mS.  
#define TXpower 10                              //dBm power to use for ACK   


uint8_t RXPacketL;                              //stores length of packet received
uint8_t RXPayloadL;                             //stores length of payload received
uint8_t PacketOK;                               //set to > 0 if packetOK
int16_t PacketRSSI;                             //stores RSSI of received packet
int8_t PacketSNR;                               //stores SNR of received packet
uint16_t LocalPayloadCRC;                       //locally calculated CRC of payload
uint16_t RXPayloadCRC;                          //CRC of payload received in packet
uint16_t TransmitterNetworkID;                  //the NetworkID from the transmitted and received packet
uint8_t BatteryVolts;

const uint16_t NetworkID = 0x3210;              //NetworkID identifies this connection, needs to match value in transmitter

uint16_t RXPacketType;                          //packet type received
uint8_t NodeNumber = '1';                       //output number on node we are controlling
uint8_t ReliableErrors = 0;
uint8_t ReliableFlags = 0;
uint16_t IRQmsb;
uint16_t AlarmCount;
uint32_t BeaconCount;


void loop()
{
  Serial.println(F("Listening"));
  PacketOK = receiveSXReliable(0, NetworkID, RXtimeout); //wait for a packet to arrive with 120seconds (120000mS) timeout

  if (PacketOK)
  {
    RXPacketL = LoRa.readRXPacketL();             //get the received packet length
    Serial.println(F("Packet OK"));
    Serial.print(F("Length "));
    Serial.println(RXPacketL);
  }

  if (IRQmsb != IRQ_RX_TIMEOUT)
  {
    Serial.println(F("Read packet"));
    RXPayloadL = RXPacketL - 4;
    PacketRSSI = LoRa.readPacketRSSI();           //read the received packets RSSI value

    LoRa.startReadSXBuffer(0);                    //start buffer read at location 0
    NodeNumber = LoRa.readUint8();                //load the node number
    if (RXPacketL == 6)
    {
      BatteryVolts = LoRa.readUint8();
    }
    RXPayloadL = LoRa.endReadSXBuffer();          //this function returns the length of the array read

    if (PacketOK > 0)
    {
      Serial.print(F("NodeNumber "));
      Serial.println(NodeNumber);

      if (RXPacketL == 6)
      {
        Serial.println(F("Alarm packet"));
        Serial.print(F("Battery "));
        Serial.print((BatteryVolts * 10) + 2500);
        Serial.println(F("mV"));
        AlarmCount++;
        ActionAlarm();
      }

      if (RXPacketL == 5)
      {
        Serial.println(F("Beacon packet"));
        BeaconCount++;
        ActionBeacon();
      }

      if ((RXPacketL != 6) && (RXPacketL != 5))
      {
        Serial.println(F("ERROR - Unknown packet"));
      }

    }
    else
    {
      //if the LoRa.receiveReliable() function detects an error PacketOK is 0
      packet_is_Error();
    }

  }

  Serial.println();
}


void ActionBeacon()
{
  led_Flash(5, 25);                         //5 quick LED flashes to indicate beacon
  digitalWrite(BUZZ, HIGH);
  delay(25);
  digitalWrite(BUZZ, LOW);
  screen1();
}


void ActionAlarm()
{
  uint8_t index;

  digitalWrite(LED1, LOW);                  //LED on
  digitalWrite(BUZZ, HIGH);                 //External Buzzer on
  sendACK();
  display.clearDisplay();  // Clear the buffer
  fillcircle();                             // Draw circles (filled)
  for (index = 0; index <= 9; index++)
  {
    display.invertDisplay(true);
    delay(10);
    display.invertDisplay(false);
    delay(10);
  }
  digitalWrite(LED1, HIGH);                 //LED off
  digitalWrite(BUZZ, LOW);                  //External Buzzer off
  screen1();
}


void fillcircle()
{

  for (int16_t i = max(display.width(), display.height()) / 2; i > 0; i -= 3)
  {
    // The INVERSE color is used so circles alternate white/black
    display.fillCircle(display.width() / 2, display.height() / 2, i, SSD1306_INVERSE);
    display.display(); // Update screen with each newly-drawn circle
    delay(1);
  }
}


void screen1()
{
  display.clearDisplay();
  display.setTextSize(1);                   //Normal 1:1 pixel scale
  display.setCursor(0, 0);                  //Start at top-left corner
  display.print(AlarmCount);
  display.print(F(" / "));
  display.print(BeaconCount);
  display.setCursor(0, 8);                  //next line
  display.print(NodeNumber);
  display.print(F("  "));
  display.print((BatteryVolts * 10) + 2500);
  display.print(F("mV"));

  PacketRSSI = LoRa.readPacketRSSI();       //read the received packets RSSI value
  PacketSNR = LoRa.readPacketSNR();         //read the received packets SNR value

  display.setCursor(0, 16);                 //next line
  display.print(F("RSSI "));
  display.print(PacketRSSI);
  display.print(F("dBm"));
  display.setCursor(0, 24);                  //next line
  display.print(F("SNR "));

  if (PacketSNR > 0)
  {
    display.print(F("+"));
  }

  display.print(PacketSNR);
  display.print(F("dB"));
  display.display();
}


void screen2()
{
  display.clearDisplay();
  display.setTextSize(2);                     //Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        //Draw white text
  display.setCursor(0, 8);
  display.println(F("Ready"));
  display.display();
}


uint8_t receiveSXReliable(uint8_t startaddr, uint16_t networkID, uint32_t rxtimeout )
{
  uint32_t starttimemS;
  uint16_t payloadcrc = 0, RXcrc, RXnetworkID = 0;
  uint32_t startmS;

  IRQmsb = 0;
  ReliableErrors = 0;
  ReliableFlags = 0;
  LoRa.setMode(MODE_STDBY_RC);
  LoRa.writeRegister(REG_FIFORXBASEADDR, startaddr);              //set start address of RX packet in buffer
  LoRa.setDioIrqParams(IRQ_RADIO_ALL, IRQ_RX_DONE, 0, 0);         //set for IRQ on RX done
  LoRa.setRx(0);                                                  //no actual RX timeout in this function

  startmS = millis();
  starttimemS = millis();
  do
  {
    if (millis() > (starttimemS + 2000))
    {
      starttimemS = millis();
      led_Flash(1, 50);                                           //quick LED flash to show still listening
    }
  }
  while   (!digitalRead(DIO0) && ( (uint32_t) (millis() - startmS) < rxtimeout) );

  LoRa.setMode(MODE_STDBY_RC);                                    //ensure to stop further packet reception

  if (!digitalRead(DIO0))                                         //check if DIO still low, is so must be RX timeout
  {
    Serial.print(F("RX Timeout"));
    IRQmsb = IRQ_RX_TIMEOUT;
    return 0;
  }

  if ( LoRa.readIrqStatus() != (IRQ_RX_DONE + IRQ_HEADER_VALID) )
  {
    return 0;                                                      //no RX done and header valid only, could be CRC error
  }

  RXPacketL = LoRa.readRegister(REG_RXNBBYTES);

  if (RXPacketL < 4)                                               //check received packet is 4 or more bytes long
  {
    bitSet(ReliableErrors, ReliableSizeError);
    return 0;
  }

  RXnetworkID = LoRa.readUint16SXBuffer(startaddr + RXPacketL - 4);

  if (RXnetworkID != networkID)
  {
    bitSet(ReliableErrors, ReliableIDError);
  }

  payloadcrc = LoRa.CRCCCITTReliable(startaddr, (startaddr + RXPacketL - 5), 0xFFFF);
  RXcrc = LoRa.readUint16SXBuffer(startaddr + RXPacketL - 2);

  if (payloadcrc != RXcrc)
  {
    bitSet(ReliableErrors, ReliableCRCError);
  }

  if (ReliableErrors)                                       //if there has been a reliable error return a RX fail
  {
    return 0;
  }
  return RXPacketL;                                         //return and RX OK.
}


void packet_is_OK()
{
  printPacketDetails();
  Serial.println();
}


void packet_is_Error()
{
  uint16_t IRQStatus;

  IRQStatus = LoRa.readIrqStatus();                          //read the LoRa device IRQ status register
  Serial.print(F("Error "));
  if (IRQStatus & IRQ_RX_TIMEOUT)                            //check for an RX timeout
  {
    Serial.print(F(" RXTimeout "));
  }
  else
  {
    printPacketDetails();
  }
}


void printPacketDetails()
{
  LocalPayloadCRC = LoRa.CRCCCITTReliable(0, RXPayloadL - 1, 0xFFFF);    //calculate payload crc
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


void sendACK()
{
  LocalPayloadCRC = LoRa.CRCCCITTReliable(0, RXPayloadL - 1, 0xFFFF);    //calculate payload crc
  delay(ACKdelay);
  LoRa.sendReliableACK(NetworkID, LocalPayloadCRC, TXpower);
  Serial.println(F("Ack sent"));
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  //general purpose routine for flashing LED as indicator
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, LOW);                    //LED on
    delay(delaymS);
    digitalWrite(LED1, HIGH);                   //LED off
    delay(delaymS);
  }
}




void setup()
{
  pinMode(LED1, OUTPUT);
  pinMode(BUZZ, OUTPUT);
  digitalWrite(BUZZ, LOW);
  led_Flash(2, 125);                           //two quick LED flashes to indicate program start

  Serial.begin(9600);
  Serial.println();
  Serial.println(__FILE__);

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

  LoRa.setupLoRa(434000000, 0, LORA_SF12, LORA_BW_062, LORA_CR_4_5, LDRO_AUTO);   //configure frequency and LoRa settings

  Serial.println(F("Receiver ready"));
  Serial.println();


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.cp437(true);                    // Use full 256 char 'Code Page 437' font
  display.setTextColor(SSD1306_WHITE);    // Draw white text

  screen2();
}
