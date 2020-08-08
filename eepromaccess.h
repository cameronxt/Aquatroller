#ifndef eeproma_h
#define eeproma_h
#include "Arduino.h"
#include <EEPROM.h>
#include "ph.h"
#include "lights.h"

const int settingsAddressEEPROM = 0;
const int phAddressEEPROM = sizeof(settingsAddressEEPROM) + 1;                  // Address offset for PH data
const int lightAddressEEPROM = sizeof(phAddressEEPROM) + phAddressEEPROM + 1;   // Address offset for Light data
  
enum EEPROM_SELECT {
  NONE,
  ALL,
  EEPROM_SETTINGS,
  PH_SETTINGS,
  LIGHT_SETTINGS
};

struct EMap {           // Struct to store all of eeprom settings

  char VERSION[4] = "AA1";     // number to check and see if storage is valid

  // Partition Map: holds start address of each stored struct in EEPROM

};



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
    bool checkForSettings (EMap *checkMap);
    bool settingsChanged;
    void saveSettings(EMap* eMap);
    void getSettings (EMap* savedMap);
    
    bool _needsUpdated = false;   // Flag for when to write new data
    


    EMap _eepromMap;
    EEPROM_SELECT _settingSelect; // Which Data to get or retrieve
    PHData* _phData;              // Pointer to PH Data to be stored
    LightData* _lightData;        // Pointer to Light Data to be stored
};
#endif
