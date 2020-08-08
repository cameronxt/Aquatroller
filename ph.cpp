#include "ph.h"

// PH Constructor
PH::PH(byte phInputPin, byte c02OutputPin, RTC_DS3231 *rtc) :
  _phPin(phInputPin),   // PH Sensor Analog pin
  _c02Pin (c02OutputPin),   // C02 relay pin
  _rtc(rtc)
{
 // _phData.targetPh = 7.0;
}

// Initialization function
void PH::setup() {
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

  
  // pinModes and whatnot
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
// TODO: C02 Control && Getting resting PH
void PH::loop(unsigned long ssm) {

  // Timer so we only read ph every so often
  if (millis() - _prevPhTime >= _phData.checkPhDelay) {

    //Serial.println(F("its time to check it"));
    if (!_calibrationMode) {                        // Dont read ph during calibration so probe stabilize without interrupt
      readPhRawToBuffer();                          // read raw data and store it to an array
      _prevPhTime = millis();                       // update PH timer to allow sensor to stabilize before next reading
      processPhBuffer();                            // Average and calculate new PH value
    } else {
      calibratePH();
    }

  }

  // If resting PH is requested, either because its time or from a serial event (like bluetooth)
  if (ssm > (_phData.c02OffTime + _phData.c02OffgasDelay )) {
    _needRestingPh = true;
  }
  if (_needRestingPh && _newPh) {
    calculateRestingPh();
  }


  // TODO: C02 Control
  if ((ssm > _phData.c02OnTime) && (ssm < _phData.c02OffTime)) {   // Is it time for the c02 to be on
    if (millis() - _prevC02Time >= _phData.checkC02Delay) {        // Timer for c02 control, keeps from cycling relay to quickly
      if (_currentPh > _phData.targetPh) {                                // PH is higher than target
        turnOnC02();                                               // turn on c02
      } else {
        turnOffC02();                                              // otherwise turn it off
      }
      _prevC02Time = millis();                                     // Reset timer
    }
  } else {                                                         // If its not in the time window, make sure c02 is off
    turnOffC02();
  }

}

// Reads the raw sensor value and stores it in the buffer, then advance buffer for next reading
void PH::readPhRawToBuffer() {
  if (_phIndex >= _bufSize) {
    _phIndex=0;
  }
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
      _currentPh = -5.70 * (float)pHVol + 21.34 + (float)_phData.phCalValue;                     // convert millivolts to PH reading

      // TODO: add calibration into reading

      //Serial.print("PH = ");
      //Serial.println(_currentPh);

      _prevPhTime = millis();                              // Reset the timer and buffer index
      _newPh = true;                                       // Set new PH Flag
    }
  }
}


// TODO: TEST!!!
// Enter probe calibration mode...
// Give this method the curent time in miilis and a float array with the 2 buffer solutions target values.
// Ph monitoring will be disabled during calibration to allow the probe to get the most accurate reading possible.
// After waiting for the probe to stabilize, a reading is taken and stored, then we do both again for the second solution.
// Once we have both values we convert them to a PH value and then store both sets of targets and actuals.

void PH::calibratePH() {

  static bool haveFirstPoint = false;           // flag so we know if we are on first or second cal point

  if (_calibrationMode) {

    Serial.println(F("PH Calibration Mode..."));

    if (millis() - _prevPhTime >= _phData.phStabilizeDelay) {       // Timer to wait for stabilization period before reading ph
      int phVol = 0;
      if (!haveFirstPoint) {                               // Do we need the first actual cal point

        Serial.println(F("Reading Actual 1"));
        _phIndex = 0;
        
        readPhRawToBuffer();                                   // Read raw ph value and add to buffer
        
        phVol = (float)_buf[0] * 5.0 / 1024;                   // Convert to mv and store
        setCalActual (0, (-5.70 * phVol + 21.34));                     // convert millivolts to PH reading without calibration
        
        _prevPhTime == millis();  // reset PH timer
        haveFirstPoint = true;    // Flag the first point as done

      } else {

        Serial.println(F("Reading Actual 2"));
        readPhRawToBuffer();                                   // Reset buffer index
        phVol = (float)_buf[1] * 5.0 / 1024;                   // Convert to mv and store
        setCalActual(1,(-5.70 * phVol + 21.34));                     // convert millivolts to PH reading without calibration

        _prevPhTime = millis();        // reset PH timer 
        _phIndex = 0;                  // Reset index so we can start getting ph readings again
        
        calculateCalibration();        // Calculate Calibration Offset
        
        haveFirstPoint = false;        // Reset flag so we grab first point next time
        _calibrationMode = false;      // Reset flag to exit Calibration Mode

      }
    }
  }
}


// Set resting PH as the current PH, only call once you know the C02 has offgassed, a
// couple hours after c02 off should be plenty but i havent done any real life testing yet
void PH::calculateRestingPh() {
  // Get the resting PH to know our drop in PH with c02 and reset our flag
  _phData.restingPh = _currentPh;
  _needRestingPh = false;
}

// TODO: Testing to decide which of the following methods is more reliable

// Calculate the c02 in PPM that is in the water based on current PH and KH hardness
int PH::calculateC02PPM() {
  // c02 PPM = 3*KH*10^(7-ph)
  return _currentC02PPM = 3 * _phData.khHardness * pow(10, (7 - _currentPh));
}

// Calculate what PH should be to achieve desired PPM of c02
void PH::calculateTargetPh() {
  // pH = 6.35 + log(15.664 * dKH / cO2Target)
  _phData.targetPhC02 = 6.35 + log(12.839 * _phData.khHardness / _phData.targetPPMC02);
}

void PH::turnOnC02() {
  if (_co2On) {
    digitalWrite(_c02Pin, LOW);
  }
}

void PH::turnOffC02() {
  if (!_c02On) {
    digitalWrite(_c02Pin, HIGH);
  }
}
