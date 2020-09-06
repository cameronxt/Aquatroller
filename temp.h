#ifndef temp_h
#define temp_h
#include "Arduino.h"
#include <OneWire.h>
#include<DallasTemperature.h>

const int tempPin = 2;
const int heaterPin = 4;

struct TemperatureData {
  
  float targetTemp = 82.0;             // Our desired temperature
  unsigned int heaterDelayTime = 60;   // checks temp every X seconds
  unsigned int tempDelayTime = 10;     // How long to wait before getting temp again in seconds
};

class Temp {
    TemperatureData tempData;
    unsigned long prevTempTime;               // Timer to only check temp every so often
    unsigned long prevConversionTime;         // Timer for conversion times
    unsigned long conversionTime = 750;       // Amount of time to wait for conversion after request in milliseconds
    unsigned long prevHeaterTime;             // Timer to control how often the heater can be cycled
    float temps[2] = {0.0, 0.0};                  // Most recent available temps
    bool waitingToCheck = false;              // Flag for when we are waiting to check after conversion
    bool isHeaterOn = false;                  // Flag to track state of heater

  public:
    Temp(DallasTemperature* _temp);
    void init();
    void loop(unsigned long currentTime);
    
    float getCurrentTemp() { return ( (temps[0] + temps[1]) / 2 );};

    float getTargetTemp();
    void setTargetTemp(float newTemp);
    
    unsigned int getHeaterDelay();
    void setHeaterDelay(unsigned int newDelay);
    
    unsigned int getTempDelay();
    void setTempDelay(unsigned int newDelay);
    
    TemperatureData* getDataAddress() { return &tempData; };

  private:
    OneWire* _wire;
    DallasTemperature* _temp;
    void getTemps(unsigned long currentTime);
    void turnHeaterOn();
    void turnHeaterOff();

};
#endif
