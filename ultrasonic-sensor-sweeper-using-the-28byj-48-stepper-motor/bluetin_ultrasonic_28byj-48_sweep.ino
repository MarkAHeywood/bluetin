
/*
 * www.bluetin.io
 * 
MIT License

Copyright (c) 2017 Mark A Heywood

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 * This is an ultrasonic distance measuring sensor
 * sweeper routine using the 28BYJ-48 hobby stepper motor.
 * 
 * The sensor sweeps back and forth between the rotation limit
 * switch collecting distance data. 
 * 
 * The program begins by first checking the limit switch. If
 * the limit switch is in contact with the platform, the program  
 * will stall until the switch contact is manually cleared. This 
 * procedure will help avoid jumper wire twist or tangle.
 * 
 * If the limit switch is not in contact with the platform,
 * the program will begin the sensor sweep routine as normal.
 */

//------------------- Port and Pins Configuration -------------------
uint8_t stepperPort = 0xF0; // Set stepper port pins for port D - 0b11110000.
uint8_t serialMask = 0x03;  // Mask the remaining pins not used on port D.
const uint8_t SWITCH_PIN = 8;  // Set the pin number for limit switch.
bool currentSwitchState = false;  // Switch flag.
int stepsPerSwitchCheck = 5; // Set steps per limit switch check.
 
//------------------- Stepper Motor Variables and Constants -------------------
// Phase sequence for the 28BYJ-48 hobby stepper motor.
const uint8_t STEPPERPHASES8[8] = {0x30, 0x20, 0x60, 0x40, 0xC0, 0x80, 0x90, 0x10};
// STEPS_PER_DEG = steps per motor turn * gear ratio / 360 degrees.
// const double STEPS_PER_DEG = 60 * 60 / 360;
const double STEPS_PER_DEG = 11.38;
uint8_t phaseIndex = 0; // Keep track of current stepper motor phase.
long timeInterval = 1500; // The delay time between each stepper step.
bool currentStepDirection = true; // Track current stepper ratation.
uint16_t maxDistance = 100; // Maximum sensor distance range, cm or inch.
uint16_t minDistance = 5; // Minimum sensor distance range, cm or inch.

//------------------- Time and Counter variables -------------------
long savedTime = 0;
uint16_t degreesCount = 0;
uint16_t stepCount = 0; // Track stepper steps between limit switch checks.
uint16_t oldStepCount = 0;  // Step count on last total degrees update.
uint16_t stepsPosition = 0; // Count steps between limit switch for angle calculations.
uint16_t maxStepsPerSweep = 5000;  // This will be adjusted during operation.

//------------------- Ultrasonic Sensor Pin Configuration -------------------
const uint8_t ECHO_PIN = 11;
const uint8_t TRIGGER_PIN = 10;

/*================================================ */
/*
 * This fuction is the stepper driver and is
 * responsible for managing the stepper motor step
 * sequence.
 */
void phase8(bool setDirection) {
  // Clear stepper pins
  PORTD = PORTD & serialMask;

  bool rotateDirection = setDirection;
  
  if (rotateDirection == true) {
    // Output the next stepper phase - Clockwise
    PORTD = PORTD | STEPPERPHASES8[phaseIndex];
    if (phaseIndex == 7) {
      phaseIndex = 0;
    } else {
      phaseIndex++;
    }
  } else {
    // Output the next stepper phase - Counter Clockwise
    PORTD = PORTD | STEPPERPHASES8[phaseIndex];
    if (phaseIndex == 0) {
      phaseIndex = 7;
    } else {
      phaseIndex--;
    }
  }
}

/*================================================ */
/*
 * A sweeping function that is non blocking.
 * This is ideal if you want to sweep left and right
 * with a proximitiy sensor such as an ultrasonic
 * distance measuring sensor.
 * The non locking feature allows you to run other 
 * code between each step of the stepper motor.
 */
bool sweep() {
  bool rotateDirection = currentStepDirection;
  bool newStep = false; 
  long timeNow = micros();
  
  if (timeNow - savedTime > timeInterval) { // limit the stepper motor stepping frequency.
    phase8(rotateDirection);  // Make a stepper motor step.
    savedTime = micros(); // Record the time of the last stepper motor step.
    stepCount++;  // Count stepper motor steps for limiting switch state checks.
    stepsPosition++;  // Count the steps for the current stepper sweep direction.
    newStep = true; // Indicate that a new step was made.
    
    if (stepCount >= stepsPerSwitchCheck) {
      if (digitalRead(SWITCH_PIN) == false) { // Check limit switch state.
        // Contact with limit switch made
        if (currentSwitchState == false) {  // New switch state.
          // Change stepper rotation
          if (currentStepDirection == true) {
            currentStepDirection = false; // Change stepper rotation direction. 
            maxStepsPerSweep = stepsPosition; // Update number of steps per sweep.
          } else {
            currentStepDirection = true; // Change stepper rotation direction.
            maxStepsPerSweep = stepsPosition;  // Update number of steps per sweep.
          }
          currentSwitchState = true; // Indicate that this limit switch trigger was processed.
          //Serial.print(currentStepDirection);
          //Serial.print(" - ");
          //Serial.println(stepsPosition);
          stepsPosition = 0;  // Reset counters for next sweep.
          oldStepCount = 0;
        }
      } else {
        // Since we're now clear of the limit switch,
        // we can now clear the switch state flag.
        currentSwitchState = false;
      }
      stepCount = 0;  // Reset the counter after chaging the stepper sweep direction.
    }
  }
  return newStep; // Return true if a stepper step was made.
}

/*================================================ */

uint16_t echo(bool isCM) {
  long echoDuration;
  uint16_t distance;

  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIGGER_PIN, LOW);

  echoDuration = pulseIn(ECHO_PIN, HIGH, 2500); // Returns microseconds

  if (isCM) {
    // Distance in cm conversion.
    distance = echoDuration / 29 / 2;
    /*
    Serial.print(distance);
    Serial.println("cm");
    */
  } else {
    // Distance in inches conversion.
    distance = echoDuration / 74 / 2;
    /*
    Serial.print(distance);
    Serial.println("in");
    */
  }
  delay(1);
  return distance;
}

/*================================================ */

bool convertToDegrees() {
  // We only want to calculate degrees when enough motor steps have past.
  bool newCalculation = false;  // New calculation status flag.
  uint16_t newStepCount = oldStepCount + STEPS_PER_DEG;

  if (stepsPosition >= newStepCount) {  // Is the new position 1 deg more than last check.
    if (currentStepDirection == true) { // Are we moving in the positive direction?
      degreesCount = stepsPosition / STEPS_PER_DEG;
    } else {
      uint16_t steps = maxStepsPerSweep - stepsPosition;
      degreesCount = steps / STEPS_PER_DEG;
    }
    oldStepCount = stepsPosition; // Save the current step position.
    //Serial.println(stepsPosition);
    newCalculation = true;  // New data is generated.
  }
  return newCalculation;
}

/*================================================ */

void setup() {
  Serial.begin(115200);
  // Set stepper pins to outputs.
  DDRD = DDRD | stepperPort;
  // Configure limit switch pin.
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  // Configure ultrasonic sensor pins.
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  if (sweep() == true) {
    // We have a new step.
    // Check for new degree change freshold.
    if (convertToDegrees() == true) {
      // Get new sensor reading on each new degree turn.
      uint16_t distance = echo(true); // Report in cm = true.
      // Filter the distance range to print
      if (distance >= minDistance && distance <= maxDistance) {
        Serial.print("Object detected ");
        Serial.print(distance);
        Serial.print("cm");
        Serial.print(" at ");
        Serial.print(degreesCount);
        Serial.println(" deg");
      }
    }
  }

  // Add aditional routines here.

}
