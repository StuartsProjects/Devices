<br>
<p align="center">
  <img width="460"  src="/images/Easy_ESP32CAM_PCB.jpg">
</p>

<br>


<br>
<p align="center">
  <img width="460"  src="/images/Easy_ESP32CAM_Tracker.jpg">
</p>

<br>


This is a simple PCB that can be used turn your low cost ESP32CAM into a LoRa enabled GPS tracker cable of taking and storing pictures. 

A simple tracker can be built just by soldering the LoRa module and antenna in place and adding some pin headers for programming, GPS and battery connections.

Additional components can be added to provide a boot switch, improved regulator, battery monitor and debug connections. 

Only a PCB is supplied, no ESP32CAM, LoRa module, GPS or other components are provided.   

With good line of sight between a LoRa transmitter and receiver, such as from the ground to a high altitude balloon, you can receive the GPS co-ordinates from hundreds of kilometres away. There is Arduino software that allows the ESP32CAM to take pictures and store them on SD card whilst still being used as a GPS tracker. 

The assembled tracker requires that you are familiar with the ESP32CAM software and have a good working example. No troubleshooting information for the ESP32CAM itself is provided.

The Arduino software needed to make the ESP32CAM a tracker, reading the GPS, taking a picture, saving it to SD and then sending the GPS coordinates via LoRa has been tested. The use of the ESP32CAM as a plain tracker, no camera or SD Card used is provided by program '10\_ESP32CAM\_LoRa\_Tracker\_Basic.ino' and the same program that takes pictures and stores them on SD card is program '15\_ESP32CAM\_LoRa\_Tracker\_Camera.ino'

The required RFM9X LoRa module can be soldered in place surface mount style or using the supplied 2mm pin headers. A wire antenna or edge SMA socket can be fitted for antenna connections (neither are supplied).

The ESP32CAM comes with a AMS1117 regulator which needs a input voltage of at least 4.6v which rather limits the battery choices. You can remove the AMS1117 from the ESP32CAM and fit on the Easy ESP32CAM tracker board a low dropout regulator such as MCP1700, MCP1702 or HT7833 which would need an input voltage of only 3.5v or so. You can fit either a T092 or SOT89 package regulator.

On the bottom edge of the PCB there is a 4 pin connector for the GPS and 2 pin connection for a battery, alternately a JST style silicone battery lead can be secured to the bottom of the PCB. 

The program connector is positioned on the edge of a PCB so that you can easily attach spring loaded test hook, or just fit a 4 pin 0.1" header. You need to disconnect the GPS for programming or use the switch which allows you to disconnect the GPS from circuit when programming is required. If the GPS is connected and you cannot program the ESP32CAM put the switch in the other position. A two pin header can be fitted to allow debug output to be read via an attached USB to serial adapter. 

There are spaces for additional decoupling capacitors (SMT 1206 size). You can fit two resistors which allow the supply voltage to be read. You can attach a wire to pin 33 on the ESP32 and use it as a serial debug output when the GPS is in use on the programming port.

Holes in the Easy ESP32CAM tracker board allow you to access the reset switch on the ESP32CAM and see the pin 33 LED. You can fit a boot switch to the PCB for easy programming. 

For a lightweight GPS, you can make up a light weight  L70/L76 GPS using the breakout **[here](https://www.tindie.com/products/20921/)** or use a Ublox MAX 8Q using the breakout board **[here](https://www.tindie.com/products/20919/)**. 

There are a series of example program which test specific parts of the ESP32CAM, its strongly recommend to go through the examples in order. 

**Good luck**

