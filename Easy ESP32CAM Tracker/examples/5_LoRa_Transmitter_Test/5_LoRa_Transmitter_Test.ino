/*******************************************************************************************************
  lora Programs for Arduino - Copyright of the author Stuart Robinson - 12/12/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This is a minimum setup LoRa test transmitter. First an FM tone is transmitted,
  this can be heard on a UHF handheld receiver or low cost software defined radio (SDR). The pins to
  access the lora  device need to be defined at the top of the program also.

  A packet containing the ASCII text from one of the buff[] definitions below is then as LoRa using the
  frequency and LoRa settings specified in the LoRa.setupLoRa() command. This uses medium range settings.

  Next a short LoRa packet containing latitude, longitude and altitude is sent using long range settings
  is sent.

  The details of the packet sent and any errors are shown on the Arduino IDE Serial Monitor. The matching
  receiver program, '9_Balloon_Tracker_Receiver' can be used to check the packets are being sent
  correctly, the frequency and LoRa settings (in the LoRa.setupLoRa() commands) must be the same for the
  transmitter and receiver programs.

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <SPI.h>                                //the lora device is SPI based so load the SPI library                                         
#include <SX127XLT.h>                           //get library here > https://github.com/StuartsProjects/SX12XX-LoRa    
#include "Settings.h"

SX127XLT LoRa;                                  //create a library class instance called LoRa
#include <ProgramLT_Definitions.h>


uint8_t TXPacketL;
uint32_t TXPacketCount;

//uint8_t buff[] = "Hello World 1234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ";      //a message to send
uint8_t buff[] = "$MyFlight1,20,14:54:37,51.48230,-3.18136,15,6,3680,23,20,3,1500*A604";      //a message to send


void loop()
{
  whiteFlash(5, 1, 199);

  delay(1000);

  Serial.print(ToneTXpower);                      //print the tone transmit power defined
  Serial.print(F("dBm "));
  Serial.print(F("FM Tone"));
  Serial.flush();
  LoRa.setupDirect(TrackerFrequency, Offset);     //setup for direct FM tones
  digitalWrite(REDLED, LOW);
  LoRa.toneFM(1000, 1000, 10000, 1, ToneTXpower); //Transmit an FM tone, 1000hz, 1000ms, deviation 10000hz, tone freq adjust 1
  digitalWrite(REDLED, HIGH);
  Serial.println(F(" Done"));

  delay(1000);

  //Send long HAB format packet - medium range settings
  Serial.print(TrackerTXpower);                  //print the transmit power defined
  Serial.print(F("dBm "));
  Serial.print(F("HABPacket> "));
  TXPacketL = sizeof(buff) - 1;                  //set TXPacketL to length of buffer, omitting null character at end.
  LoRa.printASCIIPacket(buff, TXPacketL);        //print the buffer (the sent packet) as ASCII
  Serial.flush();
  LoRa.setupLoRa(TrackerFrequency , Offset, TrackerSpreadingFactor, TrackerBandwidth, TrackerCodeRate, TrackerOptimisation); //we have used toneFM() so setup for LoRa again

  digitalWrite(REDLED, LOW);
  TXPacketL = LoRa.transmit(buff, TXPacketL, 10000, TrackerTXpower, NO_WAIT);
  digitalWrite(REDLED, HIGH);
  waitIRQTXDone(5000);                           //wait for IRQ_TX_DONE, timeout of 5000mS
  Serial.println(F(" Sent"));

  delay(1000);

  //send location only packet - long range settings
  Serial.print(SearchTXpower);                                       //print the transmit power defined
  Serial.print(F("dBm "));
  Serial.print(F("LocationPacket> "));
  Serial.print(F("51.48230,-3.18136,15,3"));
  Serial.flush();
  TXPacketL = buildLocationOnly(51.48230, -3.18136, 15, 3);          //put location data in SX12xx buffer
  LoRa.setupLoRa(SearchFrequency, Offset, SearchSpreadingFactor, SearchBandwidth, SearchCodeRate, SearchOptimisation);   //we have used toneFM() so setup for LoRa again
  digitalWrite(REDLED, LOW);
  TXPacketL = LoRa.transmitSXBuffer(0, TXPacketL, 10000, SearchTXpower, NO_WAIT);
  digitalWrite(REDLED, HIGH);
  waitIRQTXDone(5000);                                               //wait for IRQ_TX_DONE, timeout of 5000mS
  Serial.println(F(" Sent"));
  Serial.print(F("SXBuffer > "));
  LoRa.printSXBufferHEX(0, TXPacketL - 1);
  Serial.print(F(" End"));
  Serial.println();
  delay(1000);                                                       //have a delay between packets
}


uint16_t waitIRQTXDone(uint32_t txtimeout)
{
  //there is no DIO0 pin to read on the Easy ESP32CAM so the LoRa device IRQ register is read direct

  uint32_t startmS, endtimeoutmS;

  startmS = millis();
  endtimeoutmS = (millis() + txtimeout);

  while (! (LoRa.readIrqStatus() & IRQ_TX_DONE) && (millis() < endtimeoutmS));

  if ((LoRa.readIrqStatus() & IRQ_TX_DONE))
  {
    Serial.print(F(" IRQ_TX_DONE "));
    Serial.print(millis() - startmS);
    Serial.print(F("mS"));
    return IRQ_TX_DONE;
  }
  else
  {
    Serial.print(F(" IRQ_TX_TIMEOUT "));
    return IRQ_TX_TIMEOUT;
  }
}


uint8_t buildLocationOnly(float Lat, float Lon, uint16_t Alt, uint8_t stat)
{
  uint8_t len;
  LoRa.startWriteSXBuffer(0);                   //initialise buffer write at address 0
  LoRa.writeUint8(LocationBinaryPacket);        //identify type of packet
  LoRa.writeUint8(Broadcast);                   //who is the packet sent too
  LoRa.writeUint8(ThisNode);                    //tells receiver where is packet from
  LoRa.writeFloat(Lat);                         //add latitude
  LoRa.writeFloat(Lon);                         //add longitude
  LoRa.writeInt16(Alt);                         //add altitude
  LoRa.writeUint8(stat);                        //add tracker status
  len = LoRa.endWriteSXBuffer();                //close buffer write
  return len;
}


void packet_is_OK()
{
  //if here packet has been sent OK
  Serial.print(F("  BytesSent,"));
  Serial.print(TXPacketL);                      //print transmitted packet length
  Serial.print(F("  PacketsSent,"));
  Serial.print(TXPacketCount);                  //print total of packets sent OK
}


void packet_is_Error()
{
  //if here there was an error transmitting packet
  uint16_t IRQStatus;
  IRQStatus = LoRa.readIrqStatus();             //read the the interrupt register
  Serial.print(F(" SendError,"));
  Serial.print(F("Length,"));
  Serial.print(TXPacketL);                      //print transmitted packet length
  Serial.print(F(",IRQreg,"));
  Serial.print(IRQStatus, HEX);                 //print IRQ status
  LoRa.printIrqStatus();                        //prints the text of which IRQs set
}


void whiteFlash(uint16_t flashes, uint16_t ondelaymS, uint16_t offdelaymS)
{
  uint16_t index;

  pinMode(WHITELED, OUTPUT);                     //setup pin as output

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(WHITELED, HIGH);
    delay(ondelaymS);
    digitalWrite(WHITELED, LOW);
    delay(offdelaymS);
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println(F("5_LoRa_Transmitter_Test for Easy ESP32CAM Starting"));

  SPI.begin(SCK, MISO, MOSI, NSS);

  if (LoRa.begin(NSS, LORA_DEVICE))
  {
    Serial.println(F("LoRa Device found"));
    delay(1000);
  }
  else
  {
    Serial.println(F("No device responding"));
    while (1);
  }

  LoRa.setupLoRa(TrackerFrequency, Offset, TrackerSpreadingFactor, TrackerBandwidth, TrackerCodeRate, TrackerOptimisation); //configure frequency and LoRa settings

  Serial.println();
  LoRa.printModemSettings();                                     //reads and prints the configured LoRa settings, useful check
  Serial.println();
  LoRa.printOperatingSettings();                                 //reads and prints the configured operting settings, useful check
  Serial.println();
  Serial.println();

  Serial.print(F("Transmitter ready"));
  Serial.println();
  Serial.println();
  pinMode(REDLED, OUTPUT);                                       //ESP32CAM red led turned on during transmit
}

