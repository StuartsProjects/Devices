/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 06/08/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation - This test program has been written to check that hardware for reading the nodeMCU
  VIN pin voltage has been assembled correctly such that it is funtional. The value defined as 'ADMultiplier'
  is used to adjust the value read from the 47K\10K resistor divider and convert into mV.

  When the voltage is being read, the pin LED on the NodeMCU will flash. The read voltage is printed onto
  the OLED display.

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <U8x8lib.h>                                        //get library here >  https://github.com/olikraus/u8g2 
U8X8_SSD1306_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);      //use this line for standard 0.96" SSD1306
//U8X8_SH1106_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);     //use this line for 1.3" OLED often sold as 1.3" SSD1306
#define DEFAULTFONT u8x8_font_chroma48medium8_r             //font for U8X8 Library

#define ADMultiplier 5.72                      //adjustment to convert AD value read into mV of battery voltage 
#define VINAD 36                               //Resistor divider for battery connected here 
#define LED1 2                                 //NodeMCU on board LED 

uint16_t volts = 0;


void loop()
{
  digitalWrite(LED1, HIGH);
  printVINVoltage();
  Serial.println();
  screen1();                                   //send volts to OLED display
  delay(50);                                   //so we can see LED flash
  digitalWrite(LED1, LOW);
  delay(1500);
}


void printVINVoltage()
{
  //get and display supply volts on terminal or monitor
  Serial.print(F("VIN Voltage "));
  Serial.print(readVINVoltage());
  Serial.println(F("mV"));
}


uint16_t readVINVoltage()
{
  //relies on internal reference and 47K & 10K resistor divider
  uint16_t temp;
  byte index;

  volts = 0;
  temp = analogRead(VINAD);

  for (index = 0; index <= 4; index++)      //sample AD 5 times
  {
    temp = analogRead(VINAD);
    volts = volts + temp;
  }
  volts = ((volts / 5) * ADMultiplier);

  return volts;
}


void screen1()
{
  disp.clearLine(0);
  disp.setCursor(0, 0);
  disp.print(F("VIN "));
  disp.print(volts);
  disp.print(F("mV"));
}


void setup()
{
  pinMode(LED1, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.println();
  Serial.println(__TIME__);
  Serial.println();
  Serial.println(__DATE__);
  Serial.println();

  disp.begin();
  disp.setFont(DEFAULTFONT);
  disp.clear();
}
