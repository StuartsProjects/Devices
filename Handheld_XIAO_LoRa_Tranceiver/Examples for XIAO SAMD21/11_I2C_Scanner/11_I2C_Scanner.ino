/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO SAMD21.
  
  Program Operation - This scans the I2C bus from address 1 to address 126 and reports if an I2C device
  is found.

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <Wire.h>


void loop()
{
  uint8_t I2Cstatus, I2Caddress;
  int16_t I2Cdevicesfound;

  Serial.println("Checking");

  I2Cdevicesfound = 0;

  for (I2Caddress = 1; I2Caddress < 127; I2Caddress++ )
  {
    Wire.beginTransmission(I2Caddress);
    I2Cstatus = Wire.endTransmission();

    if (I2Cstatus == 0)
    {
      Serial.print("I2C device at address 0x");
      if (I2Caddress < 16)
      {
        Serial.print("0");
      }
      Serial.println(I2Caddress, HEX);
      I2Cdevicesfound++;
    }
    else if (I2Cstatus == 4)
    {
      Serial.print("ERROR at I2Caddress 0x");
      if (I2Caddress < 16)
      {
        Serial.print("0");
      }
      Serial.println(I2Caddress, HEX);
    }
  }

  Serial.print("I2C devices found ");
  Serial.println(I2Cdevicesfound);
  Serial.println();
  delay(5000);
}


void setup()
{
  Wire.begin();
  Serial.begin(115200);
  Serial.println("11_I2C_Scanner starting");
  Serial.println();
}
