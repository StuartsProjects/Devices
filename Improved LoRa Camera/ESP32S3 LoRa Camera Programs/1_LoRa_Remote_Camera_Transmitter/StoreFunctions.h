/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 10/06/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//**********************************************************************
// FLASH Store used store names
// "Store"
// "StoredVarCRC"
// "StoredVarSUM"
//**********************************************************************

/*
  Issues:
*/

//**********************************************************************
// Store function declarations and variables
//**********************************************************************

//#define DEBUG  //enable to print structure data as read

//so that Monitorport prints default to the primary Monitorport port of Monitorport
#ifndef Monitorport
#define Monitorport Serial
#endif

#include <Preferences.h>  //details here >  https://docs.espressif.com/projects/arduino-esp32/en/latest/api/preferences.html
Preferences FlashSettings;

void init_store();
bool check_store();
void erase_store();
void copy_var_store();
void copy_vartemp_store();
void copy_store_var();
void copy_store_vartemp();
uint16_t CRC_structure(uint8_t* ptr, uint8_t structsize);
uint16_t SUM_structure(uint8_t* ptr, uint8_t structsize);

//**********************************************************************
// Start FLASH Store functions
//**********************************************************************

void init_store() {
#ifdef DEBUG
  Monitorport.println(F("init_store()"));
#endif

  FlashSettings.begin("Store", false);  //open namespace "Store" in false mode = RW mode
}


bool check_store() {
  //check the sum and CRC of structure in store agrees with saved sum and CRC
  uint16_t savedCRC;
  uint16_t savedSUM;
  uint16_t vartempCRC;
  uint16_t vartempSUM;

#ifdef DEBUG
  Monitorport.println(F("check_store()"));
#endif

  copy_store_vartemp();

  vartempCRC = CRC_structure((uint8_t*)&vartemp, sizeof(settings));
  vartempSUM = SUM_structure((uint8_t*)&vartemp, sizeof(settings));

  Monitorport.print(F("CRC of var structue in store 0x"));
  Monitorport.println(vartempCRC, HEX);

  savedCRC = FlashSettings.getUShort("StoredVarCRC", 0);
  Monitorport.print(F("StoredVarCRC 0x"));
  Monitorport.println(savedCRC, HEX);

  Monitorport.print(F("Sum of var structue in store 0x"));
  Monitorport.println(vartempSUM, HEX);

  savedSUM = FlashSettings.getUShort("StoredVarSUM", 0);
  Monitorport.print(F("StoredVarSUM 0x"));
  Monitorport.println(savedSUM, HEX);

  if (vartempCRC != savedCRC) {
    Monitorport.println(F("StoredVarCRC not a match"));
    return false;
  }

  if (vartempSUM != savedSUM) {
    Monitorport.println(F("StoredVarSUM not a match"));
    return false;
  }

  if (savedSUM == 0) {
    Monitorport.println(F("Error - StoredVarSUM is zero"));
    return false;
  }

  return true;
}


void copy_var_store() {
#ifdef DEBUG
  Monitorport.println("copy_var_store()");
#endif
  uint16_t storeCRC, storeSUM, readCRC, readSUM;

  FlashSettings.putBytes("Store", &var, sizeof(settings));  //copy settings var structure to store

  storeCRC = CRC_structure((uint8_t*)&var, sizeof(var));  //calculate crc of var structure
  Monitorport.print(F("var structure CRC 0x"));
  Monitorport.println(storeCRC, HEX);
  FlashSettings.putUShort("StoredVarCRC", storeCRC);

  readCRC = FlashSettings.getUShort("StoredVarCRC", 0);
  Monitorport.print(F("var Structure CRC read back from store 0x"));
  Monitorport.println(readCRC, HEX);

  storeSUM = SUM_structure((uint8_t*)&var, sizeof(var));
  Monitorport.print(F("var structure SUM 0x"));
  Monitorport.println(storeSUM, HEX);
  FlashSettings.putUShort("StoredVarSUM", storeSUM);

  readSUM = FlashSettings.getUShort("StoredVarSUM", 0);
  Monitorport.print(F("var Structure SUM read back from store 0x"));
  Monitorport.println(readSUM, HEX);
}


void copy_vartemp_store() {
#ifdef DEBUG
  Monitorport.println("copy_vartemp_store()");
#endif
  uint16_t storeCRC, storeSUM, readCRC, readSUM;

  FlashSettings.putBytes("Store", &vartemp, sizeof(settings));  //copy settings vartemp structure to store

  storeCRC = CRC_structure((uint8_t*)&vartemp, sizeof(vartemp));  //calculate crc of vartemp structure
  Monitorport.print(F("vartemp structure CRC 0x"));
  Monitorport.println(storeCRC, HEX);
  FlashSettings.putUShort("StoredVarCRC", storeCRC);

  readCRC = FlashSettings.getUShort("StoredVarCRC", 0);
  Monitorport.print(F("var Structure CRC read back from store 0x"));
  Monitorport.println(readCRC, HEX);

  storeSUM = SUM_structure((uint8_t*)&vartemp, sizeof(vartemp));
  Monitorport.print(F("vartemp structure SUM 0x"));
  Monitorport.println(storeSUM, HEX);
  FlashSettings.putUShort("StoredVarSUM", storeSUM);

  readSUM = FlashSettings.getUShort("StoredVarSUM", 0);
  Monitorport.print(F("var Structure SUM read back from store 0x"));
  Monitorport.println(readSUM, HEX);
}


void copy_store_var() {
#ifdef DEBUG
  Monitorport.println("copy_store_var()");
#endif
  FlashSettings.getBytes("Store", &var, sizeof(settings));
}


void copy_store_vartemp() {
#ifdef DEBUG
  Monitorport.println("copy_store_vartemp()");
#endif

  FlashSettings.getBytes("Store", &vartemp, sizeof(settings));  //copy store to vartemp structure
}


void erase_store() {
#ifdef DEBUG
  Monitorport.println("erase_store()");
#endif

  memset(&vartemp, 0, sizeof(struct settings));
  FlashSettings.putBytes("Store", &vartemp, sizeof(settings));  //settings vartemp structure to store
  FlashSettings.putUShort("StoredVarCRC", 0x0000);              //CRC of all 0s is 0xFFFF
  FlashSettings.putUShort("StoredVarSUM", 0x0);
}

//**********************************************************************
// End FLASH Store functions
//**********************************************************************


//**********************************************************************
// Common Store functions
//**********************************************************************

uint16_t CRC_structure(uint8_t* ptr, uint8_t structsize) {
  uint8_t index, byteread;
  uint16_t i, structureCRC;
  uint8_t j;

  structureCRC = 0xFFFF;  //start value for CRC16

#ifdef DEBUG
  Monitorport.print("Structure contents ");
#endif

  for (index = 0; index < structsize; index++) {
    byteread = (*(ptr + index));
#ifdef DEBUG
    Monitorport.print(byteread, HEX);
    Monitorport.print(" ");
#endif
    structureCRC ^= ((uint16_t)byteread << 8);

    for (j = 0; j < 8; j++) {
      if (structureCRC & 0x8000)
        structureCRC = (structureCRC << 1) ^ 0x1021;
      else
        structureCRC <<= 1;
    }
  }

#ifdef DEBUG
  Monitorport.println();
#endif

  return structureCRC;
}


uint16_t SUM_structure(uint8_t* ptr, uint8_t structsize) {
  uint8_t index, byteread;
  uint16_t i, structureSUM;

  structureSUM = 0;

#ifdef DEBUG
  Monitorport.print("Structure contents ");
#endif

  for (index = 0; index < structsize; index++) {
    byteread = (*(ptr + index));
#ifdef DEBUG
    Monitorport.print(byteread, HEX);
    Monitorport.print(" ");
#endif
    structureSUM = structureSUM + byteread;
  }

#ifdef DEBUG
  Monitorport.println();
#endif

  return structureSUM;
}
//**********************************************************************
