// save a copy of eeprom settings
// create generic log creator
// delete log entries after upload or after certain time has passed
#include "sdaccess.h"

void SDAccess::init() {
  Serial.print(F("Initializing SD Card..."));
  // Initialize the SD card
  if (SD.begin()) {
    enableLogs = true;
    Serial.println(F(" Done"));
  } else {
    enableLogs = false;
    Serial.println(F(" None Found"));
  }

}


void SDAccess::logData() {

}

void SDAccess::readData() {

}
