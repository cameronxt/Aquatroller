
#include "ph.h"

// PH Constructor
PH::PH() {

}

// Initialization function
void PH::init() {
  // TODO: Set initial variables if we find them stored in eeprom

  //    setRestingPh(float restingPh);
  //
  //    setC02PhTarget(float c02PhTarget);
  //
  //    setTargetPhDrop(float targetPhC02);
  //
  //    setTargetPPMC02 (int targetPPM);
  //
  //    setKhHardness (float khHardness);
  //
  //    setCalPoints(float calTarget[], float calActual[]);
}

/* Loop function, place in your main loop() of your sketch.
   This is non blocking, using timers and flags. All it needs from
   your main sketch is the current time from millis() to run the
   timer. This loop will check the ph every so often (configurable)
   and add it to a buffer (size configurable). Once the buffer is
   full we will process the data in the buffer. We sort all the
   values from lowest to highest, then drop the highest and lowest
   values. Then we average the remaining values to come to our
   final output PH. Based on this PH, we decide if we need to turn
   on c02 or turn it off.
*/
void PH::loop(unsigned long currentTime) {

  // Timer so we only read ph every so often
  if (_prevPhTime - currentTime >= phData.checkPhDelay) {

    //Serial.println(F("its time to check it"));
    if (!_calibrationMode) {                        // Dont read ph during calibration so probe stabilize without interrupt
      readPhRawToBuffer();
      _prevPhTime = millis();                         // update PH timer to allow sensor to stabilize before next reading
      processPhBuffer();
    }

  }

  // If resting PH is requested
  if (_needRestingPh && _newPh) {
    calculateRestingPh();
  }

}

// Reads the raw sensor value and stores it in the buffer, then advance buffwe for next reading
void PH::readPhRawToBuffer() {
  if (_phIndex < _bufSize) {                // Bounds checking
    _buf[_phIndex] = analogRead(_phPin);    // store anolg value into buffer position
    _phIndex++;                             // Advance buffer to next position
  }
}

// Called when buffer is full.
void PH::processPhBuffer() {

  if (_phIndex == _bufSize) {                    // if buffer is full, lets calculate average value
    for (int i = 0; i < (_bufSize - 1); i++) {   // First lets sort our results
      for (int j = i + 1; j < _bufSize; j++) {
        if (_buf[i] > _buf[j]) {
          _temp = _buf[i];
          _buf[i] = _buf[j];
          _buf[j] = _temp;
        }

      }

      _avgValue = 0;                                               // Reset average value
      for (int i = 1; i < _bufSize - 1; i++) _avgValue += _buf[i]; // Drop highest and lowest value, add the rest together

      float pHVol = (float)_avgValue * 5.0 / 1024 / 8;        // average and convert to milli-volts
      _currentPh = -5.70 * pHVol + 21.34;                     // convert millivolts to PH reading
      
      // TODO: add calibration into reading
      
      //Serial.print("PH = ");
      //Serial.println(_currentPh);

      _prevPhTime = millis();                              // Reset the timer and buffer index
      _phIndex = 0;                                        // Reset Buffer
      _newPh = true;                                       // Set new PH Flag
    }
  }
}


// Enter probe calibration mode...
// TODO: LOTS!!!!
void PH::calibratePH(unsigned long currentTime, float* target) {

  static bool haveFirstPoint = false;           // flag so we know if we are on first or second cal point
  static bool pointsReady = false;              // Do we have both calibration points
  static float actual[2];                       // Temp cal data

  if (_calibrationMode) {
    //Serial.println(F("PH Calibration Mode"));



    Serial.println(F("Enter Calibration Targets"));
    Serial.println(F("Waiting to stabilize..."));

    if (_prevPhTime - currentTime >= phData.phStabilizeDelay) {       // Timer to wait for stabilization period before reading ph

      if (!haveFirstPoint) {                               // Do we need the first actual cal point
        
        Serial.println(F("Reading Actual 1"));
        _phIndex = 0;             // reset buffer index
        readPhRawToBuffer();
        actual[0] = (float)_buf[0] * 5.0 / 1024;     // Convert to mv and store
        _prevPhTime == millis();  // reset PH timer

      } else {

        Serial.println(F("Reading Actual 2"));
        readPhRawToBuffer();
        actual[1] = (float)_buf[0] * 5.0 / 1024;       // Convert to mv and store
        pointsReady = true;       // we have both points
        _prevPhTime = millis();   // reset PH timer

      }
    }


    if (pointsReady) {                                // If we have both points
      setCalPoints(target, actual);
      haveFirstPoint = false;
      pointsReady = false;
      _calibrationMode = false;
    }
  }
}

// Set calibration points

//     PH::setCalPoints(calPoints[] , calReadVal[]);
void PH::setCalPoints(float * calTarget, float * calActual) {
  for (byte i = 0; i < 2; i++) {
    phData.calTarget[i] = calTarget[i];
    phData.calActual[i] = calActual[i];
  }
}

// Set resting PH as the current PH, only call once you know the C02 has offgassed, a
// couple hours after c02 off should be plenty but i havent done any real life testing yet
void PH::calculateRestingPh() {
  // Get the resting PH to know our drop in PH with c02 and reset our flag
  phData.restingPh = _currentPh;
  _needRestingPh = false;
}

// TODO: Testing to decide which of the following methods is more reliable

// Calculate the c02 in PPM that is in the water based on current PH and KH hardness
int PH::calculateC02PPM() {
  // c02 PPM = 3*KH*10^(7-ph)
  return _currentC02PPM = 3 * phData.khHardness * pow(10, (7 - _currentPh));
}

// Calculate what PH should be to achieve desired PPM of c02
void PH::calculateTargetPh() {
  // pH = 6.35 + log(15.664 * dKH / cO2Target)
  phData.targetPhC02 = 6.35 + log(12.839 * phData.khHardness / phData.targetPPMC02);
}
