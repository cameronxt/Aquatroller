#include "temp.h"

Temp::Temp(DallasTemperature* tempSensors) {
  _temp = tempSensors;
}

void Temp::init() {
  _temp->begin();
  _temp->setWaitForConversion("FALSE");
  pinMode(heaterPin, OUTPUT);
  digitalWrite(heaterPin, HIGH);  // Turn it off
}

void Temp::loop(unsigned long currentTime) {
  getTemps(currentTime);

  if (currentTime - prevHeaterTime >= tempData.heaterDelayTime) {

    //    Serial.println("Checking Heater");
    prevHeaterTime = currentTime;
    if ((temps[0] <= tempData.targetTemp - 0.5) && (temps[1] <= tempData.targetTemp - 0.5)) {
      //TurnHeaterOn
      if (!isHeaterOn) {
        turnHeaterOn();
      }
    } else if ((temps[0] >= tempData.targetTemp) || (temps[1] >= tempData.targetTemp)) {
      // Turn Heater Off
      if (isHeaterOn) {
        turnHeaterOff();
      }
    }
  }
}

void Temp::getTemps(unsigned long currentTime) {
  if (currentTime - prevTempTime > tempData.tempDelayTime) {
    //    Serial.println("Requesting Conversion");
    _temp->requestTemperatures();
    waitingToCheck = true;
    prevTempTime = millis();
    conversionTime = millis();
  }
  if ((currentTime - prevConversionTime >= conversionTime) && (waitingToCheck)) {
    waitingToCheck = false;
    temps[0] = _temp->getTempFByIndex(0);
    temps[1] = _temp->getTempFByIndex(1);
    Serial.print("Temp 1: ");
    Serial.println(temps[0]);
    Serial.print("Temp 2:");
    Serial.println(temps[1]);
  }
}

void Temp::turnHeaterOn() {
  if (!isHeaterOn) {
    Serial.println("Turning heater ON");
    digitalWrite(heaterPin, LOW);
    isHeaterOn = true;
  }
}

void Temp::turnHeaterOff() {
  if (isHeaterOn) {
    Serial.println("Turning heater OFF");
    digitalWrite(heaterPin, HIGH);
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
