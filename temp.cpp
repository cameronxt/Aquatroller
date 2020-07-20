#include "temp.h"

Temp::Temp(DallasTemperature* tempSensors) {
  _temp = tempSensors;    // Pointer to our temp sensors
}

void Temp::init() {
  _temp->begin();                           // Initialize comms with temp sensors
  _temp->setWaitForConversion("FALSE");     // We dont want to wait, it blocks us, so we will time it ourselves
  pinMode(heaterPin, OUTPUT);               // Setup heater pin as output
  digitalWrite(heaterPin, HIGH);            // Turn it off  
}

void Temp::loop(unsigned long currentTime) {
  getTemps(currentTime);    // Non blocking temp request

  if (currentTime - prevHeaterTime >= tempData.heaterDelayTime) {         // If it has been past the heater delay time

    //    Serial.println("Checking Heater");
    prevHeaterTime = currentTime;                                                                   // reset heater timer
    if ((temps[0] <= tempData.targetTemp - 0.5) && (temps[1] <= tempData.targetTemp - 0.5)) {       // if temp is low
      //TurnHeaterOn
      if (!isHeaterOn) {                                                                            // and heater is off
        turnHeaterOn();                                                                             // then turn it on
      }
    } else if ((temps[0] >= tempData.targetTemp) || (temps[1] >= tempData.targetTemp)) {            // if temp is to high
      // Turn Heater Off
      if (isHeaterOn) {                                                                             // and heater is on
        turnHeaterOff();                                                                            // Then Turn it off
      }
    }
  }
}

void Temp::getTemps(unsigned long currentTime) {
  if (currentTime - prevTempTime > tempData.tempDelayTime) {      // Request new temp after set delay
    //    Serial.println("Requesting Conversion");
    _temp->requestTemperatures();                                 // actual temp request
    waitingToCheck = true;                                        // true so we know there is a conversion happening
    prevTempTime = millis();                                      // reset temp delay timer
    conversionTime = millis();                                    // start Conversion timer
  }
  if ((currentTime - prevConversionTime >= conversionTime) && (waitingToCheck)) {       // If we are waiting on a conversion and it has been long enough
    waitingToCheck = false;                                                             // Reset conversion check flag
    temps[0] = _temp->getTempFByIndex(0);                                               // Read first sensor on the wire
    temps[1] = _temp->getTempFByIndex(1);                                               // Read second sensor on the wire
    Serial.print("Temp 1: ");
    Serial.println(temps[0]);
    Serial.print("Temp 2:");
    Serial.println(temps[1]);
  }
}

void Temp::turnHeaterOn() {
  if (!isHeaterOn) {                                  // Only Turn it on if its off
    Serial.println("Turning heater ON");
    digitalWrite(heaterPin, LOW);                     // Turn on heater
    isHeaterOn = true;                                
  }
}

void Temp::turnHeaterOff() {
  if (isHeaterOn) {                                   // Only turn it off if its on
    Serial.println("Turning heater OFF");
    digitalWrite(heaterPin, HIGH);                    // Turn off heater
    isHeaterOn = false;
  }
}

unsigned long Temp::getHeaterDelay(){
  return tempData.heaterDelayTime;
}

void Temp::setHeaterDelay(unsigned long newDelay) {
  tempData.heaterDelayTime = newDelay;
}

unsigned long Temp::getTempDelay(){
  return tempData.tempDelayTime;
}

void Temp::setTempDelay(unsigned long newDelay){
  tempData.tempDelayTime = newDelay;
}

float Temp::getTargetTemp(){
  return tempData.targetTemp;
}

void Temp::setTargetTemp(float newTemp){
  tempData.targetTemp = newTemp;
}
