/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/02/23

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This test program has been written to check that hardware for reading the battery
  voltage. The value defined as 'ADMultiplier' is used to adjust the value read from the resistor divider
  and convert into mV.

  The serial prints are sent to the Serial port defined by MonitorPort. This can be Serial, the default
  which is the USB port or to Serial1 on pins A6 and A7. 

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/


const float ADMultiplier = 1.623;                     //adjustment to convert AD value read into mV of battery voltage 
const float DiodeV = 0.181;                            //average forward voltage drop of diode  
const uint8_t LED1 = 13;                              //pin for PCB LED  
const uint8_t SupplyAD = A0;                          //Resitor divider for battery connected here 

#define MonitorPort Serial1                           //can be changed to output serial prints on Serial or Serial1 

void loop()
{
  MonitorPort.println(F("LED Flash"));
  led_Flash(4, 125);
  printSupplyVoltage();
  MonitorPort.println();
  delay(1500);
}


void printSupplyVoltage()
{
  float supplyvolts = readSupplyVoltage();
  MonitorPort.print(F("Supply Volts "));
  MonitorPort.print(supplyvolts, 2);
  MonitorPort.println(F("V"));
}


float readSupplyVoltage()
{
  uint16_t temp = 0;
  float volts = 0;
  byte index;

  analogReadResolution(ADC_RESOLUTION);
  MonitorPort.printf("Set the Analog to digital converter resolution to %d bits\r\n", ADC_RESOLUTION);
  
  pinMode(SupplyAD, INPUT);
  MonitorPort.printf("Initialized pin A%d as an input.\r\n", SupplyAD);

  temp = analogRead(SupplyAD);

  for (index = 0; index <= 4; index++)      //sample AD 5 times
  {
    temp = analogRead(SupplyAD);
    volts = volts + temp;
  }

  MonitorPort.print(F("Average AD value "));
  MonitorPort.println((volts / 5),0);
  
  volts = ((volts / 5000) * ADMultiplier);

  return volts + DiodeV;
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

  MonitorPort.println();
  MonitorPort.begin(115200);                       //setup Serial console ouput
  MonitorPort.println();
  MonitorPort.println("14_Battery_Voltage_Read Starting");
}
