/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This scans the I2C bus from address 1 to address 126 and reports if an I2C device
  is found. Common device addresses;

  0x20 PCF8574

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <Wire.h>
#define LED1 13                         //on board LED is yellow

void loop()
{
  uint8_t I2Cstatus, I2Caddress;
  int16_t I2Cdevicesfound;

  SerialUSB.println("Scanning for I2C devices");

  I2Cdevicesfound = 0;

  for (I2Caddress = 1; I2Caddress < 127; I2Caddress++ )
  {
    Wire.beginTransmission(I2Caddress);
    I2Cstatus = Wire.endTransmission();

    if (I2Cstatus == 0)
    {
      SerialUSB.print("I2C device at address 0x");
      if (I2Caddress < 16)
      {
        SerialUSB.print("0");
      }
      SerialUSB.println(I2Caddress, HEX);
      I2Cdevicesfound++;
    }
    else if (I2Cstatus == 4)
    {
      SerialUSB.print("ERROR at I2Caddress 0x");
      if (I2Caddress < 16)
      {
        SerialUSB.print("0");
      }
      SerialUSB.println(I2Caddress, HEX);
    }
  }

  SerialUSB.print("I2C devices found ");
  SerialUSB.println(I2Cdevicesfound);
  SerialUSB.println();
  delay(5000);
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, LOW);                    //board LED on
    delay(delaymS);
    digitalWrite(LED1, HIGH);                     //board LED off
    delay(delaymS);
  }
}


void setup()
{
  pinMode(LED1, OUTPUT);                          //setup board LED pin as output
  led_Flash(10, 500);                             //10 LED flashes to indicate program start
  
  digitalWrite(LED1, LOW);                        //LED on
  
  Wire.begin();
  SerialUSB.begin(115200);
  SerialUSB.println("11_I2C_Scanner starting");
  SerialUSB.println();
}
