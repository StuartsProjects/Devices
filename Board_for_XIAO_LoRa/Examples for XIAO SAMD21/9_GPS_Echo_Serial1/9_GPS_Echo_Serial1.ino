/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO SAMD21.

  Program Operation - This is a simple program to test a GPS connected to a Seeeduino XIAO. The program
  reads characters from the GPS through a UART serial port and sends (echoes) them to the IDE serial
  monitor. Connect the GPS RX pin to XIAO A6 (TX) and GPS TX pin to XIAO A7 (RX).

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
  Serial1.begin(9600);                    //Serial port for GPS

  Serial.begin(115200);
  Serial.println();
  Serial.println(F(__FILE__));
  Serial.println();

}
