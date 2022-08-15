/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/08/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a simple program to test a GPS. It reads characters from the GPS using
  a hardware serial port, Serial2, and then sends them (echoes) to the Arduino IDE serial monitor.

  If you get no data displayed on the serial monitor, the most likely cause is that you have the receive
  data pin connected incorrectly.

  At program start you should see '11_GPS_Echo Starting' in the serial monitor, if you dont the serial
  monitor baud rate is probably incorrectly set. If you then see data displayed on the serial terminal
  which appears to be random text with odd symbols its very likely you have the GPS serial baud rate set
  incorrectly.

  Serial monitor baud rate is set at 115200.

*******************************************************************************************************/

#define RXDATA 16                         //pin where data from GPS is received, default for Serial2, but can be changed
#define TXDATA 17                         //pin where data is sent to GPS, default for Serial2, but can be changed 


void loop()
{
  while (Serial2.available())
  {
    Serial.write(Serial2.read());
  }
}


void setup()
{
  Serial2.begin(9600, SERIAL_8N1, RXDATA, TXDATA);            //serial for GPS
  
  Serial.begin(115200);
  Serial.println();
  Serial.println("11_GPS_Echo Starting");
}
