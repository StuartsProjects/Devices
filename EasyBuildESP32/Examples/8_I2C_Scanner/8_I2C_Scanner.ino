// --------------------------------------
// i2c_scanner
// from: https://playground.arduino.cc/Main/I2cScanner/

// This sketch tests the standard 7-bit addresses
// Devices with higher bit address might not be seen properly.


#include <Wire.h>
uint16_t counter;

#define SCL 21                               //this is the default pin for Wire on a NodeMCU, but it can be changed
#define SDA 22


void setup()
{
  Wire.begin(SCL, SDA);

  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.println();
  Serial.println(__TIME__);
  Serial.println();
  Serial.println(__DATE__);
  Serial.println();
}


void loop()
{
  uint8_t error, address;
  int16_t nDevices;

  counter++;
  Serial.print(counter);
  Serial.println(F(" Scanning..."));

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print(F("I2C device found at address 0x"));
      if (address < 16)
        Serial.print(F("0"));
      Serial.println(address, HEX);
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print(F("Unknown error at address 0x"));
      if (address < 16)
        Serial.print(F("0"));
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0)
  {
    Serial.println(F("No I2C devices found"));
  }
  else
  {
    Serial.println();
    Serial.println(F("Done"));
    Serial.println();
  }

  delay(5000);           // wait 5 seconds for next scan
}
