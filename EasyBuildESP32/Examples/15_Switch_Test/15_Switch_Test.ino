/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 04/08/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This program tests a series of switches, SWITCH1, SWITCH2 and SWITCH3.

  The switches are read in a loop, if a switch is pressed then a message prints to the serial monitor.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#define SWITCH1 0          //pin for SWITCH1
#define SWITCH2 13         //pin for SWITCH2 
#define SWITCH3 4          //pin for SWITCH3


void loop()
{
  if (!digitalRead(SWITCH1))
  {
    Serial.println(F("SWITCH1 Pressed"));
  }

  if (!digitalRead(SWITCH2))
  {
    Serial.println(F("SWITCH2 Pressed"));
  }

  if (!digitalRead(SWITCH3))
  {
    Serial.println(F("SWITCH3 Pressed"));
  }

}


void setup()
{
  pinMode(SWITCH1, INPUT_PULLUP);              //setup switch pin, ground to activate
  pinMode(SWITCH2, INPUT_PULLUP);              //setup switch pin, ground to activate
  pinMode(SWITCH3, INPUT_PULLUP);              //setup switch pin, ground to activate

  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.println();
  Serial.println(__TIME__);
  Serial.println();
  Serial.println(__DATE__);
  Serial.println();
  Serial.println(F("Waiting for switch press"));
}
