#include "Automation.h"
#include "Pins.h"
#include "webpage.h"
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <time.h>

// ==========================================
//           NETWORK CONFIGURATION
// ==========================================

// ==========================================
//           NETWORK CONFIGURATION
// ==========================================
String ssid = "";
String password = "";

Preferences preferences;

// AP Mode Settings (Hotspot)
const char *ap_ssid = "Smart-Fish-Farm";
const char *ap_password = "12345678";

// NTP Server Settings
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // GMT+5:30 = 5.5 * 3600 = 19800
const int daylightOffset_sec = 0;

// ==========================================
//              GLOBAL OBJECTS
// ==========================================
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;
const byte DNS_PORT = 53;

// ==========================================
//            WEBSOCKET HANDLING
// ==========================================
void notifyClients() {
  // Defines a JSON buffer
  JsonDocument doc;

  // Sensor Data
  doc["type"] = "sensors";
  doc["level"] = currentDistance;
  doc["tds"] = currentTDS;
  doc["ph"] = currentPH;
  doc["turb"] = currentTurbidity;

  String output;
  serializeJson(doc, output);
  ws.textAll(output);

  // Actuator States
  doc.clear();
  doc["type"] = "states";
  doc["auto"] = autoMode;
  // Note: Relays are often Active LOW.
  // If digitalWrite(LOW) = ON, then !digitalRead() is true for ON.
  // Adjust logic based on your relay module. Assuming LOW = ON here.
  doc["p1"] = !digitalRead(PIN_RELAY_PUMP_FILL);
  doc["p2"] = !digitalRead(PIN_RELAY_TDS_1);
  doc["p3"] = !digitalRead(PIN_RELAY_TDS_2);
  doc["p4"] = !digitalRead(PIN_RELAY_PH_UP);
  doc["p5"] = !digitalRead(PIN_RELAY_PH_DOWN);
  doc["sol"] = !digitalRead(PIN_RELAY_SOLENOID);

  String output2;
  serializeJson(doc, output2);
  ws.textAll(output2);

  // Settings
  doc.clear();
  doc["type"] = "settings";
  doc["h"] = feedHour;
  doc["m"] = feedMinute;
  doc["d"] = servoDuration;
  doc["lf"] = (lastFedMillis > 0) ? (millis() - lastFedMillis) / 1000 : -1;

  // Get current IST time
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[10];
    strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
    doc["ct"] = timeStr;
  } else {
    doc["ct"] = "--:--";
  }

  String output3;
  serializeJson(doc, output3);
  ws.textAll(output3);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len &&
      info->opcode == WS_TEXT) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error)
      return;

    String cmd = doc["cmd"];
    if (cmd == "auto") {
      autoMode = doc["val"];
    } else if (cmd == "feed") {
      runFeeder();
    } else if (cmd == "toggle") {
      if (!autoMode) { // Only allow manual toggle if Auto Mode is OFF
        int id = doc["id"];
        int pin = -1;
        switch (id) {
        case 1:
          pin = PIN_RELAY_PUMP_FILL;
          break;
        case 2:
          pin = PIN_RELAY_TDS_1;
          break;
        case 3:
          pin = PIN_RELAY_TDS_2;
          break;
        case 4:
          pin = PIN_RELAY_PH_UP;
          break;
        case 5:
          pin = PIN_RELAY_PH_DOWN;
          break;
        case 6:
          pin = PIN_RELAY_SOLENOID;
          break;
        }
        if (pin != -1) {
          digitalWrite(pin, !digitalRead(pin)); // Toggle
        }
      }
      // Immediately notify clients of state change
      notifyClients();
    } else if (cmd == "save_wifi") {
      String new_ssid = doc["s"];
      String new_pass = doc["p"];

      if (new_ssid.length() > 0) {
        preferences.begin("wifi-config", false);
        preferences.putString("ssid", new_ssid);
        preferences.putString("password", new_pass);
        preferences.end();

        // Send confirmation
        // ESP.restart() will happen via client side reload or manual reboot
        // Actually, let's force a reboot after a short delay
        delay(1000);
        ESP.restart();
      }
    } else if (cmd == "save_settings") {
      feedHour = doc["h"];
      feedMinute = doc["m"];
      servoDuration = doc["d"];
      Serial.println("Settings Updated via Web");
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_DATA) {
    handleWebSocketMessage(arg, data, len);
  }
}

// ==========================================
//              SETUP & LOOP
// ==========================================
void setup() {
  Serial.begin(115200);

  // Initialize Pins
  setupSensors();
  setupActuators();

  // Connect to WiFi
  WiFi.mode(WIFI_AP_STA); // Dual Mode

  // 1. Setup Soft AP (Hotspot) Always
  WiFi.softAP(ap_ssid, ap_password);
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

  // 2. Load Credentials
  preferences.begin("wifi-config", true); // Read only
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();

  if (ssid == "") {
    Serial.println("No WiFi Credentials Saved.");
  } else {
    // 3. Try Connect to Router
    WiFi.begin(ssid.c_str(), password.c_str()); // Fixed: Use .c_str()

    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    // Timeout for connection (10 seconds)
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nFailed to connect. Running in AP Mode.");
    }
  }

  // Setup Web Server
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  // Captive Portal Detection URLs (Apple, Android, Windows)
  server.on("/hotspot-detect.html", HTTP_GET,
            [](AsyncWebServerRequest *request) {
              request->send_P(200, "text/html", index_html);
            });
  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });
  server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  // Captive Portal Catch-All
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.begin();

  // 4. Start DNS Server for Captive Portal
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  Serial.println("Web Server & DNS Started");

  // Setup mDNS
  if (MDNS.begin("fishfarm")) {
    Serial.println("MDNS responder started");
    Serial.println("Access via: http://fishfarm.local");
  }

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Waiting for time...");
}

void loop() {
  // 1. Handle Feeder Timing (Non-blocking)
  updateFeeder();

  // 2. Update Sensors & Automation periodically (e.g., every 200ms)
  //    Fast enough for reaction, slow enough to avoid sensor echo interference
  static unsigned long lastSensorTime = 0;
  if (millis() - lastSensorTime > 200) {
    updateSensors();
    runAutomation();
    lastSensorTime = millis();
  }

  // 3. Limit WebSocket broadcasts to every 500ms to save bandwidth
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 500) {
    notifyClients();
    lastTime = millis();
    // Heartbeat to confirm loop is alive
    Serial.print("Alive: ");
    Serial.println(millis());
  }

  // 4. Clean up WebSocket clients
  ws.cleanupClients();

  // 5. DNS Server for Captive Portal
  dnsServer.processNextRequest();

  // No delay() needed here, loop runs as fast as possible

  // 5. Check Schedule
  checkSchedule();
}
