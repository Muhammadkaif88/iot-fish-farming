#ifndef AUTOMATION_H
#define AUTOMATION_H

#include "Pins.h"
#include <Arduino.h>
#include <ESP32Servo.h>
#include <time.h>

// --- Constants & Thresholds ---
// These need to be calibrated!
// --- Constants & Thresholds ---
// These need to be calibrated!
// If water level < 3cm from sensor, Fill OFF (Full)
const float LEVEL_LOW_CM = 3.0;

// If water level > 14cm from sensor, Fill ON (Empty)
const float LEVEL_HIGH_CM = 14.0;

const float TDS_LOW = 100.0;  // ppm - Too low, need minerals
const float TDS_HIGH = 500.0; // ppm - Too high, need dilution

const float PH_LOW = 6.0;     // Acidic threshold
const float PH_HIGH = 7.5;    // Alkaline threshold

const float TURBIDITY_DIRTY_NTU = 1000.0; // Threshold to trigger auto-cleaning
const float TURBIDITY_CLEAR_NTU = 200.0;  // Threshold where water is considered "Clear"

// --- Global Objects ---
Servo feederServo;

// --- State Variables ---
float currentDistance = 0;
float currentTDS = 0;
float currentPH = 0;
float currentTurbidity = 0;

bool autoMode = false; // false = Manual Override via Web
bool isFeeding = false;
unsigned long feedStartTime = 0;

// --- Tank Cleaning Mode ---
bool cleaningMode = false;
// Phase 0 = Draining (solenoid open)
// Phase 1 = Refilling (fill pump on)
int cleaningPhase = 0;

// Relay Management
struct RelayControl {
  int pin;
  bool active; // Current logical state (true = ON)
  unsigned long lastToggle;
};

// Index 0 unused, 1-6 map to pumps/solenoid
// 1: Fill, 2: TDS Correction A, 3: TDS Correction B, 4: pH Up, 5: pH Down, 6: Solenoid
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

// Safe Relay Control with Debounce to protect hardware
void setRelayState(int id, bool state) {
  if (id < 1 || id > 6)
    return;

  // Debounce: 200ms to prevent rapid relay chatter from noisy sensor data
  if (millis() - relays[id].lastToggle < 200)
    return;

  if (relays[id].active != state) {
    relays[id].active = state;
    relays[id].lastToggle = millis();
    // Active LOW logic: ON = LOW, OFF = HIGH (Common in 5V/12V Relay Modules)
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
}

void setupSensors() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_MANUAL_SWITCH, INPUT_PULLUP);
}

// Median filter for Ultrasonic to remove outliers
float readUltrasonic() {
  const int numReadings = 5;
  float readings[numReadings];

  for (int i = 0; i < numReadings; i++) {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    long duration = pulseIn(PIN_ECHO, HIGH, 30000); // 30ms timeout (~5m range)
    if (duration == 0) {
      readings[i] = 999; // Error/Timeout
    } else {
      readings[i] = duration * 0.0343 / 2;
    }
    delay(10);
  }

  // Bubble Sort for median
  for (int i = 0; i < numReadings - 1; i++) {
    for (int j = 0; j < numReadings - i - 1; j++) {
      if (readings[j] > readings[j + 1]) {
        float temp = readings[j];
        readings[j] = readings[j + 1];
        readings[j + 1] = temp;
      }
    }
  }

  return readings[numReadings / 2];
}

float readTDS() {
  int raw = analogRead(PIN_TDS);
  float voltage = raw * (3.3 / 4095.0);
  // Basic TDS formula (needs calibration for specific sensor)
  float tdsValue = (133.42 * pow(voltage, 3) - 255.86 * pow(voltage, 2) + 857.39 * voltage) * 0.5;
  return tdsValue;
}

float readPH() {
  int raw = analogRead(PIN_PH);
  // Mapping 0-3.3V to 0-14 pH (Linear approximation)
  return (raw / 4095.0) * 14.0;
}

float readTurbidity() {
  int raw = analogRead(PIN_TURBIDITY);
  float voltage = (raw / 4095.0) * 3.3;
  
  // High Voltage (>2.5V) = Clear Water
  // Low Voltage (<1.0V) = Very Dirty Water
  float ntu = 0;
  if (voltage < 2.5) {
    ntu = 3000; // Max dirty
  } else {
    // Linear mapping for demonstration: 3.3V -> 0 NTU, 2.5V -> 3000 NTU
    ntu = (3.3 - voltage) * (3000.0 / 0.8);
  }
  
  return (ntu < 0) ? 0 : (ntu > 3000 ? 3000 : ntu);
}

void runFeeder() {
  if (!isFeeding) {
    if (!feederServo.attached()) {
      feederServo.attach(PIN_SERVO);
    }
    feederServo.write(0); // Full speed rotation for continuous servo
    feedStartTime = millis();
    lastFedMillis = feedStartTime;
    isFeeding = true;
    Serial.println("Feeder: DISPENSING");
  }
}

void updateFeeder() {
  if (isFeeding && millis() - feedStartTime >= (servoDuration * 1000)) {
    feederServo.write(90); // Stop continuous servo
    delay(50);
    feederServo.detach();  // Detach to save power and prevent jitter
    isFeeding = false;
    Serial.println("Feeder: STOPPED");
  }
}

void checkSchedule() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int currentSlot = timeinfo.tm_yday * 10000 + timeinfo.tm_hour * 100 + timeinfo.tm_min;

  for (int i = 0; i < feedCount && i < MAX_FEED_TIMES; i++) {
    if (feedTimes[i][0] == -1) continue;

    if (timeinfo.tm_hour == feedTimes[i][0] && timeinfo.tm_min == feedTimes[i][1]) {
      if (currentSlot != lastFedSlot) {
        runFeeder();
        lastFedSlot = currentSlot;
        break;
      }
    }
  }
}

void getNextFeedTime(int &next_h, int &next_m, long &seconds_remaining) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    seconds_remaining = -1; next_h = -1; next_m = -1;
    return;
  }

  long currentSeconds = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
  long minDiff = -1;

  for (int i = 0; i < feedCount && i < MAX_FEED_TIMES; i++) {
    if (feedTimes[i][0] == -1) continue;
    long feedSeconds = feedTimes[i][0] * 3600 + feedTimes[i][1] * 60;
    long diff = feedSeconds - currentSeconds;
    if (diff < 0) diff += 86400; // Tomorrow

    if (minDiff == -1 || diff < minDiff) {
      minDiff = diff; next_h = feedTimes[i][0]; next_m = feedTimes[i][1];
    }
  }
  seconds_remaining = minDiff;
}

void updateSensors() {
  currentDistance = readUltrasonic();
  currentTDS = readTDS();
  currentPH = readPH();
  currentTurbidity = readTurbidity();
}

// Tank Cleaning State Machine
void runCleaning() {
  if (!cleaningMode) return;

  // PHASE 0: Draining
  if (cleaningPhase == 0) {
    setRelayState(1, false); // Fill pump OFF
    setRelayState(6, true);  // Solenoid OPEN (drain)

    if (currentDistance >= 14.5) { // Tank nearly empty
      cleaningPhase = 1;
      setRelayState(6, false); // Close solenoid
      Serial.println("Cleaning: Tank empty. Starting refill...");
    }
  }
  // PHASE 1: Refilling
  else if (cleaningPhase == 1) {
    setRelayState(6, false); // Solenoid CLOSED
    setRelayState(1, true);  // Fill pump ON

    if (currentDistance <= 9.0) { // Tank refilled to 50%
      setRelayState(1, false); // Pump OFF
      cleaningMode = false;
      cleaningPhase = 0;
      Serial.println("Cleaning: DONE.");
    }
  }
}

void runAutomation() {
  if (!autoMode || cleaningMode) return;

  // 1. Water Level Control
  if (currentDistance > LEVEL_HIGH_CM) {
    setRelayState(1, true); // Too low, fill up
  } else if (currentDistance < LEVEL_LOW_CM) {
    setRelayState(1, false); // Full, stop
  }

  // 2. TDS Control (Hysteresis)
  if (currentTDS > TDS_HIGH) {
    setRelayState(2, true);  // High TDS: Drain/Refill
    setRelayState(3, false);
  } else if (currentTDS < TDS_LOW) {
    setRelayState(2, false);
    setRelayState(3, true);  // Low TDS: Add Minerals
  } else if (currentTDS > (TDS_LOW + 20) && currentTDS < (TDS_HIGH - 20)) {
    setRelayState(2, false); // Stable Zone
    setRelayState(3, false);
  }

  // 3. pH Control
  if (currentPH < PH_LOW) {
    setRelayState(4, true);  setRelayState(5, false); // pH Up
  } else if (currentPH > PH_HIGH) {
    setRelayState(4, false); setRelayState(5, true);  // pH Down
  } else {
    setRelayState(4, false); setRelayState(5, false); // Stable
  }

  // 4. Turbidity Trigger
  if (currentTurbidity > TURBIDITY_DIRTY_NTU) {
    cleaningMode = true;
    cleaningPhase = 0;
    Serial.println("Automation: Dirty water detected! Starting Clean Cycle.");
  }
}


#endif // AUTOMATION_H
