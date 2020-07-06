//read eeprom as a structure
//write to eeprom as a structure
#include "eepromaccess.h"

void EepromAccess::saveSettings() {         // Write Settings to EEPROM
  EEPROM_write(0, eepromMAP);
}

void EepromAccess::getSettings(Settings savedMap) {       // Read Settings From EEPROM
  EEPROM_read(0, savedMap);
}



bool EepromAccess::checkForSettings (Settings &checkMap) {
  if (strstr(checkMap.VERSION, "AA1")) {         // If previous version is found, then true
    Serial.println(F("Previous config found..."));
    return true;
  } else {                                        // else false
    Serial.println(F("No config found..."));
    return false;
  }
}
void EepromAccess::init() {                      // Check for previous config, if not found generate new one
  getSettings(eepromMAP);
  if (checkForSettings(eepromMAP))  {
    Serial.println(F("Settings loaded from EEPROM"));
  } else {
    Serial.print(F("Generating new config file..."));
    Settings eeprom;                                         // Create default structure
    saveSettings();                                    // Sets everything to default values
    Serial.println(F(" Done"));
  }
}

void EepromAccess::updateSettings() {           // Updates(not write) eeprom with new values
  if (settingsChanged) {
    Serial.print(F("Saving changed settings..."));
    saveSettings();
    Serial.println(F(" Done"));
  } else {
    Serial.println(F("No changes to EEPROM"));
  }
}

void EepromAccess::loop(){
  
}
