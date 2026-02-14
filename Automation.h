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

const float TURBIDITY_HIGH_V = 3.0; // Threshold Voltage

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

// Relay Management
struct RelayControl {
  int pin;
  bool active; // Current logical state (true = ON)
  unsigned long lastToggle;
};

// Index 0 unused, 1-6 map to pumps/solenoid
// 1: Fill, 2: TDS1, 3: TDS2, 4: pH Up, 5: pH Down, 6: Solenoid
RelayControl relays[7];

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

// Safe Relay Control with Debounce
void setRelayState(int id, bool state) {
  if (id < 1 || id > 6)
    return;

  // Debounce: 200ms to prevent rapid relay chatter
  if (millis() - relays[id].lastToggle < 200)
    return;

  if (relays[id].active != state) {
    relays[id].active = state;
    relays[id].lastToggle = millis();
    // Active LOW logic: ON = LOW, OFF = HIGH
    digitalWrite(relays[id].pin, state ? LOW : HIGH);
  }
}

void toggleRelay(int id) {
  if (id < 1 || id > 6)
    return;
  setRelayState(id, !relays[id].active);
}

void setupActuators() {
  // Initialize Relay Map
  relays[1] = {PIN_RELAY_PUMP_FILL, false, 0};
  relays[2] = {PIN_RELAY_TDS_1, false, 0};
  relays[3] = {PIN_RELAY_TDS_2, false, 0};
  relays[4] = {PIN_RELAY_PH_UP, false, 0};
  relays[5] = {PIN_RELAY_PH_DOWN, false, 0};
  relays[6] = {PIN_RELAY_SOLENOID, false, 0};

  for (int i = 1; i <= 6; i++) {
    pinMode(relays[i].pin, OUTPUT);
    digitalWrite(relays[i].pin, HIGH); // Default OFF (Active LOW)
  }

  feederServo.attach(PIN_SERVO);
  feederServo.write(0); // Initial position
}

void setupSensors() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_MANUAL_SWITCH, INPUT_PULLUP);
}

float readUltrasonic() {
  const int numReadings = 5;
  float readings[numReadings];

  // Take multiple readings
  for (int i = 0; i < numReadings; i++) {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    long duration = pulseIn(PIN_ECHO, HIGH, 30000); // 30ms timeout
    if (duration == 0) {
      readings[i] = 999; // Error/Timeout
    } else {
      readings[i] = duration * 0.0343 / 2;
    }
    delay(10); // Small delay between readings
  }

  // Sort the readings (Bubble Sort)
  for (int i = 0; i < numReadings - 1; i++) {
    for (int j = 0; j < numReadings - i - 1; j++) {
      if (readings[j] > readings[j + 1]) {
        float temp = readings[j];
        readings[j] = readings[j + 1];
        readings[j + 1] = temp;
      }
    }
  }

  // Get Median (middle element)
  float medianDistance = readings[numReadings / 2];

  // Debug Output
  Serial.print("Ultra raw: [");
  for (int i = 0; i < numReadings; i++) {
    Serial.print(readings[i]);
    if (i < numReadings - 1)
      Serial.print(", ");
  }
  Serial.print("] -> Median: ");
  Serial.println(medianDistance);

  return medianDistance;
}

float readTDS() {
  int raw = analogRead(PIN_TDS);
  return map(raw, 0, 4095, 0, 1000); // Demo mapping
}

float readPH() {
  int raw = analogRead(PIN_PH);
  return map(raw, 0, 4095, 0, 1400) / 100.0; // Demo mapping
}

float readTurbidity() {
  int raw = analogRead(PIN_TURBIDITY);
  return (raw / 4095.0) * 3.3;
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

  int currentSlot =
      timeinfo.tm_yday * 10000 + timeinfo.tm_hour * 100 + timeinfo.tm_min;

  for (int i = 0; i < feedCount && i < MAX_FEED_TIMES; i++) {
    if (feedTimes[i][0] == -1)
      continue;

    if (timeinfo.tm_hour == feedTimes[i][0] &&
        timeinfo.tm_min == feedTimes[i][1]) {
      if (currentSlot != lastFedSlot) {
        runFeeder();
        lastFedSlot = currentSlot;
        break;
      }
    }
  } // End for loop
} // End checkSchedule

long getSecondsToNextFeed() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return -1;
  }

  long currentSeconds =
      timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
  long minDiff = -1;

  for (int i = 0; i < feedCount && i < MAX_FEED_TIMES; i++) {
    if (feedTimes[i][0] == -1)
      continue;

    long feedSeconds = feedTimes[i][0] * 3600 + feedTimes[i][1] * 60;
    long diff = feedSeconds - currentSeconds;

    if (diff < 0) {  // If time passed today
      diff += 86400; // Then it's tomorrow (+24h)
    }

    if (minDiff == -1 || diff < minDiff) {
      minDiff = diff;
    }
  }
  return minDiff;
}

void updateSensors() {
  currentDistance = readUltrasonic();
  currentTDS = readTDS();
  currentPH = readPH();
  currentTurbidity = readTurbidity();
}

void runAutomation() {
  if (!autoMode)
    return;

  // 1. Water Level Control
  // Distance > HIGH (e.g. 30 > 25) -> Level Low -> Fill ON
  if (currentDistance > LEVEL_HIGH_CM) {
    setRelayState(1, true);
  } else if (currentDistance < LEVEL_LOW_CM) {
    setRelayState(1, false);
  }

  // 2. TDS Control (With Hysteresis)
  // Deadband: 50ppm
  if (currentTDS > TDS_HIGH) {
    setRelayState(2, true); // Drain/Repl
    setRelayState(3, false);
  } else if (currentTDS < (TDS_HIGH - 50) && currentTDS > (TDS_LOW + 50)) {
    // In "Safe Zone" - turn off both
    setRelayState(2, false);
    setRelayState(3, false);
  } else if (currentTDS < TDS_LOW) {
    setRelayState(2, false);
    setRelayState(3, true); // Add Minerals
  }

  // 3. pH Control (With Hysteresis)
  // Deadband: 0.2 pH
  if (currentPH < PH_LOW) {
    setRelayState(4, true); // pH Up
    setRelayState(5, false);
  } else if (currentPH > (PH_LOW + 0.2) && currentPH < (PH_HIGH - 0.2)) {
    // In "Safe Zone" - turn off both
    setRelayState(4, false);
    setRelayState(5, false);
  } else if (currentPH > PH_HIGH) {
    setRelayState(4, false);
    setRelayState(5, true); // pH Down
  }

  // 4. Turbidity -> Solenoid
  // Add simple hysteresis for solenoid too
  if (currentTurbidity < 2.0) {
    setRelayState(6, true); // Open Valve (Dirty)
  } else if (currentTurbidity > 2.5) {
    // Only turn off if significantly clear
    // Check Manual Switch first (Active LOW)
    if (digitalRead(PIN_MANUAL_SWITCH) == LOW) {
      setRelayState(6, true);
    } else {
      setRelayState(6, false);
    }
  }
}

#endif // AUTOMATION_H
