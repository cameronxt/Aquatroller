#ifndef eeproma_h
#define eeproma_h
#include "Arduino.h"
#include <EEPROM.h>

struct Settings {           // Struct to store all of eeprom settings

  char VERSION[4] = "AA1";     // number to check and see if storage is valid
};


template <class T> int EEPROM_write(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.update(ee++, *p++);
  return i;
}

template <class T> int EEPROM_read(int ee, const T& value)
{
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}


class EepromAccess {

  public:
    Settings eepromMAP;
    void init();
    void loop();
  private:
    bool checkForSettings (Settings &checkMap);
    void updateSettings();
    bool settingsChanged = false;
    void saveSettings();
    void getSettings (Settings savedMap);
};
#endif
