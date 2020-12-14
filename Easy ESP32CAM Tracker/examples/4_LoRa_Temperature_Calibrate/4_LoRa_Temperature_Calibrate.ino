/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 12/12/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This program reads the internal temperature sensor in the SX127X range of devices.
  The temeprature sensor definetly needs calibrating, so run a test, check what the error is which could
  be +\- 10C or even more and calculate the value for the temperature_compensate constant. This constant
  will be different for each individual device.

  Sample Serial Monitor output;

  4_LoRa_Temperature_Calibrate for ESP32CAM Starting
  LoRa Device found
  Temperature Sensor Ready

  Temperature Register Raw 14
  Temperature Compensated 14c


  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/


//These are the pin definitions for the Easy ESP32CAM boards

#define NSS 13                              //lora device select
#define SCK 14
#define MISO 2
#define MOSI 15
#define LORA_DEVICE DEVICE_SX1278           //this is the device we are using 

#define WHITELED 4                          //pin number for ESP32CAM on board white LED, set logic level high for on

#include <SPI.h>
#include <SX127XLT.h>                       //get library here > https://github.com/StuartsProjects/SX12XX-LoRa  

SX127XLT LoRa;

const int8_t temperature_compensate = 0;   //value, degrees centigrade, to add to read temperature for calibration. Can be negative
//this compensate value will be different for each LoRa device instance, so best to
//label and record values for each device


void loop()
{
  int8_t temperature_register;
  LoRa.resetDevice();

  temperature_register = LoRa.getDeviceTemperature();

  Serial.print(F("Temperature Register Raw "));
  Serial.println(temperature_register);
  Serial.print(F("Temperature Compensated "));
  Serial.print(temperature_register + temperature_compensate);
  Serial.println(F("C"));
  Serial.println();
  delay(2000);
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
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println(F("4_LoRa_Temperature_Calibrate for Easy ESP32CAM Starting"));
  Serial.println();
  whiteFlash(5, 5, 195);

  SPI.begin(SCK, MISO, MOSI, NSS);

  Serial.println();

  //setup hardware pins used by device, then check if device is found
  if (LoRa.begin(NSS, LORA_DEVICE))
  {
    Serial.println(F("LoRa Device found"));
  }
  else
  {
    Serial.println(F("No device responding"));
    while (1);
  }

  Serial.println(F("Temperature Sensor Ready"));
  Serial.println();
}

