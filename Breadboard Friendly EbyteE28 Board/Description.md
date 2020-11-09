### Breadboard Friendly Ebyte E28 Board

![](/E28_BBF_Board.jpg)

This is a board to adapt a 2.4Ghz Ebyte E28-2G4M12S  and E28-2G4M20S LoRa modules into a breadboard friendly format. The board will allow you to use your Ebyte E28 module with a standard 0.1” matrix breadboard or strip board. 

The Ebyte E28 module has a PCB antenna, so no external antenna is needed. The boards are supplied with the 0.1" pins unsoldered, leaving you the option of how to assemble or use the board in a project. 
This board can be used with the Easy mikroBUS™ Pro Mini controller available in this shop, but the E28-2G4M20S module is only recommended if you absolutely must have the extra output power, see the notes below.  

The  E28-2G4M20S module needs additional RX and TX enable pins, these are available on the board and the SX12XX-LoRa  does support their use. However if your using the Easy mikroBUS™ Pro Mini controller boards you will need to connect up links LK1 and LK2 on the board and use Arduino pins A2 and A3 to control the E28 modules TXEN and RXEN pins respectively. Also with A6 and A7 already allocated for TXEN and RXEN they cannot be used elsewhere so in total the following Arduino pins will not be available for other uses, A2,A3,A6,A7. The  E28-2G4M12S is a better choice for the Easy mikroBUS™ Pro Mini controller boards and no links need to be made.  

The board can be used with an E28-2G4M12S as a compatible 2x8pin mikroBUS™ board but the AN pin on the mikroBUS™ socket will need to be capable of digital operation, and you need to make link LK3.

The Ebyte E28 is a 3.3V supply and 3.3V logic level device, do not use directly with 5V controller boards such as the Arduino UNO. 
The pinout of the board is below, the module pins on the outside.


![](/E28_BBF.jpg)


There are many examples for using these boards for GPS trackers or sensors provided in an Arduino the library here;  

[https://github.com/StuartsProjects/SX12XX-LoRa](https://github.com/StuartsProjects/SX12XX-LoRa)

Note: Some of the boards are marked on the silk screen print as for the  E28-2G4M16S, this is a error and it should read E28-2G4M12S, this has no impact on the correct operation of the board.  




