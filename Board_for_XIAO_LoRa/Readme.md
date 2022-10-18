## Seeeduino XIAO

This is the repository for programs for the Seeeduino XIAO some of which use the SX127X-LoRa library. The XIAO is discussed in this article;


[**https://stuartsprojects.github.io/2022/10/03/Seeeduino_XIAO.html**](https://stuartsprojects.github.io/2022/10/03/Seeeduino_XIAO.html)


### 1\_LED\_Blink

Tests that the on board LED is working

### 2\_LoRa\_Register\_Test

A standalone program, no LoRa library required, that will read and printout the register values of a LoRa device connected to the SPI bus. 

### 3\_LoRa\_Transmitter

Simple LoRa transmitter to test the LoRa device is working. 

### 4\_LoRa\_Receiver

Simple LoRa receiver to test the LoRa device is working.

### 5\_WakeUpOnExternalInterrupt

Puts the XIAO into deep sleep and it wakes up when the logic state of a GPIO pin changes. 

### 6\_RTCAlarm\_Sleep\_Wakeup

Uses the RTC alarm to wake the XIAO up from deep sleep after a specified time. 

### 7\_LoRa\_Transmitter\_Sleep\_Switch\_Wakeup

And external switch is used to wake up the XIAO from deep sleep. The program then sends a LoRa packet and goes back to sleep.

### 8\_LoRa\_Transmitter\_Sleep\_RTC\_Wakeup

Uses the XIAO processors RTC alarm to wake up the XIAO from deep sleep. The program then sends a LoRa packet and goes back to sleep. 

### 9\_GPS\_Echo\_Serial1

Copies the character coming from the GPS to the Arduino IDE Serial monitor. Basic check to see if a GPS is working. 

### 10\_GPS\_Echo\_Sercom2

Demonstrates the re-direction of a Serial port to one of the other Sercom ports, in this case the port normally used for I2C.

### 11\_I2C\_Scanner

Scans the I2C bus and shows the addresses of devices found.

### 12\_SSD1306\_SH1106\_OLED\_Checker

Checks that an SSD1306 or SH1106 OLED is working. 

### 13\_GPS\_Checker\_With\_Display

Reads a GPS and shows the Latitude, Longitude, Altitude, Speed, Number
of satellites in use, HDOP value, time and date on an OLED display and IDE serial monitor.

### 14\_Battery\_Voltage\_Read

Demonstrates how to use a resistor divider to read a battery voltage. 

### 15\_GPS\_Tracker\_Transmitter

A tracker transmitter, read the location fixes from a connected GPS and then transmits the location as a LoRa packet, use program 16\_GPS\_Tracker\_Receiver\_With\_Display\_and\_GPS to display the transmitted data.

### 16\_GPS\_Tracker\_Receiver\_With\_Display\_and\_GPS

A receiver for the 15\_GPS\_Tracker\_Transmitter program, transmitter and receiver must have the same LoRa settings.

### 17\_BME280\_Sensor\_Transmitter

Reads a BME280 sensor and transmits the sensor data via LoRa.

### 18\_BME280\_Sensor\_Receiver

Receiver for program 17\_BME280\_Sensor\_Transmitter, displays sensor data on an XIAO LoRa board.


<br>
<br>

#### Stuart Robinson
#### October 2022
