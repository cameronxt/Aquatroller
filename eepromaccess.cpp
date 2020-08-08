//read eeprom as a structure
//write to eeprom as a structure
#include "eepromaccess.h"
EepromAccess::EepromAccess(PHData *phData, LightData *lightData): _phData(phData), _lightData(lightData)
{}

void EepromAccess::saveSettings(EMap* eMap) {         // Write Settings to EEPROM
  EEPROM.put(0, *eMap);
}

void EepromAccess::getSettings(EMap *savedMap) {       // Read Settings From EEPROM
  EEPROM.get(0, *savedMap);
}


bool EepromAccess::checkForSettings (EMap *checkMap) {

  if (strstr(checkMap->VERSION, _eepromMap.VERSION)) {         // If previous version is found, then true
    Serial.println(F("Previous config found..."));
    return true;
  } else {                                        // else false
    Serial.println(F("No config found..."));
    return false;
  }
}

// TODO: Check for settings in eeprom and load into memory if there
void EepromAccess::setup() {                      // Check for previous config, if not found generate new one

  EMap tempMap;                                           // New Temporary EEPROM map
  getSettings(&tempMap);                                  // Read settings from eeprom

  if (checkForSettings(&tempMap))  {                      // If the EEPROM contains previous data
    Serial.println(F("Loading Settings from EEPROM"));
    _eepromMap = tempMap;                                  // Load it

    getSelectEeprom(PH_SETTINGS);       // Read Settings From EEPROM
    getSelectEeprom(LIGHT_SETTINGS);

    //getSettings(*_lightData);      // Read Settings From EEPROM

  } else {                                                // Otherwise, config as defaults
    Serial.print(F("Generating new config..."));
    _eepromMap = EMap();                                   // Create default structure
    saveSettings(&_eepromMap);                             // Sets everything to default values
    Serial.println(F(" Done"));
  }
}


void EepromAccess::updateSettings() {           // Updates(not write) eeprom with new values
  EEPROM.put(settingsAddressEEPROM, _eepromMap);
  EEPROM.put(phAddressEEPROM, *_phData );
  EEPROM.put(lightAddressEEPROM, *_lightData);
}

void EepromAccess::getSettings() {
  EEPROM.put(settingsAddressEEPROM, _eepromMap);
  EEPROM.put(phAddressEEPROM, *_phData );
  EEPROM.put(lightAddressEEPROM, *_lightData);
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
