#include "ato.h"

AutoTopOff::AutoTopOff() {

}

void AutoTopOff::init() {
  pinMode(_data.opticalSwitchPin, INPUT);
  pinMode(_data.waterValvePin, OUTPUT);
}

void AutoTopOff::loop() {
  unsigned long alarmCheckDelay;
  unsigned long levelCheckDelay;
  // As long as an alarm hasnt been triggered
  if (!isAlarm()) {
    // Are we currently filling? if so use different timers
    if (isValveOpen()) {
      // Set Timer: Alarm Check Delay - Filling
      alarmCheckDelay = getAlarmCheckDelayWhileFilling();
      // Set Timer: Optical Level - filling
      levelCheckDelay = getLevelCheckDelayWhileFilling();
    } else {  // We arent filling use standard times
      // Set Timer: Alarm Check Delay - Filling
      alarmCheckDelay = getAlarmCheckDelay();
      // Set Timer: Optical Level - filling
      levelCheckDelay = getLevelCheckDelay();
    }

    // Timer to check optical sensor, delay set based on if we are filling or not
    if (millis() - _prevOpticalTime > levelCheckDelay) {
      if (checkOpticalSwitch()) {
        enableATOValve();
      } else {
        disableATOValve();
      }
    }
  } else { // Alarm is triggered
    disableATOValve();
  }
  // Timer to check alarm, delay set based on if we are filling or not - check every pass
  if (millis() - _prevAlarmCheckTime > alarmCheckDelay) {
    // Check For Alarm condition
    if (checkForAlarm()) {
      _prevAlarmCheckTime += alarmCheckDelay;
    }
  }
}


// Returns true if the alarm is active
bool AutoTopOff::isAlarm() {
  return _overfillAlarm;
}

bool AutoTopOff::checkForAlarm() {
  _overfillAlarm = digitalRead(_data.alarmSwitchPin);
  return _overfillAlarm;
}

// optical switch produces reading in mv, there is a threshold for when
// -- its no longer covered with fluid.
// Returns: True when sensor uncovered, false when covered
// Tunable: _data.opticalSwitchThreshold -- Controls the mv threshold that
// -- the sensor is considered triggered at, each switch appears to be
// -- slightly different so may need some adjustment
bool AutoTopOff::checkOpticalSwitch() {
  int switchReading = analogRead(_data.opticalSwitchPin);
  if ( switchReading < _data.opticalSwitchThreshold) {
    return true; // Needs filled
  } else {
    return false; // Doesn't need filled
  }
}

bool AutoTopOff::isValveOpen() {
  return _isFilling;
}


// Enable auto top off fill valve. This has a built in timer to prevent
// -- triggereing the solonoid to quickly, allowing everything to stabilize
// -- Only runs if the valve needs toggled
// Tunable!!!: data.valveToggleDelay: This allows you to change the toggle delay
// -- !!!Please dont change this unless you know what your doing!!!
void AutoTopOff::enableATOValve() {
  if (!isValveOpen()) {
    if (millis() - _prevValveTime > _data.valveToggleDelay) {
      digitalWrite(_data.waterValvePin, HIGH);
      _isFilling = true;
      _prevValveTime = millis();
    }
  }
}

// Disable auto top off fill valve. This has a built in timer to prevent
// -- triggereing the solonoid to quickly, allowing everything to stabilize
// -- Only runs if the valve needs toggled
// Tunable: data.valveToggleDelay: This allows you to change the toggle delay
// -- !!!Please dont change this unless you know what your doing!!!
void AutoTopOff::disableATOValve() {
  if (isValveOpen()) {
    if (millis() - _prevValveTime > _data.valveToggleDelay) {
      digitalWrite(_data.waterValvePin, LOW);
      _isFilling = false;
      _prevValveTime = millis();
    }
  }
}


unsigned long AutoTopOff::getLevelCheckDelay() {
  return _data.levelCheckDelay;
}

unsigned long AutoTopOff::getLevelCheckDelayWhileFilling(){
  return _data.levelCheckDelayWhileFilling;
}

unsigned long AutoTopOff::getAlarmCheckDelay(){
    return _data.alarmCheckDelay;
}

unsigned long AutoTopOff::getAlarmCheckDelayWhileFilling(){
    return _data.alarmCheckDelayWhileFilling;
}

unsigned long AutoTopOff::getValveToggleDelay() {
      return _data.valveToggleDelay;
}

int AutoTopOff::getOpticalSwitchThreshold(){
  return _data.opticalSwitchThreshold;
}

int AutoTopOff::getOpticalSwitchPin(){
  return _data.opticalSwitchPin;
}

int AutoTopOff::getAlarmSwitchPin(){
  return _data.alarmSwitchPin;
}

int AutoTopOff::getWaterValvePin(){
  return _data.waterValvePin;
}

void AutoTopOff::setLevelCheckDelay(unsigned long levelCheckDelay){
  _data.levelCheckDelay=levelCheckDelay;
  _updateData=true;
}

void AutoTopOff::setLevelCheckDelayWhileFilling(unsigned long levelCheckFillingDelay){
  _data.levelCheckDelayWhileFilling=levelCheckFillingDelay;
  _updateData=true;
}

void AutoTopOff::setAlarmCheckDelay(unsigned long alarmDelay){
  _data.alarmCheckDelay=alarmDelay;
  _updateData=true; 
}

void AutoTopOff::setAlarmCheckDelayWhileFilling(unsigned long alarmFillingDelay){
  _data.alarmCheckDelayWhileFilling=alarmFillingDelay;
  _updateData=true;
}

void AutoTopOff::setValveToggleDelay(unsigned long toggleDelay){
  _data.valveToggleDelay=toggleDelay;
  _updateData=true;
}

void AutoTopOff::setOpticalSwitchThreshold(int threshold){
  _data.opticalSwitchThreshold=threshold;
  _updateData=true;
}

void AutoTopOff::setOpticalSwitchPin(int pin){
  _data.opticalSwitchPin=pin;
  _updateData=true;
}

void AutoTopOff::setAlarmSwitchPin(int pin){
  _data.alarmSwitchPin=pin;
  _updateData=true;
}

void AutoTopOff::setWaterValvePin(int pin){
  _data.waterValvePin=pin;
  _updateData=true;
}
