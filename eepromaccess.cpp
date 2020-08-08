//read eeprom as a structure
//write to eeprom as a structure
#include "eepromaccess.h"
EepromAccess::EepromAccess(PHData *phData, LightData *lightData): _phData(phData), _lightData(lightData)
{}

void EepromAccess::saveMap() {         // Write Settings to EEPROM
  EEPROM.put(settingsAddressEEPROM, _eepromMap);
}

EMap EepromAccess::getMap() {       // Read Settings From EEPROM
  EMap eMap;
  EEPROM.get(settingsAddressEEPROM, eMap);
  return eMap;
}


bool EepromAccess::checkForSettings (EMap checkMap) {

  if (strstr(checkMap.VERSION, _eepromMap.VERSION)) {         // If previous version is found, then true
    Serial.println(F("Previous config found..."));
    return true;
  } else {                                        // else false
    Serial.println(F("No config found..."));
    return false;
  }
}

// TODO: Check for settings in eeprom and load into memory if there
void EepromAccess::setup() {                      // Check for previous config, if not found generate new one

  EMap tempMap = getMap();;                                      // New Temporary EEPROM map
  //getMap();                                  // Read settings from eeprom

  if (checkForSettings(tempMap))  {                      // If the map from EEPROM contains previous data
    Serial.println(F("Loading Settings from EEPROM"));
    //_eepromMap = getMap();                                  // Load it

    getSettings();

    //getSettings(*_lightData);      // Read Settings From EEPROM

  } else {                                                // Otherwise, config as defaults
    Serial.print(F("Generating new config..."));
    _eepromMap = EMap();                                   // Create default structure
    updateSettings();                             // Sets everything to default values
    Serial.println(F(" Done"));
  }
}


void EepromAccess::updateSettings() {           // Updates(not write) eeprom with new values
  EEPROM.put(settingsAddressEEPROM, _eepromMap);
  EEPROM.put(phAddressEEPROM, *_phData );
  EEPROM.put(lightAddressEEPROM, _lightData);
}

void EepromAccess::getSettings() {
  
  EEPROM.get(settingsAddressEEPROM, _eepromMap);
  EEPROM.get(phAddressEEPROM, *_phData );
  EEPROM.get(lightAddressEEPROM, _lightData);
}

void EepromAccess::loop() {
  // Timer to reduce writes to eeprom
  // compare eeprom to current values
  if (_needsUpdated) {
    updateSettings();
    _needsUpdated = false;
  }

}


void EepromAccess::updateSelectEeprom(EEPROM_SELECT selection) {
  switch (selection) {
    case ALL:
      updateSettings();
      break;
    case EEPROM_SETTINGS:
      EEPROM.put(settingsAddressEEPROM, _eepromMap);
      break;

    case PH_SETTINGS:
      EEPROM.put(phAddressEEPROM, *_phData );
      break;

    case LIGHT_SETTINGS:
      EEPROM.put(lightAddressEEPROM, *_lightData);
      break;

  }
}


void EepromAccess::getSelectEeprom(EEPROM_SELECT selection) {
  switch (selection) {
    case ALL:
      getSettings();
      break;
    case EEPROM_SETTINGS:
      EEPROM.get(settingsAddressEEPROM, _eepromMap);
      break;

    case PH_SETTINGS:
      EEPROM.get(phAddressEEPROM, *_phData );
      break;

    case LIGHT_SETTINGS:
      EEPROM.get(lightAddressEEPROM, *_lightData);
      break;

  }
}
