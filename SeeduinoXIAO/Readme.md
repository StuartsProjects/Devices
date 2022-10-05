## Cycle Alarm Extender

This is the repository for programs for the Seeeduino XIAO soem of which use the SX127X-LoRa library. The XIAO is discussed in this article;

[**https://stuartsprojects.github.io/2022/09/08/Cycle-Alarm-Extender.html**](https://stuartsprojects.github.io/2022/09/08/Cycle-Alarm-Extender.html)


## Example programs

The programs in this repository are provided so that you can test the board you have built, the fully working transmitter and receiver programs are also provided.

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

### 6\_RTCAlarm

Demonstrates the use of the XIAO real time clock alarm during a running program.

### 7\_RTCAlarm\_Sleep\_Wakeup

Uses the RTC alarm to wake the XIAO up from deep sleep after a specified time. 

### 8_LoRa_Transmitter_Sleep_Switch_Wakeup

And external switch is used to wake up the XIAO from deep sleep, transmit a LoRa packet and go back to sleep.

### 9_LoRa_Transmitter_RTC_Sleep_Wakeup

Uses the RTC alarm to wake up the XIAO from deep sleep, which sends a LoRa packet and goes back to sleep again. 

### 10_GPS_Echo_Serial

Copy the output of a GPS onto the IDE serial monitor

### 11_GPS_Echo_Sercom2 

Demonstrates the re-direction of a serial port using Sercom. The GPS is connected to the XIA) default I2C port which is then re-configured as a serial UART port. 

 


<br>
<br>

#### Stuart Robinson
#### September 2022
