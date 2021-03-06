#include "temp.h"

Temp::Temp(DallasTemperature* tempSensors) {
  _temp = tempSensors;    // Pointer to our temp sensors
}

void Temp::init() {
  Serial.println(F("Initializing Temperature Controls... "));
  _temp->begin();                           // Initialize comms for sensors
  _temp->setWaitForConversion("FALSE");     // We dont want to wait, it blocks us, so we will time it ourselves
  pinMode(heaterPin, OUTPUT);               // Setup heater pin as output
  digitalWrite(heaterPin, LOW);            // Turn it off

  /////// Testing ////////////////

  Serial.print(F("Sensors Found: "));
  Serial.println(_temp->getDeviceCount(), DEC);
  ////////////////////////////////


  Serial.println(F("Done"));

}

void Temp::loop(unsigned long currentTime) {
  getTemps(currentTime);    // Non blocking temp request

  if (currentTime - prevHeaterTime >= (tempData.heaterDelayTime*1000)) {         // If it has been past the heater delay time

    //    Serial.println("Checking Heater");
    prevHeaterTime = currentTime;                                                                   // reset heater timer
    if ((temps[0] <= tempData.targetTemp - 0.5) && (temps[1] <= tempData.targetTemp - 0.5)) {       // if temp is low
      turnHeaterOn();                                                                             // then turn it on
    } else if ((temps[0] > tempData.targetTemp) || (temps[1] > tempData.targetTemp)) {            // if temp is to high
      turnHeaterOff();                                                                            // Then Turn it off
    }
  }
}

void Temp::getTemps(unsigned long currentTime) {

  if (currentTime - prevTempTime > (tempData.tempDelayTime*1000)) {      // Request new temp after set delay
    // Serial.println(F("Requesting Conversion"));
    _temp->requestTemperatures();                                 // actual temp request
    waitingToCheck = true;                                        // true so we know there is a conversion happening
    prevTempTime = millis();                                      // reset temp delay timer
  }

  if ((currentTime - prevConversionTime >= conversionDelayTime) && (waitingToCheck)) {       // If we are waiting on a conversion and it has been long enough
    waitingToCheck = false;                                                             // Reset conversion check flag
    temps[0] = _temp->getTempFByIndex(0);                                               // Read first sensor on the wire
    temps[1] = _temp->getTempFByIndex(1);                                               // Read second sensor on the wire
    prevConversionTime += conversionDelayTime;          // start Conversion timer

    
//    if (temps[0] == -196.0) {
//      Serial.println(F("Temp 1: ERROR"));
//    } else {
//      Serial.print("Temp 1: ");
//      Serial.println(temps[0]);
//    }
//
//    if (temps[1] == -196.0) {
//      Serial.println(F("Temp 2: ERROR"));
//    } else {
//      Serial.print("Temp 2:");
//      Serial.println(temps[1]);
//    }
  }
}

void Temp::turnHeaterOn() {
  if (!isHeaterOn) {                                  // Only Turn it on if its off
    Serial.println("Turning heater ON");
    digitalWrite(heaterPin, HIGH);                     // Turn on heater
    isHeaterOn = true;
  }
}

void Temp::turnHeaterOff() {
  if (isHeaterOn) {                                   // Only turn it off if its on
    Serial.println("Turning heater OFF");
    digitalWrite(heaterPin, LOW);                    // Turn off heater
    isHeaterOn = false;
  }
}

// Getter for current heater change delay (heaterDelayTime)
unsigned int Temp::getHeaterDelay() {
  return tempData.heaterDelayTime;
}

// Setter for current heater change delay (heaterDelayTime)
void Temp::setHeaterDelay(unsigned int newDelay) {
  tempData.heaterDelayTime = newDelay;
}

// Getter for current temp check delay (tempDelayTime)
unsigned int Temp::getTempDelay() {
  return tempData.tempDelayTime;
}

// Setter for current temp delay (tempDelayTime)
void Temp::setTempDelay(unsigned int newDelay) {
  tempData.tempDelayTime = newDelay;
}

// Getter for current target temperature (targetTemp)
float Temp::getTargetTemp() {
  return tempData.targetTemp;
}

// Setter for current target temperature (targetTemp)
void Temp::setTargetTemp(float newTemp) {
  tempData.targetTemp = newTemp;
}
