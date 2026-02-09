#ifndef PINS_H
#define PINS_H

// ==========================================
//               PIN DEFINITIONS
// ==========================================

// --- Sensors (Inputs) ---
#define PIN_TRIG            5   // Ultrasonic Trig
#define PIN_ECHO            18  // Ultrasonic Echo
#define PIN_MANUAL_SWITCH   19  // Manual Switch for Cleaning/Solenoid

// --- Analog Sensors (ADC1 only for WiFi usage) ---
// Note: ESP32 ADC2 pins cannot be used when WiFi is active.
#define PIN_PH              32  // pH Sensor
#define PIN_TURBIDITY       34  // Turbidity Sensor
#define PIN_TDS             35  // TDS Sensor

// --- Actuators (Outputs) ---
// We have 5 pumps and 1 solenoid.
// Using 2-channel relay modules x3 = 6 relays.

#define PIN_RELAY_PUMP_FILL 13  // Pump 1: Fish Tank Water Filling
#define PIN_RELAY_TDS_1     12  // Pump 2: TDS Correction (e.g., Drain/Replace)
#define PIN_RELAY_TDS_2     14  // Pump 3: TDS Correction (e.g., Add Minerals)
#define PIN_RELAY_PH_UP     27  // Pump 4: pH Up Solution
#define PIN_RELAY_PH_DOWN   26  // Pump 5: pH Down Solution
#define PIN_RELAY_SOLENOID  25  // Solenoid Valve: Drain/Clean (Turbidity High)

#define PIN_SERVO           4   // Servo Motor: Food Serving

#endif // PINS_H
