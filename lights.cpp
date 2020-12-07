#include "lights.h"

Light::Light(Adafruit_PWMServoDriver *pwm, DS3232RTC *rtc) {
  _pwm = pwm;
  _rtc = rtc;
  _data.normalFadeDelay = 5;
  _data.testFadeDelay = 5;
  _targetBright = _normalMaxBright;
  for (byte i; i < _numChannels; i++) {
    _data.ch[i].prevFadeTime = 0;
  }
}

void Light::init() {
  Wire.setClock(400000);
  _pwm->begin();
  // _pwm.setOutputMode(false);           // Set PWM Driver to open Drain for LEDs
  _pwm->setPWMFreq(1000);                // Set PWM Hz to reduce buzzing sound and Meanwell LLD's have a max of 1kHz
  for (int i = 0; i < 6; i++) {
    _pwm->setPin(i, 0, 0); // turn all channels off
  }

  _currentTime = now();

  // Initialize Variables
  _onTime = intTimesToTime_t(_data.onTimeHours, _data.onTimeMinutes); // Convert on time from to time_t from 2 bytes
  _offTime =  intTimesToTime_t(_data.offTimeHours, _data.offTimeMinutes);  // Convert off time from to time_t from 2 bytes
  _prevDay = day();   // Set daily timer

  calcStartStep();
  calcFadeMap();  // Initialize the fade map

  if ( isLightTime() ) {
    Serial.println();
    Serial.println(F("Light Should be on..."));
    Serial.println(F("Setting fadeDelay to bring light to proper brightness"));
    Serial.println();

    for (byte i = 0; i < _numChannels; i++) {

      Serial.print(F("Ch #"));
      Serial.print(i);
      Serial.print(F(" set to "));

       unsigned long timeDiff = 1000ul * abs( (  _currentTime - intTimesToTime_t(_data.ch[i].stepTimeHours[_data.ch[i].fadeIndex], _data.ch[i].stepTimeMinutes[_data.ch[i].fadeIndex])));  // Calculate time difference in millis
       unsigned long brightDiff = abs(_data.ch[i].brightness[_data.ch[i].fadeIndex] - _data.ch[i].currentBright); // calculate total amount of change in brightness

      _data.ch[i].fadeDelay[_data.ch[i].fadeIndex] = timeDiff / brightDiff;

      Serial.println(_data.ch[i].fadeDelay[_data.ch[i].fadeIndex]);
      Serial.println();

    }
  }


}

// Main loop function.
// Calls light control based on time and _data.mode
void Light::loop(time_t currentTime) {

  _currentTime = currentTime;  // Update time in Library
  if (isLightTime()) {
    switch (_data.mode) {
      case NORMAL:
        normalLights();
        break;
      case SUNLIGHT:
        break;
      case CUSTOM:
        customLights();
        //Serial.println(F("Custom Lights"));
        break;
      case TEST:
        testLights();
        break;


    }
  } else {  // Run the moonlights
    //Serial.println(F("Its moonlight time"));
    for (int i = 0; i < _numChannels; i++) {
      if (_data.ch[i].currentBright != _data.ch[i].moonlightBright) {
        _data.ch[i].currentBright = _data.ch[i].moonlightBright;
        _pwm->setPin(i, _data.ch[i].currentBright, 0);

        Serial.print(F("Moonligthts Ch#"));
        Serial.print(i);
        Serial.print(F(" set brightness to: "));
        Serial.println(_data.ch[i].currentBright);
      }
    }

  }

  // Reset fadeMap at midnight
  if (day() != _prevDay)  { // Once a day timer
    calcFadeMap();
    _prevDay = day(); // Reset timer
  }
}

// Turns 2 ints (one for hours one for minutes) into a time_t of today
// Need to remove in favor of byte storage
time_t Light::intTimesToTime_t (int hoursNow, int minutesNow) {
  return ((hoursNow * SECS_PER_HOUR) + (minutesNow * SECS_PER_MIN) + (previousMidnight(_currentTime)));
}

// Turns 2 byte (one for hours one for minutes) into a time_t of today
time_t Light::byteTimesToTime_t (byte hoursNow, byte minutesNow) {
  return ((hoursNow * SECS_PER_HOUR) + (minutesNow * SECS_PER_MIN) + (previousMidnight(_currentTime)));
}

// Make work even if time passes midnight
bool Light::isLightTime() {
  time_t onTime = byteTimesToTime_t(_data.onTimeHours, _data.onTimeMinutes);
  time_t offTime = byteTimesToTime_t(_data.offTimeHours, _data.offTimeMinutes);
  //    Serial.print ("Current Time: ");
  //    Serial.print (hour(_currentTime));
  //    Serial.print (F(":"));
  //    Serial.println(minute(_currentTime));
  //    Serial.print ("On Time: ");
  //    Serial.print (hour(onTime));
  //    Serial.print(F(":"));
  //    Serial.println(minute(onTime));
  //    Serial.print ("Off Time: ");
  //    Serial.print (hour(offTime));
  //    Serial.print(F(":"));
  //    Serial.println(minute(offTime));
  if (onTime > offTime) {
    //     Serial.println(F("On Time is MORE than Off Time"));
    time_t modOffTime = offTime + SECS_PER_DAY;
    if ((_currentTime < offTime) || (_currentTime >= onTime)) {
      //Serial.println(F("Light should be: ON"));
      return true;
    } else {
      //Serial.println(F("Light should be: OFF"));
      return false;
    }
  } else {
    //Serial.println(F("On Time is LESS than Off Time"));
    if ((_currentTime >= onTime) && (_currentTime < offTime)) {
      //Serial.println(F("Light should be: ON"));
      return true;
    } else {
      //Serial.println(F("Light should be: OFF"));
      return false;
    }
  }
  //    Serial.println(_currentTime);
  //    Serial.println(onTime);
  //    Serial.println(offTime);
}

// Light mode = Normal - Main loop method
void Light::normalLights() {
  static unsigned long prevMillis;

  if (millis() - prevMillis > _data.normalFadeDelay) {     // Wait for delay
    //Serial.println("Light Delay Exceeded");
    if (isLightTime()) {                           // if light is supposed to be on
      _targetBright = _normalMaxBright; // Automatically goes to max -
      if ((_targetBright > _normalCurrentBright) && (_normalCurrentBright < _normalMaxBright)) {             // and target bright is brighter than current

        _normalCurrentBright++;                                 // make one step brighter
        for (int i = 0; i < _numChannels; i++) {                           // set all cahnnels to new value
          _pwm->setPin(i, _normalCurrentBright, 0);
        }
      }
    } else {  // If light is supposed to be off
      _targetBright = 0; // Change Our target
      if ((_targetBright < _normalCurrentBright) && (_normalCurrentBright > 0)) { // and target brightness is lower than current bright
        _normalCurrentBright--; // make one step dimmer
        for (int i = 0; i < _numChannels; i++) {  // Set all channels to new value
          _pwm->setPin(i, _normalCurrentBright, 0);
        }
      }
    }

    prevMillis += _data.normalFadeDelay;  // reset delay timer
  }
}

// Light mode = Custom - Main loop method
void Light::customLights() {
  //Serial.println(F("Custom Light Loop"));
  //    Serial.println(F("Light Time: true"));
  unsigned long currentMillis = millis();
  // static unsigned long prevMillis = millis();

  //static bool runOnce = false;



  // Cycle through the channels
  for (int i = 0; i < _numChannels; i++ ) {

    byte fadeIndex = _data.ch[i].fadeIndex;
    //
    //              Serial.print(F("currentMillis: "));
    //              Serial.print(currentMillis);
    //              Serial.print(F(" - prevFadeTime: "));
    //              Serial.print( _data.ch[i].prevFadeTime );
    //              Serial.print(F(" - Fade Delay "));
    //              Serial.println( _data.ch[i].fadeDelay[fadeIndex] );

    if (currentMillis - _data.ch[i].prevFadeTime > _data.ch[i].fadeDelay[fadeIndex]) {  // Channel timers
      Serial.println();
      Serial.print(F("Channel #"));
      Serial.print(i);
      Serial.print(F(" - Index: "));
      Serial.println(fadeIndex );

      _data.ch[i].prevFadeTime += _data.ch[i].fadeDelay[fadeIndex]; // Reset timer

      if (_data.ch[i].currentBright < _data.ch[i].brightness[fadeIndex]) { // if brightness needs to increase
        _data.ch[i].currentBright++;
        Serial.print(F("Brightness increased to: "));
        Serial.println(_data.ch[i].currentBright);
      } else if (_data.ch[i].currentBright > _data.ch[i].brightness[fadeIndex]) {   // if brightness needs to decrease
        _data.ch[i].currentBright--;
        Serial.print(F("Brightness decreased to: "));
        Serial.println(_data.ch[i].currentBright);

        // otherwise check to see if its time to move to the next fade cycle
      } else if (_currentTime >= byteTimesToTime_t( _data.ch[i].stepTimeHours[fadeIndex], _data.ch[i].stepTimeMinutes[fadeIndex] )) {
        if ( _data.ch[i].fadeIndex <= _mapSize ) {   // Advance and reset fade index
          Serial.println(F("Incresing fadeIndex to "));
          _data.ch[i].fadeIndex++;
          Serial.println(_data.ch[i].fadeIndex);
        } else if (_currentTime >= byteTimesToTime_t( _data.offTimeHours, _data.offTimeMinutes )) {
          _data.ch[i].fadeIndex = 0;
        }
      }

      _pwm->setPin(i, _data.ch[i].currentBright, 0);  // Set brightness on PWM Driver



      Serial.print(F("Number of steps: "));
      Serial.println(_data.ch[i].numberOfSteps[fadeIndex]);

      Serial.print(F("Target Brightness: "));
      Serial.println(_data.ch[i].brightness[fadeIndex]);

      Serial.print(F("fadeDelay: "));
      Serial.println(_data.ch[i].fadeDelay[fadeIndex]);

      Serial.print(F("Channel "));
      Serial.print( i );

      Serial.print(F(" Set to "));
      Serial.println(_data.ch[i].currentBright);
      Serial.println();

    }
  }
}

// determines length of delay between each fade step
// TODO: Fade from off to current values when turned on during photo cycle

void Light::calcFadeMap() {

  // These are to test the speed of the function
  unsigned long startTime = micros();
  unsigned long endTime;


  // Debug output
  Serial.println();
  Serial.println(F("Calculating Fade Map..."));
  Serial.println();
  Serial.print(F("Current Time: "));
  Serial.println(_currentTime);
  Serial.print(F("On Time: "));
  Serial.println( byteTimesToTime_t(_data.onTimeHours, _data.onTimeMinutes) );
  Serial.print(F("Off Time: "));
  Serial.println( byteTimesToTime_t(_data.onTimeHours, _data.onTimeMinutes) );
  Serial.println();

  for (byte i = 0; i < _numChannels ; i++) {  // iterate through channels

    Serial.print(F("Channel #"));
    Serial.println(i);


    for (byte j = _data.ch[i].fadeIndex; j <= _mapSize; j++) {  // iterate through step times

      Serial.print(F("Map Step #"));
      Serial.println(j);

      unsigned long timeDiff;  // Difference between times in millis
      unsigned int brightDiff;          // Difference in brightness

      if ((j == _data.ch[i].fadeIndex) && isLightTime()) {  // If we are in this step on resume

        Serial.println(F("J >= fadeIndex && isLightTime() "));

        timeDiff = 1000ul * abs( (  _currentTime - intTimesToTime_t(_data.ch[i].stepTimeHours[j], _data.ch[i].stepTimeMinutes[j])));  // Calculate time difference in millis
        brightDiff = abs(_data.ch[i].brightness[j] - _data.ch[i].currentBright); // calculate total amount of change in brightness

      }
      else if (j == 0) {     // If index is on its first pass

        Serial.println(F("J == 0"));
        timeDiff = 1000ul * abs( (  _onTime - intTimesToTime_t(_data.ch[i].stepTimeHours[j], _data.ch[i].stepTimeMinutes[j])));  // Calculate time difference in millis
        brightDiff = abs(_data.ch[i].brightness[j] - _data.ch[i].currentBright); // calculate total amount of change in brightness

        //        Serial.print(F("Next requested brightness: "));
        //        Serial.println(_data.ch[i].brightness[j]);
        //        Serial.print(F("Previous requested brightness: "));
        //        Serial.println(_data.ch[i].moonlightBright);
        //        Serial.print(F("Step Time: "));
        //        Serial.println(intTimesToTime_t(_data.ch[i].stepTimeHours[j], _data.ch[i].stepTimeMinutes[j]));
        //        Serial.print(F("OnTime: "));
        //        Serial.println(_onTime);

      } else if (( j > 0 ) && (j < _mapSize  )) {   // If index is all but first or last
        Serial.println(F("J > 0 && j < mapSize"));
        timeDiff = 1000ul * abs(( intTimesToTime_t(_data.ch[i].stepTimeHours[j], _data.ch[i].stepTimeMinutes[j])  - intTimesToTime_t(_data.ch[i].stepTimeHours[j - 1], _data.ch[i].stepTimeMinutes[j - 1])));
        brightDiff = abs((_data.ch[i].brightness[j]) - (_data.ch[i].brightness[j - 1]));

        //        Serial.print(F("Current requested brightness: "));
        //        Serial.println(_data.ch[i].brightness[j]);
        //        Serial.print(F("Previous requested brightness: "));
        //        Serial.println(_data.ch[i].brightness[j - 1]);

      } else {

        Serial.println(F("J >= mapSize"));
        timeDiff =  1000ul * abs( (intTimesToTime_t(_data.ch[i].stepTimeHours[j - 1], _data.ch[i].stepTimeMinutes[j - 1] )) -  _offTime );
        brightDiff = _data.ch[i].brightness[j - 1 ] - _data.ch[i].moonlightBright;

        //        Serial.print(F("Current requested brightness: "));
        //        Serial.println(_data.ch[i].moonlightBright);
        //        Serial.print(F("Previous requested brightness: "));
        //        Serial.println(_data.ch[i].brightness[j - 1]);

      }
      _data.ch[i].fadeDelay[j] = timeDiff / brightDiff;
      Serial.print(F("fadeDelay: "));
      Serial.print(_data.ch[i].fadeDelay[j]);
      Serial.println();
    }
  }
  endTime = micros();
  Serial.println(F("Done Calculating Fade Map"));
  Serial.print(F("Fade Map calculation took "));
  Serial.print(endTime - startTime);
  Serial.println(F(" microseconds"));
}


// "inject" us into the correct part of fade if we turn on during On Period
void Light::calcStartStep() {

  if ( isLightTime() ) {

    bool positionFound[_numChannels] = {false, false, false, false, false, false}; // So we know when to stop searching for a channel


    // DEBUG
    Serial.println();
    Serial.println(F("Calculating Start steps..."));
    Serial.println();
    //    Serial.print(F("Current Time: "));
    //    Serial.println(_currentTime);
    //    Serial.print(F("On Time: "));
    //    Serial.println( byteTimesToTime_t(_data.onTimeHours, _data.onTimeMinutes) );
    //    Serial.print(F("Off Time: "));
    //    Serial.println( byteTimesToTime_t(_data.onTimeHours, _data.onTimeMinutes) );
    //    Serial.println();

    for (byte i = 0; i < _numChannels ; i++) {  // iterate through channels
      //      Serial.println();
      //      Serial.print(F("Channel #"));
      //      Serial.println(i);  // +1 for human readability

      // iterate through step times, there is an additional fade period between array and off time as compared to the "map"
      for (byte j = 0; j <= _mapSize; j++) {


        if (!positionFound[i]) {

          //          Serial.print(F("Checking Step #"));
          //          Serial.println(j);

          if (j == 0) {     // If index is on its first pass
            //  DEBUG
            //                        Serial.println(F("Checking position (j == 0)"));
            //                        Serial.print(F("Index: "));
            //                        Serial.println(j);
            //                        Serial.print(F("Current Time: "));
            //                        Serial.println(_currentTime);
            //                        Serial.print(F("On Time:"));
            //                        Serial.println( byteTimesToTime_t(_data.onTimeHours, _data.onTimeMinutes) );
            //                        Serial.print(F("Step Time: "));
            //                        Serial.println( byteTimesToTime_t( _data.ch[i].stepTimeHours[j], _data.ch[i].stepTimeMinutes[j] ) );

            // Check to see if this is the correct step to start on if we start in the middle of a fade cycle
            if (_currentTime > byteTimesToTime_t(_data.onTimeHours, _data.onTimeMinutes)) {
              if (_currentTime < byteTimesToTime_t(_data.ch[i].stepTimeHours[j], _data.ch[i].stepTimeMinutes[j] )  ) {
                // If this is the correct index window for current time
                _data.ch[i].fadeIndex = j;    // Save our index for this channel
                positionFound[i] = true;


                //Serial.println();
                Serial.print(F("Found fade start point ch #"));
                Serial.println(i + 1);
                Serial.print(F("Index: "));
                Serial.println(_data.ch[i].fadeIndex);
                Serial.print(F("Step: "));
                Serial.println(_data.ch[i].stepCount);
              }

            }

          }
          else if (( j > 0 ) && (j < _mapSize  )) {
            //Serial.println(F("J > 0 && j < mapSize"));

            //                        Serial.println(F("Checking position (j < _mapSize)"));
            //                        Serial.print(F("Index: "));
            //                        Serial.println(j);
            //                        Serial.print(F("Current Time: "));
            //                        Serial.println(_currentTime);
            //                        Serial.print(F("Current Step Time:"));
            //                        Serial.println( byteTimesToTime_t(_data.ch[i].stepTimeHours[j], _data.ch[i].stepTimeMinutes[j] ));
            //                        Serial.print(F("Next Step Time: "));
            //                        Serial.println(byteTimesToTime_t(_data.ch[i].stepTimeHours[j + 1], _data.ch[i].stepTimeMinutes[j + 1]));

            // Check to see if this is the correct step to start on if we start in the middle of a fade cycle
            if (_currentTime > byteTimesToTime_t(_data.ch[i].stepTimeHours[j - 1], _data.ch[i].stepTimeMinutes[j - 1])) {
              if (_currentTime < byteTimesToTime_t(_data.ch[i].stepTimeHours[j], _data.ch[i].stepTimeMinutes[j])) {
                _data.ch[i].fadeIndex = j;
                _data.ch[i].stepCount = map(_currentTime, byteTimesToTime_t( _data.ch[i].stepTimeHours[j], _data.ch[i].stepTimeMinutes[j]), byteTimesToTime_t( _data.ch[i].stepTimeHours[j - 1], _data.ch[i].stepTimeMinutes[j]), _data.ch[i].brightness[j - 1] , _data.ch[i].brightness[j]);
                positionFound[i] = true;


                //Serial.println();
                Serial.print(F("Found fade start point ch #"));
                Serial.println(i + 1);
                Serial.print(F("Index: "));
                Serial.println(_data.ch[i].fadeIndex);
                Serial.print(F("Step: "));
                Serial.println(_data.ch[i].stepCount);
              }

            }

          } else {
            //            Serial.println(F("j >= _mapSize"));
            //                        Serial.println(F("Checking position (else)"));
            //                        Serial.print(F("Index: "));
            //                        Serial.println(j);
            //                        Serial.print(F("Current Time: "));
            //                        Serial.println(_currentTime);
            //                        Serial.print(F("On Time:"));
            //                        Serial.println(byteTimesToTime_t(_data.ch[i].stepTimeHours[j], _data.ch[i].stepTimeMinutes[j]));
            //                        Serial.print(F("Off Time: "));
            //                        Serial.println( byteTimesToTime_t(_data.offTimeHours, _data.onTimeMinutes ));

            // Check to see if this is the correct step to start on if we start in the middle of a fade cycle
            if (_currentTime > byteTimesToTime_t(_data.ch[i].stepTimeHours[j], _data.ch[i].stepTimeMinutes[j])) {
              if (_currentTime < byteTimesToTime_t(_data.offTimeHours, _data.offTimeMinutes ) ) {
                _data.ch[i].fadeIndex = j;
                _data.ch[i].stepCount = map(_currentTime, byteTimesToTime_t( _data.ch[i].stepTimeHours, _data.ch[i].stepTimeMinutes), byteTimesToTime_t( _data.offTimeHours, _data.offTimeMinutes) , _data.ch[i].brightness[j], 0);
                positionFound[i] = true;

                //Serial.println();
                Serial.print(F("Found fade start point ch #"));
                Serial.println(i + 1);
                Serial.print(F("Index: "));
                Serial.println(_data.ch[i].fadeIndex);
                Serial.print(F("Step: "));
                Serial.println(_data.ch[i].stepCount);


              }
            }
            Serial.println();

          }

        }
      }
    }
    Serial.println(F("Done calculating start steps"));
  }
}
// Test function to make sure everything is working right
void Light::testLights() {
  static int currentBright = 0;

  static unsigned long prevFadeTime = millis();
  static bool isRising;
  //Serial.println("------------------------------------------");
  //Serial.print("Current Millis: ");
  //Serial.println(currentMillis);
  //Serial.print("Previous Fade Time: ");
  //Serial.println(prevFadeTime);

  if (millis() - prevFadeTime >= _data.testFadeDelay) {
    //Serial.println("Time to adjust light");
    if (isRising) {
      if (currentBright < 4096) {
        currentBright++;
      } else {
        isRising = false;
      }
    } else {
      if (currentBright > 0) {
        currentBright--;
      } else {
        isRising = true;
      }
    }
    for (int i = 0; i < 6; i++) {
      //Serial.println("Adjusting Light");
      _pwm->setPin(i, currentBright, 0);
    }
    prevFadeTime = millis();
  }
}

// Set Mode - Enum = NORMAL, CUSTOM, TEST
void Light::setMode(byte mode) {
  _data.mode = mode;
}

// Returns Mode Enum
byte Light::getMode() {
  return _data.mode;
}

// set time with a time_t
void Light::setOnTime(time_t onTime) {
  _data.onTimeHours = hour(onTime);
  _data.onTimeMinutes = minute(onTime);

}

// returns on time as a time_t
time_t Light::getOnTime() {
  return intTimesToTime_t(_data.onTimeHours, _data.onTimeMinutes);
}

// set off time with a time_t
void Light::setOffTime(time_t offTime) {
  _data.offTimeHours = hour(offTime);
  _data.onTimeHours = minute(offTime);
}

time_t Light::getOffTime() {
  return intTimesToTime_t(_data.offTimeHours, _data.offTimeMinutes);
}
