/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 03/11/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO RP2040.

  Program Operation - This is a simple program to test a GPS connected to a Seeeduino XIAO RP2040. The
  program reads characters from the GPS through a UART serial port and sends (echoes) them to the IDE
  serial monitor. Connect the GPS RX pin to XIAO D6 (TX) and GPS TX pin to XIAO D7 (RX).

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
  Serial.println("9_GPS_Echo_Serial1 Starting");
}
