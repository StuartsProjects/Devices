### Mini Tracker Board for LoRa

This is a board that will enable you to build a small and light LoRa based GPS tracker. LoRa trackers are capable of sending GPS co-ordinates over great distances. The current record for receiving signals line of sight is 832km from a high altitude balloon, see **[here](https://www.thethingsnetwork.org/article/lorawan-world-record-broken-twice-in-single-experiment-1)**. 

This is a board only, the DRF1278 and Quectel GPS are not supplied.

There is a 5 pin connector fitted for programming the board as an Arduino device in the normal way via a USB to Serial adapter. There is also a 3 pin connector that will allow the tracker to be powered from the 5V servo supply usually found in drones and other radio controlled models.  

A simple receiver can be built from the **[Easy Mikrobus Pro Mini](https://github.com/StuartsProjects/Devices/tree/master/Easy%20Mikrobus%20Pro%20Mini)** kit and **https://github.com/StuartsProjects/Devices/tree/master/Breadboard%20Friendly%20DRF1278%20LoRa%20Module** Module 

Detailed assembly instructions are not provided, so be sure you are happy to assemble 0805 SMT devices and have the equipment to load the Arduino bootloader into the ATmega328P processor which needs to be done before its soldered in place.  

**Do not operate the DRF1278F module without an antenna.** 

**The DRF1278F and Quectel GPS are 3.3V supply and logic level devices, do not use directly with 5V controller boards such as the Arduino UNO.** 

There are examples for using this module for GPS trackers or sensors provided in an Arduino library at this link;  [SX12XX Library](https://github.com/StuartsProjects/SX12XX-LoRa).