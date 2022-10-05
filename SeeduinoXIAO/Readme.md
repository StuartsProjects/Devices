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

And external switch is used to wake up the XIAO from deep sleep, transmit a LoRa packet and go back to sleep.

### 8\_LoRa\_Transmitter\_Sleep\_RTC\_Wakeup

Uses the RTC alarm to wake up the XIAO from deep sleep, which sends a LoRa packet and goes back to sleep again. 

More examples coming soon.

<br>
<br>

#### Stuart Robinson
#### October 2022
