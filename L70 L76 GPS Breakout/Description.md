### Quectel L70 and L76 GPS Breakout Board

This is a board that allows you to use a low cost Quectel L70 or L76 GPS to build a very light GPS board for use with your micro controller projects. With a simple wire antenna the board and GPS can weigh as little as 2g. You can also build the board with a small ceramic patch antenna and this weighs 5.9g. 

This is a board for the GPS only, you will need to supply your own GPS. 

In is simplest build all you need to do is solder the GPS module to the board, add a 7 pin 0.1” header, a bit of stiff wire for an antenna and make two links.

The Quectel GPSs can be put into 'balloon' mode so that it operates up to 80km altitude. There are examples of this configuration in the tracker applications in this library;

[SX12XX_Library](https://github.com/StuartsProjects/SX12XX-LoRa)

There are options you can add to the board;

LED fix indicator

Decoupling capacitors

GPS backup for hot fixing

Lithium battery or super capacitor

Power switching to turn the GPS off and save power

The board uses 0805 sized surface mount components to keep the size and weight down. Previous experience of SMT assembly is essential and static precautions are needed when assembling and handling the Quectel GPS. All capacitors are ceramic chip types. 

**The assembled GPS module is for use on 3.3V microcontroller projects only**

**Do not connect directly to 5V microcontrollers such as the Arduino UNO**

The build instructions can be found be selecting the link below.

**[Build Instructions](https://github.com/StuartsProjects/Devices/tree/master/L70%20L76%20GPS%20Breakout)**


