### YModem file transfers to PC

For the StuartsProjects ESP32CAM applications the image receive program receives the picture (from the remote ESP32CAM) as a series of segments and writes these into a memory array. When the transfer is finished this array is written as a file to the receivers SD card. This normally means that to see the images you need to remove the SD card from the ESP32CAM and read it in a tablet or PC etc. Whilst the SD card is out of the receiving ESP32CAM transfer of images is halted. 

To work around this issue it is possible to transfer the contents of the array as an image over the serial program upload port to an attached PC. The protocol used is YModem and its received on the PC with Tera Term, see set-up instructions below. 

It makes sense to test that you have Tera Term set up correctly on the PC before proceeding to use it to transfer images from the remote ESP32CAM. A test program is provided for this purpose; **5\_ESP32CAM\_PCFileTransfer\_Test**, so load this onto the Long Range Wireless Adapter your using as a receiver.


### Tera Term - Setup Serial Port  

I was using version 4.106 of Tera Term.

After the ESP32CAM is loaded with the **5\_ESP32CAM\_PCFileTransfer\_Test** program you need to configure Tera Term to use the correct program upload port for file transfers. 

On my PC, Windows8, the USB to serial adapter, that's used to program the ESP32CAM,  turns up on port COM42 so in Tera Term select \Setup\Serial Port... and configure for 115200 baud, see below;

<p align="center">
  <img width="400"  src="/YModem1.jpg">
</p>
<br>


### Tera Term - Set-up File Transfer directory

Tera Term needs to know where to save the files transferred. You probably need to change it, as it will default to its own program directory. I created a folder on my desktop called YModem and in Tera Term selected \File\Change Directory and browsed to the YModem directory;


<p align="center">
  <img width="400"  src="/YModem2.jpg">
</p>
<br>

<br>

If you want to make the change the default, so you don't have to set it every time, locate the TERATERM.INI file in the Tera Term program directory and alter this line to point to a folder of your choice;

	FileDir=C:\Users\Stuart\Desktop\YModem` 


### Timeouts on Tera Term

I noticed that if you leave the transfers and YModem uploads running, that after the first image is sent when the next YModem transfer starts there is a delay after the PC picks up the file name of the impending transfer before the actual transfer starts. This delay was around 20 seconds. If this delay is a problem then load the TERATERM.INI file, located in the Tera Term program folder into an text editor and you will find this section;
  
    ; YMODEM Timeout value(v1,v2,v3,v4,v5) by seconds
    ;  v1=NAK mode: Timeout value for first packet
    ;  v2=CRC mode: Timeout value for first packet
    ;  v3=Timeout short time 
    ;  v4=Timeout long time 
    ;  v5=Timeout very long time 
    YmodemTimeouts=10,3,10,20,60

Note the line; YmodemTimeouts=10,3,10,**20**,60. The value 20, highlighted in bold can be changed to say 1 and the delay mentioned above does not occur. 


### Tera Term - Start File Transfers

To start Tera Term file transfers running, in Tera Term select, \File\Transfer\YMODEM\Receive..

<p align="center">
  <img width="400"  src="/YModem3.jpg">
</p>
<br>


And that's it, the transmitting program,   **5\_ESP32CAM\_PCFileTransfer\_Test**, should now attempt to transfer a test file called TESTFILE1.TXT. The program first allocates an array in PSRAM and then fills the array with 8 lines of ASCII characters 0x20 to 0x7E. This file is then transferred repeatedly using YModem. The destination directory on the PC should fill up with files called 'TESTFILE1.TXT' etc. The file should look like this when loaded up in Notepad.  


     !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
     !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
     !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
     !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
     !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
     !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
     !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
     !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
     !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~


Files should now be transferred into the selected folder on the PC. Note that the Tera Term receiver will recognise that if the the same file name is being transferred and the destination files will be named, Pic1.JPG, Pic11.JPG, Pic12.JPG etc. 

This is the Tera Term window showing the file transfer process;

<p align="center">
  <img width="400"  src="/YModem6.jpg">
</p>
<br>


### Stuart Robinson

### February 2022
