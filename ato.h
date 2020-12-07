#ifndef ato_h
#define ato_h
#include "Arduino.h"

struct ATOData {
  // Delays are all milliseconds
  unsigned long levelCheckDelay = 6*(1000*60*60);
  unsigned long levelCheckDelayWhileFilling = 10000;

  unsigned long alarmCheckDelay = 1*(1000*60);
  unsigned long alarmCheckDelayWhileFilling = 1000;

  unsigned long valveToggleDelay = 1000;    // delay in ms

  int opticalSwitchThreshold = 400;   // reading in mv

  int opticalSwitchPin = A1;
  int alarmSwitchPin = 3;
  int waterValvePin = 8;
};


class AutoTopOff{

  public:
  AutoTopOff();
  void init();
  void loop();   // Main Loop
  
  bool needsFilled();   // Check fluid level, returns true when below fill level
  bool isAlarm();   // Checks for alarm condiion, if alram return true

  // Turns ATO on/off respectively
  void enableATOValve();
  void disableATOValve();

  bool readOpticalSwitch();
  bool readAlarmSwitch();
  bool isValveOpen();

  bool checkForAlarm();
  bool checkOpticalSwitch();
  ///////// Getters //////////////
  unsigned long getLevelCheckDelay();
  unsigned long getLevelCheckDelayWhileFilling();

  unsigned long getAlarmCheckDelay();
  unsigned long getAlarmCheckDelayWhileFilling();

  unsigned long getValveToggleDelay();    // delay in ms

  int getOpticalSwitchThreshold();   // reading in mv

  int getOpticalSwitchPin();
  int getAlarmSwitchPin();
  int getWaterValvePin();
  ///////// Setters ////////////////
  void setLevelCheckDelay(unsigned long levelCheckDelay);
  void setLevelCheckDelayWhileFilling(unsigned long levelCheckFillingDelay);

  void setAlarmCheckDelay(unsigned long alarmDelay);
  void setAlarmCheckDelayWhileFilling(unsigned long alarmFillingDelay);

  void setValveToggleDelay(unsigned long toggleDelay);    // delay in ms

  void setOpticalSwitchThreshold(int threshold);   // reading in mv

  void setOpticalSwitchPin(int pin);
  void setAlarmSwitchPin(int pin);
  void setWaterValvePin(int pin);

  private:
  
  ATOData _data;

  bool _updateData=false;
  bool _isFilling = false;
  bool _overfillAlarm = false;
  unsigned long _prevValveTime = 0;
  unsigned long _prevOpticalTime = 0;
  unsigned long _prevAlarmCheckTime = 0;
  

  
};
#endif
