// TODO: Rework lights so that we can have custom fade maps
// TODO: Add a moonlights function, basically a second timer that runs from a different fade map
// TODO: Add a timed override mode so we can bypass current settings for a set period
// TODO: adjust fade if powered on in the middle of a cycle

#ifndef lights_h
#define lights_h

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>   // https://github.com/pkourany/Adafruit_PWMServoDriver_IDE 
#include <DS3232RTC.h>                 // https://github.com/JChristensen/DS3232RTC
#include "RTClib.h"
#include <TimeLib.h>

enum LightModes {NORMAL, SUNLIGHT, CUSTOM, TEST};

struct Channel {

};

struct LightData {
  unsigned long testFadeDelay;
  unsigned long normalFadeDelay;
  byte mode = NORMAL;
  unsigned long onTime = 16UL*60*60;
  unsigned long offTime= 23UL*60*60;
  int normalMaxBright = (float)4096 * 0.8;
};

class Light {
    Adafruit_PWMServoDriver *_pwm;
    RTC_DS3231 *_rtc;
    LightData _data;

  public:
    Light(Adafruit_PWMServoDriver *pwm, RTC_DS3231 *rtc);

    int _targetBright;
    int _normalCurrentBright;

    void init();
    void loop(unsigned long ssm);
    
    int getDataAddress() { return &_data; };        // Used to return pointer to the data for eeprom purposes
    
    void normalLights(unsigned long ssm);
    void testLights(unsigned long ssm);
    void setMode(byte mode);
    byte getMode();

    // In Progress still
    void setBright(int targetBright, byte ch);
    int getBright(byte ch);
    void setOnTime(unsigned long timeOn);
    unsigned long getOnTime();
    void setOffTime(unsigned long timeOff);
    unsigned long getOffTime(byte ch);
    bool isLightTime(unsigned long ssm);
};

#endif
