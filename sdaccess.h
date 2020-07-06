#ifndef sdaccess_h
#define sdaccess_h
#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

class SDAccess {
    //  File file;
    int pinCS = 10;
    bool enableLogs = true;
  public:
    void init();
    void logData();
    void readData();
  private:
};


#endif
