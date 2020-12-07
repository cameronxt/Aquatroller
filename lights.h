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

const byte _mapSize = 5;
const byte _numChannels = 6;

struct Channel {
  byte stepTimeHours[_mapSize] =   {16, 17, 20, 21, 22};
  byte stepTimeMinutes[_mapSize] = {45, 30, 00, 30, 15};
  int brightness[_mapSize] =       {20, 80, 100, 70, 20};
  int moonlightBright = 5;
  int numberOfSteps[_mapSize + 1];      // Calculated based on brightness change between steps
  unsigned long fadeDelay[_mapSize + 1]; //  Calculated based on supplied times and brightness
  int stepCount;
  int currentBright;
  unsigned long prevFadeTime;
  byte fadeIndex;
  int resumeFadeDelay = 2000;   // Time to wait between fades when resume mid cycle
};

struct LightData {

  Channel ch[_numChannels];

  unsigned long testFadeDelay;       // RAM Candidate - Can be calculated
  unsigned long normalFadeDelay;     // RAM Candidate - Can be calculated
  byte onTimeHours = 14;                 // First 2 digits are hours, second are minutes
  byte onTimeMinutes = 00;
  byte offTimeHours = 23;                // First 2 digits are hours, second are minutes
  byte offTimeMinutes = 00;
  byte mode = CUSTOM;

};

class Light {

  public:
    Light(Adafruit_PWMServoDriver *pwm, DS3232RTC *rtc);


    void init();
    void loop(unsigned long ssm);

    int getDataAddress() {
      return &_data;
    };

    void normalLights();
    void customLights();
    void testLights();

    void setMode(byte mode);
    byte getMode();

    void setBright(int targetBright, byte ch);
    int getBright(byte ch);

    void setOnTime(time_t onTime);
    time_t getOnTime();

    void setOffTime(time_t offTime);
    time_t getOffTime();


    bool isLightTime();

    void calcFadeMap();
    void calcStartStep();

    time_t intTimesToTime_t (int hours, int minutes);
    time_t byteTimesToTime_t (byte hours, byte minutes);
  private:

    Adafruit_PWMServoDriver *_pwm;
    DS3232RTC *_rtc;
    LightData _data;

    const int _normalMaxBright = 4096;
    int _targetBright;
    int _normalCurrentBright;
    int _currentBright;
    int _prevFadeTime[_mapSize];
    byte _prevDay;
    time_t _onTime;
    time_t _offTime;
    time_t _currentTime;


};

#endif
