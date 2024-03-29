/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 13/09/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This test program has been written to check that hardware for reading the battery
  voltage on the micro controller baord is funtional and calibrated. The value defined as 'ADMultiplier'
  is used to adjust the value read from the 100K\12K resistor divider and convert into mV. The reading
  of the voltage uses the ATMega328P internal voltage reference, which although stable the voltage is
  not very accurate, hence the need for calibration.

  Measure the actual input voltage and adjust ADMultiplier so the value printed on the serial monitor is
  accurate.

  Serial monitor baud rate is set at 9600
*******************************************************************************************************/

#define ADMultiplier 10.18                     //adjustment to convert AD value read into mV of battery voltage 
#define DiodemV 180                            //typical voltage drop across diode in battery lead
#define BATVREADON 8                           //used to turn on the resistor divider to measure voltage,                              //this pin turns on the MOSFET that switches in the resistor divider
#define LED1 8                                 //pin for PCB LED  
#define SupplyAD A7                            //centre of resistor divider for battery connected here 


void loop()
{
  Serial.println(F("LED Flash"));
  led_Flash(4, 125);
  printSupplyVoltage();
  Serial.println();
  delay(1500);
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, HIGH);
    delay(delaymS);
    digitalWrite(LED1, LOW);
    delay(delaymS);
  }
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
  //relies on internal reference and 91K & 11K resistor divider
  //returns supply in mV @ 10mV per AD bit read
  uint16_t temp;
  uint16_t volts = 0;
  byte index;

  digitalWrite(BATVREADON, HIGH);           //turn MOSFET connection resitor divider in circuit

  analogReference(INTERNAL);
  temp = analogRead(SupplyAD);

  for (index = 0; index <= 4; index++)      //sample AD 5 times
  {
    temp = analogRead(SupplyAD);
    volts = volts + temp;
  }
  volts = ((volts / 5) * ADMultiplier);

  digitalWrite(BATVREADON, LOW);            //turn MOSFET connection resitor divider in circuit

  return (volts + DiodemV);
}


void setup()
{
  Serial.begin(9600);                       //setup Serial console ouput
  Serial.println();
  Serial.println(__FILE__);
  Serial.println("2_Battery_Voltage_Read_Test Starting");

  pinMode(LED1, OUTPUT);                    //for PCB LED
  pinMode (BATVREADON, OUTPUT);             //for turning on resistor divider
}
