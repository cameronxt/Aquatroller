
#include "ph.h"

PH::PH() {

}

void PH::init() {
  // TODO: Set initial variables if we find them stored in eeprom
}

void PH::loop(unsigned long currentTime) {

  // Timer so we only read ph every so often
  if (_prevPhTime - currentTime >= phData.checkPhDelay) {
    //Serial.println(F("its time to check it"));
    readPhRawToBuffer();
    _prevPhTime = millis();                         // Reset timer to allow ph sensor to stabilize before next reading
    processPhBuffer();
    calculateRestingPh();
    
  }


}

void PH::readPhRawToBuffer() {
  _buf[_phIndex] = analogRead(_phPin);    // store anolg value into buffer position
}

void PH::processPhBuffer() {
  if (_phIndex < _bufSize) {              // if not full advance to the next index
    _phIndex++;
  } else {                                  // If full calculate average value
    for (int i = 0; i < (_bufSize - 1); i++) { // First lets sort our results
      for (int j = i + 1; j < _bufSize; j++) {
        if (_buf[i] > _buf[j]) {
          _temp = _buf[i];
          _buf[i] = _buf[j];
          _buf[j] = _temp;
        }
      }
    }

    _avgValue = 0;                                       // Reset average value
    for (int i = 1; i < _bufSize - 1; i++) _avgValue += _buf[i]; // Drop highest and lowest value, add the rest together

    float pHVol = (float)_avgValue * 5.0 / 1024 / 8;      // average and convert to millivolts
    _currentPh = -5.70 * pHVol + 21.34;               // convert millivolts to PH reading
    // TODO: add calibration into reading
    Serial.print("PH = ");
    Serial.println(_currentPh);
 
    _prevPhTime = millis();                              // Reset the timer and buffer index
    _phIndex = 0;
    _newPh = true;
  }
}


// Enter probe calibration mode...
// TODO: LOTS!!!!
void PH::calibratePH() {
  Serial.println(F("PH Calibration Mode"));


  Serial.println(F("Enter Calibration Target 1"));
  Serial.println(F("Waiting to stabilize..."));
  // Timer goes here
  Serial.println(F("Reading Actual 1"));


  Serial.println(F("Enter Calibration Target 2"));
  Serial.println(F("Waiting to stabilize..."));
  // Timer goes here
  Serial.println(F("Reading Actual 2"));

  setCalPoints(phData.calTarget, phData.calActual);
}

void PH::setCalPoints(float calTarget[], float calActual[]) {
  for (byte i = 0; i < 2; i++) {
    phData.calTarget[i] = calTarget[i];
    phData.calActual[i] = calActual[i];
  }
}

void PH::calculateRestingPh(){  // PH after some time of c02 off, thinking maybe set 2 hours before light on
  // Get the resting PH to know our drop in PH with c02
  if (_newPh && _needRestingPh) {
    phData.restingPh = _currentPh;
    _needRestingPh = false;
  }
}

int PH::calculateC02PPM() {
  // c02 PPM = 3*KH*10^(7-ph)
  return _currentC02PPM = 3 * phData.khHardness * pow(10,(7-_currentPh));
}

void PH::calculateTargetPh() {  // Calculate Ph target for c02 injection based on PPM
  // pH = 6.35 + log(15.664 * dKH / cO2Target)
  phData.targetPhC02 = 6.35 + log(12.839 * phData.khHardness / phData.targetPPMC02);
}
