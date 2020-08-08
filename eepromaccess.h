/*
 * EEPROM Access by Cameron Tibbits
 * 
 * Library for writing structs to the internal eeprom
 * all other classes will pass a pointer to their data
 * strutures. This class hast a setup()function that 
 * will check the eeprom for previous Data with the correct
 * version number. If we find one then those values will 
 * be loaded into ram. If there is no previous config it 
 * will generate a new map with default value. Default values
 * are determined by their parent class.
 * 
 */

#ifndef eeproma_h
#define eeproma_h
#include "Arduino.h"
#include <EEPROM.h>
#include "ph.h"
#include "lights.h"


  
enum EEPROM_SELECT {
  NONE,
  ALL,
  EEPROM_SETTINGS,
  PH_SETTINGS,
  LIGHT_SETTINGS
};

struct EMap {           // Struct to store all of eeprom settings

  char VERSION[4] = "AA3";     // number to check and see if storage is valid
  int checkVar = 1;

  // Partition Map: holds start address of each stored struct in EEPROM

};

const int settingsAddressEEPROM = 0;
const int phAddressEEPROM = sizeof(EMap) + 1;                  // Address offset for PH data
const int lightAddressEEPROM = sizeof(PHData) + phAddressEEPROM + 1;   // Address offset for Light data

class EepromAccess {

  public:
  EepromAccess(PHData *, LightData *);
    void setup();
    void loop();
    // select settings, set and get
    void updateSelectEeprom(EEPROM_SELECT selection);
    void getSelectEeprom(EEPROM_SELECT selection);
    // All settings, set and get
    void updateSettings();
    void getSettings();
  private:
    bool checkForSettings (EMap checkMap);
    bool settingsChanged;
    void saveMap();
    EMap getMap ();
    
    bool _needsUpdated = false;   // Flag for when to write new data
    


  public:  EMap _eepromMap;
    EEPROM_SELECT _settingSelect; // Which Data to get or retrieve
    PHData* _phData;              // Pointer to PH Data to be stored
    LightData* _lightData;        // Pointer to Light Data to be stored
};
#endif
