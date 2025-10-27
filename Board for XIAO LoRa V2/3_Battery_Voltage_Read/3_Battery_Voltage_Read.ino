/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/02/23

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This test program has been written to check that hardware for reading the battery
  voltage. The value defined as 'ADMultiplier' is used to adjust the value read from the resistor divider
  and convert into mV.

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/


#define ADMultiplier 1.623                     //adjustment to convert AD value read into mV of battery voltage 
#define DiodemV 181                            //average forward voltage drop of diode  
#define LED1 13                                //pin for PCB LED  
#define SupplyAD A0                            //Resitor divider for battery connected here 


void loop()
{
  Serial.println(F("LED Flash"));
  led_Flash(4, 125);
  printSupplyVoltage();
  Serial.println();
  delay(1500);
}


void printSupplyVoltage()
{
  analogReadResolution(ADC_RESOLUTION);
  Serial.printf("Set the Analog to digital converter resolution to %d bits\n", ADC_RESOLUTION);

  pinMode(SupplyAD, INPUT);
  Serial.printf("Initialized pin A%d as an input.\n", SupplyAD);

  //get and display supply volts on terminal or monitor
  Serial.print(F("Supply Volts "));
  Serial.print(readSupplyVoltage());
  Serial.println(F("mV"));
}


uint16_t readSupplyVoltage()
{
  uint16_t temp;
  uint16_t volts = 0;
  byte index;

  temp = analogRead(SupplyAD);

  for (index = 0; index <= 4; index++)      //sample AD 5 times
  {
    temp = analogRead(SupplyAD);
    volts = volts + temp;
  }

  Serial.print(F("Average AD value "));
  Serial.println(volts / 5);
  
  volts = ((volts / 5) * ADMultiplier);

  return volts + DiodemV;
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, LOW);
    delay(delaymS);
    digitalWrite(LED1, HIGH);
    delay(delaymS);
  }
}


void setup()
{
  pinMode(LED1, OUTPUT);                      //for PCB LED
  led_Flash(2, 125);

  Serial.println();
  Serial.begin(115200);                       //setup Serial console ouput
  Serial.println();
  Serial.println("14_Battery_Voltage_Read Starting");
}
