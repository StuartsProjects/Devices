/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 15/11/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO SAMD21.

  Program Operation - This program is stand alone, it is not necessary to install the SX12XX-LoRa library
  to use it. This test program is for the SX127X LoRa devices.

  The program checks that a LoRa device connected to the Seeeduino XIAO can be accessed by doing a test
  register write and read. If there is no device found a message is printed on the serial monitor.

  The contents of the registers from 0x00 to 0x7F are printed and the program then changes the frequency
  between two values and prints out the registers. This is to prove that the device registers are being
  read and written correctly. There is a copy of the typical printout below. Note that the read back
  changed frequency may be slightly different to the programmed frequency, there is a rounding error due
  to the use of floats to calculate the frequency. Although the program sets the frequency in the 434Mhz
  band, it will work on 868Mhz and 915Mhz devices and there is no attempt to confugure the device for
  transmission or reception.

  The Arduino pin number that NSS on the LoRa device is connected to must be specified in #define NSS
  line below. Leave the NRESET and DIOx pins not connected.

  Typical printout;

  LoRa Device found
  Device version 0x12
  Frequency at Start 434000000
  Registers at Start
  Reg    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
  0x00  00 09 1A 0B 00 52 6C 80 00 4F 09 2B 20 08 02 0A
  0x10  FF 70 15 0B 28 0C 12 47 32 3E 00 00 00 00 00 40
  0x20  00 00 00 00 05 00 03 93 55 55 55 55 55 55 55 55
  0x30  90 40 40 00 00 0F 00 00 00 F5 20 82 FD 02 80 40
  0x40  00 00 12 24 2D 00 03 00 04 23 00 09 05 84 32 2B
  0x50  14 00 00 10 00 00 00 0F E0 00 0C FD 06 00 5C 78
  0x60  00 19 0C 4B CC 0D FD 20 04 47 AF 3F F6 3F DB 0B
  0x70  D0 01 10 00 00 00 00 00 00 00 00 00 00 00 00 00


  Change Frequency to 434100000
  Changed Frequency 434099968
  Reg    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
  0x00  00 09 1A 0B 00 52 6C 86 66 4F 09 2B 20 08 02 0A
  0x10  FF 70 15 0B 28 0C 12 47 32 3E 00 00 00 00 00 40
  0x20  00 00 00 00 05 00 03 93 55 55 55 55 55 55 55 55
  0x30  90 40 40 00 00 0F 00 00 00 F5 20 82 FD 02 80 40
  0x40  00 00 12 24 2D 00 03 00 04 23 00 09 05 84 32 2B
  0x50  14 00 00 10 00 00 00 0F E0 00 0C FD 06 00 5C 78
  0x60  00 19 0C 4B CC 0D FD 20 04 47 AF 3F F6 3F DB 0B
  0x70  D0 01 10 00 00 00 00 00 00 00 00 00 00 00 00 00

  Change Frequency to 434200000
  Changed Frequency 434199951
  Reg    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
  0x00  00 09 1A 0B 00 52 6C 8C CC 4F 09 2B 20 08 02 0A
  0x10  FF 70 15 0B 28 0C 12 47 32 3E 00 00 00 00 00 40
  0x20  00 00 00 00 05 00 03 93 55 55 55 55 55 55 55 55
  0x30  90 40 40 00 00 0F 00 00 00 F5 20 82 FD 02 80 40
  0x40  00 00 12 24 2D 00 03 00 04 23 00 09 05 84 32 2B
  0x50  14 00 00 10 00 00 00 0F E0 00 0C FD 06 00 5C 78
  0x60  00 19 0C 4B CC 0D FD 20 04 47 AF 3F F6 3F DB 0B
  0x70  D0 01 10 00 00 00 00 00 00 00 00 00 00 00 00 00

  Note: An SX1272 will report as version 0x22 and the frequency at power up is 915000000hz.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

const uint8_t REG_FRMSB = 0x06;                 //register number for setting and reading frequency, high byte
const uint8_t REG_FRMID = 0x07;                 //register number for setting and reading frequency, mid byte
const uint8_t REG_FRLSB = 0x08;                 //register number for setting and reading frequency, low byte
const uint8_t REG_VERSION = 0x42;               //register containg version number of device


//*********  Setup LoRa hardware pin definition here ! *********

#define NSS A3                                  //LoRa device select
#define LED1 13                                 //on board LED is yellow

//**************************************************************/


#include <SPI.h>

uint32_t frequency;


void loop()
{
  Serial.println();
  Serial.println();

  Serial.println(F("Change Frequency to 434100000"));
  setRfFrequency(434100000, 0);              //change the frequency at reset, in hertz
  frequency = getFreqInt();                  //read back the changed frequency
  Serial.print(F("Changed Frequency "));
  Serial.println(frequency);                 //print the changed frequency, did the write work (allow for rounding errors) ?
  printRegisters(0x00, 0x7F);                //show the registers after frequency change
  Serial.println();

  Serial.println(F("Change Frequency to 434200000"));
  setRfFrequency(434200000, 0);              //change the frequency, in hertz
  frequency = getFreqInt();                  //read back the changed frequency
  Serial.print(F("Changed Frequency "));
  Serial.println(frequency);                 //print the changed frequency, did the write work (allow for rounding errors) ?
  printRegisters(0x00, 0x7F);                //show the registers after frequency change
  Serial.println();

  delay(5000);
}


uint8_t readRegister(uint8_t address)
{
  uint8_t regdata;
  digitalWrite(NSS, LOW);                    //set NSS low
  SPI.transfer(address & 0x7F);              //mask address for read
  regdata = SPI.transfer(0);                 //read the byte
  digitalWrite(NSS, HIGH);                   //set NSS high
  return regdata;
}


void writeRegister(uint8_t address, uint8_t value)
{
  digitalWrite(NSS, LOW);                    //set NSS low
  SPI.transfer(address | 0x80);              //mask address for write
  SPI.transfer(value);                       //write the byte
  digitalWrite(NSS, HIGH);                   //set NSS high
}


uint32_t getFreqInt()
{
  //get the current set LoRa device frequency, return as long integer

  uint8_t Msb, Mid, Lsb;
  uint32_t freqreg;
  uint64_t freq;

  Msb = readRegister(REG_FRMSB);
  Mid = readRegister(REG_FRMID);
  Lsb = readRegister(REG_FRLSB);

  freqreg = Msb;
  freqreg = (freqreg << 8) + Mid;
  freqreg = (freqreg << 8) + Lsb;
  freq = freqreg * 6103515625LL;

  return (freq / 100000000L);
}


uint32_t getFreqInt2()
{
  //get the current set LoRa device frequency, return as long integer

  uint8_t Msb, Mid, Lsb;
  uint32_t uinttemp;
  float floattemp;
  Msb = readRegister(REG_FRMSB);
  Mid = readRegister(REG_FRMID);
  Lsb = readRegister(REG_FRLSB);
  floattemp = ((Msb * 0x10000ul) + (Mid * 0x100ul) + Lsb);
  floattemp = ((floattemp * 61.03515625) / 1000000ul);
  uinttemp = (uint32_t)(floattemp * 1000000);
  return uinttemp;
}


void printRegisters(uint16_t Start, uint16_t End)
{
  //prints the contents of lora device registers to serial monitor

  uint16_t Loopv1, Loopv2, RegData;

  Serial.print(F("Reg    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F"));
  Serial.println();

  for (Loopv1 = Start; Loopv1 <= End;)
  {
    Serial.print(F("0x"));
    if (Loopv1 < 0x10)
    {
      Serial.print(F("0"));
    }
    Serial.print((Loopv1), HEX);
    Serial.print(F("  "));
    for (Loopv2 = 0; Loopv2 <= 15; Loopv2++)
    {
      RegData = readRegister(Loopv1);
      if (RegData < 0x10)
      {
        Serial.print(F("0"));
      }
      Serial.print(RegData, HEX);
      Serial.print(F(" "));
      Loopv1++;
    }
    Serial.println();
  }
}


void setRfFrequency(uint64_t freq64, int32_t offset)
{
  freq64 = freq64 + offset;
  freq64 = ((uint64_t)freq64 << 19) / 32000000;
  writeRegister(REG_FRMSB, (uint8_t)(freq64 >> 16));
  writeRegister(REG_FRMID, (uint8_t)(freq64 >> 8));
  writeRegister(REG_FRLSB, (uint8_t)(freq64 >> 0));
}


bool begin(int8_t pinNSS)
{
  pinMode(pinNSS, OUTPUT);
  digitalWrite(pinNSS, HIGH);

  if (checkDevice())
  {
    return true;
  }

  return false;
}


bool checkDevice()
{
  //check there is a device out there, writes a register and reads back

  uint8_t Regdata1, Regdata2;
  Regdata1 = readRegister(REG_FRMID);               //low byte of frequency setting
  writeRegister(REG_FRMID, (Regdata1 + 1));
  Regdata2 = readRegister(REG_FRMID);               //read changed value back
  writeRegister(REG_FRMID, Regdata1);               //restore register to original value

  if (Regdata2 == (Regdata1 + 1))
  {
    return true;
  }
  else
  {
    return false;
  }
}

void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, LOW);                    //board LED on
    delay(delaymS);
    digitalWrite(LED1, HIGH);                   //board LED off
    delay(delaymS);
  }
}


void setup()
{
  pinMode(LED1, OUTPUT);                         //setup board LED pin as output
  digitalWrite(LED1, HIGH);                      //LED off
  led_Flash(2, 125);                             //2 LED flashes to indicate program start
  delay(2000);

  Serial.begin(115200);
  Serial.println();
  Serial.println(F(__FILE__));
  Serial.println();

  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  //The begin function setups the hardware pins used by the LoRa device and then checks if device is found

  while (!begin(NSS))
  {
    Serial.println(F("ERROR - No LoRa device responding"));
    led_Flash(10, 25);                            //10 fast LED flashes to indicate LoRa device not responding
  }

  Serial.println(F("LoRa device is responding"));
  led_Flash(2, 125);                              //2 LED flashes to indicate LoRa device is responding

  Serial.print(F("Device version 0x"));

  uint8_t deviceversion = readRegister(REG_VERSION);
  if (deviceversion < 0x10)
  {
    Serial.print(F("0"));
  }
  Serial.println(deviceversion, HEX);

  frequency = getFreqInt();                   //read the set frequency following a reset
  Serial.print(F("Frequency at Start "));
  Serial.println(frequency);

  Serial.println(F("Registers at Start "));   //show the all registers following a power up
  printRegisters(0x00, 0x7F);
}
