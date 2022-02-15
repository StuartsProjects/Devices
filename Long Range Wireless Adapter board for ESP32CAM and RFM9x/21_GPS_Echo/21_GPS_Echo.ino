/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 15/02/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a simple program to test a GPS connected to the ESP32CAM board.
  The program reads characters from the GPS connected to the boards GPS connector and sends them (echoes)
  them to the Serial debug port on pin 33 of the ESP32CAM.

  Serial monitor baud rate is set at 115200.

*******************************************************************************************************/

#define RXD0 3                //RX pin for GPS port.
#define TXD0 1                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             //TX pin for GPS port

#define RXD2 40               //RX pin for Serial2, out of range so not in use  
#define TXD2 33               //TX pin for Serial2 

#define GPSserial Serial2     //the GPS port
#define GPSBAUD 9600          //default GPS baud rate

void loop()
{
  while (GPSserial.available())
  {
    Serial.write(GPSserial.read());
  }
}


void setup()
{
  GPSserial.begin(GPSBAUD, SERIAL_8N1, RXD0, TXD0);   //GPS port on programming port
  Serial.begin(115200, SERIAL_8N1, RXD2, TXD2);       //format is Serial.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.println();
  Serial.println();
  Serial.println("Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2)");
  Serial.println(__FILE__);
  Serial.println();

}
