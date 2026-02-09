#ifndef AUTOMATION_H
#define AUTOMATION_H

#include "Pins.h"
#include <Arduino.h>
#include <ESP32Servo.h>
#include <time.h>

// --- Constants & Thresholds ---
// These need to be calibrated!
const float LEVEL_LOW_CM = 10.0;  // If water level < 10cm, Refill
const float LEVEL_HIGH_CM = 25.0; // Full tank level

const float TDS_LOW = 100.0;  // ppm
const float TDS_HIGH = 500.0; // ppm

const float PH_LOW = 6.5;
const float PH_HIGH = 8.5;

const float TURBIDITY_HIGH_V =
    3.0; // Threshold Voltage (lower voltage = clearer usually, implies
         // inversion depending on sensor)
// Standard analog Turbidity sensors: High Voltage = Clean, Low Voltage = Dirty
// (or vice versa depending on module) Let's assume standard analog output:
// Higher value = Dirty (Logic needs verification with specific sensor)

// --- Global Objects ---
Servo feederServo;

// --- State Variables ---
float currentDistance = 0;
float currentTDS = 0;
float currentPH = 0;
float currentTurbidity = 0;

bool autoMode = true; // false = Manual Override via Web
bool isFeeding = false;
unsigned long feedStartTime = 0;

// Default Settings (Can be updated via Web)
const int MAX_FEED_TIMES = 5;
int feedTimes[MAX_FEED_TIMES][2] = {{7, 0},
                                    {-1, -1},
                                    {-1, -1},
                                    {-1, -1},
                                    {-1, -1}}; // [hour, minute], -1 = inactive
int feedCount = 1;                             // Number of active schedules
int servoDuration = 1;                         // 1 Second default

// State for Scheduler
int lastFedSlot = -1; // Specific time slot (Day + HH + MM) that we last fed
unsigned long lastFedMillis = 0; // Uptime when last fed

// --- Helper Functions ---

void setupActuators() {
  pinMode(PIN_RELAY_PUMP_FILL, OUTPUT);
  digitalWrite(PIN_RELAY_PUMP_FILL, HIGH); // Relay Active LOW usually

  pinMode(PIN_RELAY_TDS_1, OUTPUT);
  digitalWrite(PIN_RELAY_TDS_1, HIGH);

  pinMode(PIN_RELAY_TDS_2, OUTPUT);
  digitalWrite(PIN_RELAY_TDS_2, HIGH);

  pinMode(PIN_RELAY_PH_UP, OUTPUT);
  digitalWrite(PIN_RELAY_PH_UP, HIGH);

  pinMode(PIN_RELAY_PH_DOWN, OUTPUT);
  digitalWrite(PIN_RELAY_PH_DOWN, HIGH);

  pinMode(PIN_RELAY_SOLENOID, OUTPUT);
  digitalWrite(PIN_RELAY_SOLENOID, HIGH);

  feederServo.attach(PIN_SERVO);
  feederServo.write(0); // Initial position
}

void setupSensors() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_MANUAL_SWITCH, INPUT_PULLUP);

  // Analog pins are INPUT by default
}

float readUltrasonic() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duration = pulseIn(PIN_ECHO, HIGH, 30000); // 30ms timeout (approx 5m)
  // Distance = (Time * SpeedOfSound) / 2
  // Speed of sound ~= 0.0343 cm/us
  float distance = duration * 0.0343 / 2;
  return distance;
}

// Dummy conversion functions - Need CAREFUL CALIBRATION with real sensors
float readTDS() {
  int raw = analogRead(PIN_TDS);
  // V = (raw / 4095.0) * 3.3;
  // float tdsValue = ... formula ...
  // Mapping 0-4095 to 0-1000ppm for demo
  return map(raw, 0, 4095, 0, 1000);
}

float readPH() {
  int raw = analogRead(PIN_PH);
  // Standard pH sensor: 2.5V = pH 7
  // This requires strict calibration per sensor module
  float voltage = (raw / 4095.0) * 3.3;
  // Example linear approx: pH = 7 + ((2.5 - voltage) / 0.18)
  // Simplifying for demo mapped 0-14
  return map(raw, 0, 4095, 0, 1400) / 100.0;
}

float readTurbidity() {
  int raw = analogRead(PIN_TURBIDITY);
  float voltage = (raw / 4095.0) * 3.3;
  // Some sensors: High V = Clean, Low V = Dirty
  // Others: Low V = Clean, High V = Dirty
  // Providing raw voltage for now
  return voltage;
}

void runFeeder() {
  if (!isFeeding) {
    feederServo.write(90); // Open
    feedStartTime = millis();
    lastFedMillis = feedStartTime;
    isFeeding = true;
  }
}

void updateFeeder() {
  if (isFeeding && millis() - feedStartTime >= (servoDuration * 1000)) {
    feederServo.write(0); // Close
    isFeeding = false;
  }
}

void checkSchedule() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  // Calculate current slot ID: Day of Year * 10000 + Hour * 100 + Min
  int currentSlot =
      timeinfo.tm_yday * 10000 + timeinfo.tm_hour * 100 + timeinfo.tm_min;

  // Check all active feeding times
  for (int i = 0; i < feedCount && i < MAX_FEED_TIMES; i++) {
    if (feedTimes[i][0] == -1)
      continue; // Skip inactive slots

    if (timeinfo.tm_hour == feedTimes[i][0] &&
        timeinfo.tm_min == feedTimes[i][1]) {
      // Ensure we trigger only once for this specific time slot
      if (currentSlot != lastFedSlot) {
        runFeeder();
        lastFedSlot = currentSlot;
        break; // Only feed once per minute
      }
    }
  }
}

void updateSensors() {
  currentDistance = readUltrasonic();
  currentTDS = readTDS();
  currentPH = readPH();
  currentTurbidity = readTurbidity();
}

void runAutomation() {
  if (!autoMode)
    return; // Skip if Manual

  // 1. Water Level Control
  // If distance is LARGE (water low), Turn ON Pump
  // Using simple Hysteresis
  // Distance 10cm = High Level (Water close to sensor)
  // Distance 30cm = Low Level (Water far from sensor)

  // Example: Sensor at top (0cm). Water at 30cm (Empty). Water at 5cm (Full).
  if (currentDistance > LEVEL_HIGH_CM) {
    digitalWrite(PIN_RELAY_PUMP_FILL, LOW); // ON (Active Low)
  } else if (currentDistance < LEVEL_LOW_CM) {
    digitalWrite(PIN_RELAY_PUMP_FILL, HIGH); // OFF
  }

  // 2. TDS Control
  if (currentTDS > TDS_HIGH) {
    digitalWrite(PIN_RELAY_TDS_1, LOW); // Pump 2 ON
    digitalWrite(PIN_RELAY_TDS_2, HIGH);
  } else if (currentTDS < TDS_LOW) {
    digitalWrite(PIN_RELAY_TDS_1, HIGH);
    digitalWrite(PIN_RELAY_TDS_2, LOW); // Pump 3 ON
  } else {
    digitalWrite(PIN_RELAY_TDS_1, HIGH); // OFF
    digitalWrite(PIN_RELAY_TDS_2, HIGH); // OFF
  }

  // 3. pH Control
  if (currentPH < PH_LOW) {
    digitalWrite(PIN_RELAY_PH_UP, LOW); // pH Up ON
    digitalWrite(PIN_RELAY_PH_DOWN, HIGH);
  } else if (currentPH > PH_HIGH) {
    digitalWrite(PIN_RELAY_PH_UP, HIGH);
    digitalWrite(PIN_RELAY_PH_DOWN, LOW); // pH Down ON
  } else {
    digitalWrite(PIN_RELAY_PH_UP, HIGH);
    digitalWrite(PIN_RELAY_PH_DOWN, HIGH);
  }

  // 4. Turbidity -> Solenoid
  // Assume Voltage < 2.0V is Dirty
  if (currentTurbidity < 2.0) {
    digitalWrite(PIN_RELAY_SOLENOID, LOW); // Open Valve
  } else {
    // Check Manual Switch
    if (digitalRead(PIN_MANUAL_SWITCH) == LOW) { // Pressed
      digitalWrite(PIN_RELAY_SOLENOID, LOW);
    } else {
      digitalWrite(PIN_RELAY_SOLENOID, HIGH);
    }
  }
}

#endif
