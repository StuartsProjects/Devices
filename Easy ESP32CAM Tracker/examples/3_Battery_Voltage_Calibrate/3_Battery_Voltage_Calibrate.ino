/*******************************************************************************************************
  lora Programs for Arduino - Copyright of the author Stuart Robinson - 13/12/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This test program has been written to check that resistor divider for reading the
  battery voltage on pin 14 of the Easy ESPCAM32 board has been assembled correctly. The value defined
  as 'ADMultiplier' in settings.h is used to adjust (calibrate) the value read from the 100K\47K resistor
  divider and convert into the battery mV.

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#define ADMultiplier 2.9                       //adjustment to convert AD value read into mV of battery voltage 
#define SupplyAD 14                            //resitor divider analogue read for battery connected here 
#define NSS 13                                 //select pin on LoRa device


void loop()
{
  printSupplyVoltage();
  Serial.println();
  delay(2000);
}


void printSupplyVoltage()
{
  //get and display supply volts on terminal or monitor
  Serial.print(F("Supply Volts "));
  Serial.print(readSupplyVoltage());
  Serial.println(F("mV"));
}


uint16_t readSupplyVoltage()
{
  //relies on 3.3V supply reference

  uint16_t temp;
  uint16_t volts = 0;
  byte index;

  temp = analogRead(SupplyAD);

  for (index = 0; index <= 19; index++)      //sample AD 20 times
  {
    temp = analogRead(SupplyAD);
    volts = volts + temp;
    delay(1);
  }
  volts = ((volts / 20) * ADMultiplier);

  return volts;
}


void setup()
{
  Serial.begin(115200);                       //setup Serial console ouput
  Serial.println();
  Serial.println();
  Serial.println("3_Battery_Voltage_Calibrate for Easy ESP32CAM Starting");
  Serial.println();
  pinMode(NSS, OUTPUT);
  digitalWrite(NSS, HIGH);                    //make sure LoRa device is not active
}

