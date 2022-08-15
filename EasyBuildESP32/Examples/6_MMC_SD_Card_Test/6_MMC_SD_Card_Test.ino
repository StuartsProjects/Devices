/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 13/08/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This test program has been written to check that a connected SD card adapter is 
  working in MMC mode. The code was tested on an NodeMCU ESP32S.
  
  The program creates a file called LOGXXXX.TXT, where XXXX is a number that increases every time the
  program is restarted. The program opens the file and writes a line like this to the file;

  #1 Hello World!

  The file is closed and the file contents are written to the IDE serial monitor and a directory of the
  SD card printed too. The process repeats with #2 Hello World! being appended to the file next. The
  directory listing allows you to keep track of the increasing size of the file. Problems with reading
  or writing to the SD card should result in an ERROR message on the IDE serial monitor.

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <SPI.h>
#include <SD_MMC.h>

char filename[] = "/LOG0000.TXT";               //filename used as base for creating root, 0000 replaced with numbers
uint16_t linenumber = 0;

File file;


void loop()
{
  linenumber++;

  Serial.print("Write to file > ");
  Serial.print("#");
  Serial.print(linenumber);
  Serial.println(" Hello World!");

  file = SD_MMC.open(filename, FILE_APPEND);
  file.print("#");
  file.print(linenumber);
  file.println(" Hello World!");
  file.close();
  dumpFile(filename);
  Serial.println();

  file = SD_MMC.open("/");
  file.rewindDirectory();
  printDirectory(file, 0);

  Serial.println();
  delay(1500);
}


void printDirectory(File dir, int numTabs)
{
  Serial.println("Card directory");

  while (true)
  {
    File entry =  dir.openNextFile();
    if (! entry)
    {
      // no more files
      break;
    }

    for (uint8_t i = 0; i < numTabs; i++)
    {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory())
    {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    }
    else
    {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}


bool dumpFile(char *buff)
{
  Serial.print("Print file ");

  if (SD_MMC.exists(buff))
  {
    Serial.println(buff);
  }
  else
  {
    Serial.print("ERROR ");
    Serial.print(buff);
    Serial.println(" not found - program halted");
    while (1);
  }

  file = SD_MMC.open(buff);
  file.seek(0);

  if (file)                                    //if the file is available, read from it
  {
    while (file.available())
    {
      Serial.write(file.read());
    }
    file.close();
    return true;
  }
  else
  {
  Serial.print("ERROR ");
  Serial.println(" dumping file ");
  Serial.print(buff);
  Serial.println(" - program halted");
  while (1);
  }
}


uint8_t setupSDLOG(char *buff)
{
  //creats a new filename

  uint16_t index;

  File dir;

  for (index = 1; index <= 9999; index++)
  {
    buff[4] = index / 1000 + '0';
    buff[5] = ((index % 1000) / 100) + '0';
    buff[6] = ((index % 100) / 10) + '0';
    buff[7] = index % 10 + '0' ;

    if (! SD_MMC.exists(filename))
    {
      // only open a new file if it doesn't exist
      dir = SD_MMC.open(buff, FILE_WRITE);
      break;
    }
  }

  dir.rewindDirectory();                          //stops SD_MMC.exists() command causing issues with directory listings
  dir.close();

  if (!dir)
  {
    return 0;
  }
  return index;                                   //return number of root created
}



void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.println();
  Serial.println(__TIME__);
  Serial.println();
  Serial.println(__DATE__);
  Serial.println();

  SPI.begin();

  if (!SD_MMC.begin("/sdcard", true))         //start MMC card in 1 bit mode 
  {
    Serial.println();
    Serial.println("ERROR MMC Card Mount Failed - program halted");
    Serial.println();
    while (1);
  }

  Serial.println("MMC Card Mount OK");

  file = SD_MMC.open("/");
  setupSDLOG(filename);
}
