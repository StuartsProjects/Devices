/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/02/23

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a simple program to test a GPS connected to a Seeeduino XIAO SAMD21. 
  The program reads characters from the GPS through a serial port and sends (echoes) them to the IDE serial
  monitor. Connect the GPS RX pin to XIAO A6 (TX) and GPS TX pin to XIAO A7 (TX).

  Before programming the XIAO or powering it up you must close the Arduino IDE Serial Monitor. 

  Serial monitor baud rate is set at 115200.

*******************************************************************************************************/


void loop()
{
  while (Serial1.available())
  {
    Serial.write(Serial1.read());
  }
}


void setup()
{
  Serial1.begin(9600);
  Serial.begin(115200);
  Serial.println("8_GPS_Echo Starting");
}
