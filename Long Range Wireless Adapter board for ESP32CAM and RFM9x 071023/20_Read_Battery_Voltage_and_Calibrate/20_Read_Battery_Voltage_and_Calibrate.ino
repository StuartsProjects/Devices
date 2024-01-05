/*******************************************************************************************************
  lora Programs for Arduino - Copyright of the author Stuart Robinson - 13/01/23

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This test program has been written to check that resistor divider for reading the
  battery voltage via a resistor divider connected to pin 13 of the ESP32CAM board is working correctly. 
  The value defined as 'ADMultiplier' in settings.h is used to adjust (calibrate) the value read from
  the 100K\47K resistor divider and convert into the battery mV. The typical voltage drop of the reverse
  protection diode in the battery circuit is accounted for. If the application for the ESP32CAM uses the
  SD card, be sure to have it inserted when deciding on the calibration value 'ADMultiplier'.

  To use this program to read the battery voltage you should remove the ESP32CAM SD card.  

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#define ADMultiplier 1.757                     //adjustment to convert AD value read into mV of battery voltage, for 100K and 47K resistor divider 
#define DiodeForwardmV 222                     //Forward voltage drop of battery series diode
#define SupplyAD 13                            //resistor divider analogue read for battery connected here 
#define NSS 12                                 //select pin on LoRa device


void loop()
{
  printBatteryVoltage();
  Serial.println();
  delay(2000);
}


void printBatteryVoltage()
{
  //get and display supply volts on terminal or monitor
  Serial.print(F("Supply Volts "));
  Serial.print(readSupplyVoltage());
  Serial.println(F("mV"));
}


uint16_t readSupplyVoltage()
{
  //relies on 3.3V as supply reference

  uint16_t temp;
  uint16_t batteryvolts = 0;
  byte index;

  temp = analogRead(SupplyAD);

  for (index = 0; index <= 19; index++)        //sample AD 20 times
  {
    temp = analogRead(SupplyAD);
    batteryvolts = batteryvolts + temp;
    delay(1);
  }
  batteryvolts = ((batteryvolts / 20) * ADMultiplier) + DiodeForwardmV;

  return batteryvolts;
}


void setup()
{
  Serial.begin(115200);                        //setup Serial console ouput
  Serial.println();
  Serial.println();
  Serial.println(__FILE__);
  Serial.println();
  pinMode(NSS, OUTPUT);
  digitalWrite(NSS, HIGH);                    //make sure LoRa device is not active
}
