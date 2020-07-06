#ifndef temp_h
#define temp_h
#include "Arduino.h"
#include <OneWire.h>
#include<DallasTemperature.h>

const int tempPin = 2;
const int heaterPin = 5;

struct TemperatureData {
  unsigned long heaterDelayTime = 30000; // checks temp for heater every minute
  unsigned long tempDelayTime = 1000;     // How long to wait before getting temp again
  float targetTemp = 82.0;               // Our desired temperature
};

class Temp {
  TemperatureData tempData;
  unsigned long prevTempTime;
  unsigned long prevConversionTime;
  unsigned long conversionTime = 750;
  unsigned long prevHeaterTime;
  float temps[2] = {0, 0};
  bool waitingToCheck = false;
  bool isHeaterOn = false;

  public:
    Temp(DallasTemperature* _temp);
    void init();
    void loop(unsigned long currentTime);
    unsigned long getHeaterDelay();
    void setHeaterDelay(unsigned long newDelay);
    unsigned long getTempDelay();
    void setTempDelay(unsigned long newDelay);
    float getTargetTemp();
    void setTargetTemp(float newTemp);

  private:
  OneWire* _wire;
  DallasTemperature* _temp;
    void getTemps(unsigned long currentTime);
    void turnHeaterOn();
    void turnHeaterOff();

};
#endif
