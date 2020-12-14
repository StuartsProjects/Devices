/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 13/12/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This is a test program for using the LoRa device to transmit upper side band FSK
  RTTY. With the LoRa device in FSK direct mode, the frequency of the generated carrier is shifted up
  or down at the correct baud rate depending on whether a logic 0 or 1 is being sent. 

  The desired shift in frequency is defined in the Settings.h file as 'FrequencyShift'. When the program 
  starts the actual frequency shift will be calculated according to the discrete frequency steps the 
  LoRa device can be set to. There are settings for number of data bits, number of start bits and the 
  value of parity which can be ParityNone, ParityOdd, ParityEven, ParityZero or ParityOne. 

  Before the actual data transmission starts you can send a series of marker pips which are short bursts 
  of up shifted carrier which will be heard as beeps in a correctly tuned receiver. These pips can aid
  in setting the receiver decode frequemcy to match the transmission. on some LoRa devices, such as the SX127x
  series there can be considerable temperature induced frequency drift. This drift can be caused by outside
  temperature changes or the RF device self heating when transmit is turned on. The duration of the pips,
  the gaps between them and the period of leadin carrier before the data starts can all be set. To send no
  pips just set the number to 0.

  The FSK RTTY routines use the micros() function for timing, and a check is made at the begging of a 
  character to send to see if micros() might overflow during the transmission of the character. This check
  assumes the lowest baud rate of 45baud, and if an overflow is likley, there will be a short pause in transmission
  to allow the overflow to occur.     
   

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <SPI.h>                                 //the lora device is SPI based                                         
#include <SX127XLT.h>                            //get library here > https://github.com/StuartsProjects/SX12XX-LoRa   
#include "Settings.h"                            //include the setiings file, frequencies, LoRa settings etc   

SX127XLT LoRa;

//uint8_t testBuffer[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789";               
//uint8_t testBuffer[] = "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU";
uint8_t testBuffer[] = "$$MyFlight1,20,14:54:37,51.48230,-3.18136,15,6,3680,23,20,3,1500*A604";

uint8_t freqShiftRegs[3];                                      //to hold returned registers that set frequency


void loop()
{
  uint8_t index;
  
  printRegisterSetup(FrequencyShift);
  Serial.println();
  
  LoRa.startFSKRTTY(FrequencyShift, NumberofPips, PipPeriodmS, PipDelaymS, LeadinmS);

  if (micros() > 0xF8000000)
  {
  Serial.print(F("Waiting micros()"));
  while(micros() < 0xFFFB6000); 
  }
  
  Serial.print(F("Start RTTY micros() = "));
  Serial.println(micros(),HEX);
  LoRa.transmitFSKRTTY(13, DataBits, StopBits, Parity, BaudPerioduS, REDLED); //send carriage return
  LoRa.transmitFSKRTTY(10, DataBits, StopBits, Parity, BaudPerioduS, REDLED); //send line feed
  for (index = 0; index < (sizeof(testBuffer)-1); index++)
  {
    LoRa.transmitFSKRTTY(testBuffer[index], DataBits, StopBits, Parity, BaudPerioduS, REDLED);
    Serial.write(testBuffer[index]);
  }
  LoRa.transmitFSKRTTY(13, DataBits, StopBits, Parity, BaudPerioduS, REDLED); //send carriage return
  LoRa.transmitFSKRTTY(10, DataBits, StopBits, Parity, BaudPerioduS, REDLED); //send line feed
    
  Serial.println();
  Serial.print(F("END RTTY micros() = "));
  Serial.println(micros(),HEX);
  digitalWrite(REDLED, LOW);
  Serial.println();
  Serial.println();

  
  Serial.println(micros(),HEX);
  
  LoRa.setMode(MODE_STDBY_RC);

  delay(2000);
}


void printRegisterSetup(uint32_t shift)
{
  
  uint32_t nonShiftedFreq, ShiftedFreq;
  uint32_t freqShift;
  float exactfreqShift; 

  LoRa.setRfFrequency(Frequency, Offset);                       //ensure base frequecy is set
  LoRa.getRfFrequencyRegisters(freqShiftRegs);                  //fill buffer with frequency setting registers values
  nonShiftedFreq = ( (uint32_t) freqShiftRegs[0]  << 16 ) +  ( (uint32_t) freqShiftRegs[1] << 8 ) + freqShiftRegs[2];
  Serial.print(F("NoShift Registers 0x"));
  Serial.println(nonShiftedFreq, HEX);

  LoRa.setRfFrequency((Frequency + shift), Offset);             //set shifted frequecy
  LoRa.getRfFrequencyRegisters(freqShiftRegs);                  //fill buffer with frequency setting registers values
  ShiftedFreq = ( (uint32_t) freqShiftRegs[0]  << 16 ) +  ( (uint32_t) freqShiftRegs[1] << 8 ) + freqShiftRegs[2];
  Serial.print(F("Shifted Registers 0x"));
  Serial.println(ShiftedFreq, HEX);

  freqShift = ShiftedFreq - nonShiftedFreq;
  exactfreqShift = freqShift * FREQ_STEP;
  Serial.print(F("FSKRTTY register shift "));
  Serial.println(freqShift,HEX);
  Serial.print(F("FSKRTTY frequency shift "));
  Serial.print(exactfreqShift, 8);
  Serial.println(F("hZ"));

  LoRa.setRfFrequency(Frequency, Offset);                       //ensure base frequecy is set
}


void printRegisterBuffer()
{
Serial.print(freqShiftRegs[0],HEX);
Serial.print(F(" "));
Serial.print(freqShiftRegs[1],HEX);
Serial.print(F(" "));
Serial.print(freqShiftRegs[2],HEX);
Serial.println();  
}


void whiteFlash(uint16_t flashes, uint16_t ondelaymS, uint16_t offdelaymS)
{
  uint16_t index;
  
  pinMode(WHITELED, OUTPUT);                          //setup pin as output
  
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
  pinMode(REDLED, OUTPUT);                                   //setup pin as output for FSKRTTY indicator LED
  whiteFlash(2, 5, 195);                                     //two quick white LED flashes to indicate program start
  delay(1000);
  
  Serial.println();
  Serial.println();
  Serial.begin(115200);
  Serial.println(F("8_FSKRTTY_Transmitter_Test"));

  SPI.begin(SCK, MISO, MOSI, NSS);

  //setup hardware pins used by device, then check if device is found
  if (LoRa.begin(NSS, LORA_DEVICE))
  {
    Serial.println(F("LoRa Device found"));
    whiteFlash(2, 5, 195);                                   //two further quick LED flashes to indicate device found
    delay(1000);
  }
  else
  {
    Serial.println(F("No device responding"));
    while (1)
    {
      whiteFlash(50, 5, 195);                                //long fast speed LED flash indicates device error
    }
  }

  LoRa.setupDirect(Frequency, Offset);
  Serial.print(F("Transmitter ready"));
  Serial.println();
}

