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

volatile bool needsBroadcast = false;

void notifySettings(AsyncWebSocketClient *client = nullptr) {
  JsonDocument doc;
  doc["type"] = "settings";
  JsonArray times = doc["times"].to<JsonArray>();
  for (int i = 0; i < feedCount && i < MAX_FEED_TIMES; i++) {
    if (feedTimes[i][0] != -1) {
      JsonArray slot = times.add<JsonArray>();
      slot.add(feedTimes[i][0]);
      slot.add(feedTimes[i][1]);
    }
  }
  doc["d"] = servoDuration;

  String output;
  serializeJson(doc, output);

  if (client) {
    client->text(output);
  } else {
    ws.textAll(output);
  }
}

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

  // Use logical state from Automation.h for faster feedback
  doc["p1"] = relays[1].active;
  doc["p2"] = relays[2].active;
  doc["p3"] = relays[3].active;
  doc["p4"] = relays[4].active;
  doc["p5"] = relays[5].active;
  doc["p6"] = relays[6].active;

  doc["lf"] = (lastFedMillis > 0) ? (millis() - lastFedMillis) / 1000 : -1;
  doc["nr"] = getSecondsToNextFeed();

  // Get current IST time
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[10];
    strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
    doc["ct"] = timeStr;
  } else {
    doc["ct"] = "--:--";
  }

  String output2;
  serializeJson(doc, output2);
  ws.textAll(output2);
}

void saveInternalSettings() {
  preferences.begin("settings", false);
  preferences.putInt("count", feedCount);
  preferences.putInt("dur", servoDuration);
  // Save times as a flat blob if possible, or individual keys
  // Simplified: Up to 5 times = 10 ints.
  // We'll use keys t0_h, t0_m, t1_h...
  for (int i = 0; i < MAX_FEED_TIMES; i++) {
    char keyH[10], keyM[10];
    sprintf(keyH, "t%d_h", i);
    sprintf(keyM, "t%d_m", i);
    preferences.putInt(keyH, feedTimes[i][0]);
    preferences.putInt(keyM, feedTimes[i][1]);
  }
  preferences.end();
  Serial.println("Settings Saved to Flash");
}

void loadInternalSettings() {
  preferences.begin("settings", true);
  feedCount = preferences.getInt("count", 1);
  servoDuration = preferences.getInt("dur", 1);

  for (int i = 0; i < MAX_FEED_TIMES; i++) {
    char keyH[10], keyM[10];
    sprintf(keyH, "t%d_h", i);
    sprintf(keyM, "t%d_m", i);
    // Default to what we had in Automation.h if needed, or -1 usually
    // The default setup in Automation.h writes to index 0.
    // If flash is empty (returns 0 or 0), we might overwrite.
    // But getInt default handles missing keys.
    // We'll trust the default values in Automation.h if keys invalid (0 is
    // valid though). Let's rely on Automation.h defaults strictly if "count"
    // was missing (implies fresh chip) Actually Automation.h inits array. We
    // just overwrite if keys exist.
    feedTimes[i][0] = preferences.getInt(keyH, feedTimes[i][0]);
    feedTimes[i][1] = preferences.getInt(keyM, feedTimes[i][1]);
  }
  preferences.end();
  Serial.println("Settings Loaded from Flash");
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
        toggleRelay(doc["id"]);
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

        delay(1000);
        ESP.restart();
      }
    } else if (cmd == "save_settings") {
      // Receive array of feeding times
      JsonArray times = doc["times"];
      feedCount = 0;

      // Clear all slots first
      for (int i = 0; i < MAX_FEED_TIMES; i++) {
        feedTimes[i][0] = -1;
        feedTimes[i][1] = -1;
      }

      // Populate from received data
      for (JsonArray slot : times) {
        if (feedCount >= MAX_FEED_TIMES)
          break;
        feedTimes[feedCount][0] = slot[0];
        feedTimes[feedCount][1] = slot[1];
        feedCount++;
      }

      servoDuration = doc["d"];
      saveInternalSettings();
      Serial.println("Multiple Schedules Updated via Web");
      notifySettings(); // Broadcast updated schedules
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_DATA) {
    handleWebSocketMessage(arg, data, len);
  } else if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket Connected");
    notifySettings(client); // Send settings ONLY on connect
  }
}

// ==========================================
//              SETUP & LOOP
// ==========================================
// WiFi Timer
unsigned long lastWifiCheck = 0;
const unsigned long WIFI_CHECK_INTERVAL = 10000; // Check every 10 seconds

void setup() {
  Serial.begin(115200);

  // Initialize Pins
  setupSensors();
  setupActuators();
  loadInternalSettings();

  // Connect to WiFi
  WiFi.mode(WIFI_AP_STA); // Dual Mode

  // LOWER TX POWER TO PREVENT BROWNOUT REBOOTS
  // Default is 19.5dBm (max). Reducing it significantly saves ~100mA current
  // spike.
  WiFi.setTxPower(WIFI_POWER_11dBm);

  // 1. Setup Soft AP (Hotspot) Always - START IMMEDIATELY
  WiFi.softAP(ap_ssid, ap_password);
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

  // 2. Load Credentials
  preferences.begin("wifi-config", true); // Read only
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();

  if (ssid != "" && password != "") {
    // 3. Try Connect into Router (Non-Blocking)
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid.c_str(), password.c_str());
    WiFi.setSleep(false); // Improve responsiveness
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
  } else {
    Serial.println("No WiFi Credentials Saved.");
  }

  // Setup Web Server
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Serve Index with Caching Headers (10 minutes)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response =
        request->beginResponse_P(200, "text/html", index_html);
    response->addHeader("Cache-Control", "public, max-age=600");
    request->send(response);
  });

  // Captive Portal Detection URLs (Apple, Android, Windows)
  server.on("/hotspot-detect.html", HTTP_GET,
            [](AsyncWebServerRequest *request) {
              request->redirect("http://" + WiFi.softAPIP().toString());
            });
  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("http://" + WiFi.softAPIP().toString());
  });
  server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("http://" + WiFi.softAPIP().toString());
  });
  server.on("/redirect", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("http://" + WiFi.softAPIP().toString());
  });
  server.on("/success.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "success");
  });

  // Captive Portal Catch-All - Always serve the portal
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
}

void loop() {
  // 1. Handle Feeder Timing (Non-blocking)
  updateFeeder();

  // 2. Update Sensors & Automation periodically
  static unsigned long lastSensorTime = 0;
  if (millis() - lastSensorTime > 200) {
    updateSensors();
    runAutomation();
    lastSensorTime = millis();
  }

  // 3. Limit WebSocket broadcasts to every 1000ms (Reduced form 500ms)
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 1000) {
    notifyClients();
    lastTime = millis();
    // Heartbeat
    Serial.print("Alive: ");
    Serial.println(millis());
  }

  // 4. Clean up WebSocket clients
  ws.cleanupClients();

  // 5. DNS Server for Captive Portal - Process EVERY LOOP for speed
  dnsServer.processNextRequest();

  // 6. Check Schedule
  checkSchedule();

  // 7. Non-Blocking WiFi Reconnection Logic
  if (ssid != "" && millis() - lastWifiCheck > WIFI_CHECK_INTERVAL) {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }

  // 8. Handle Deferred Broadcasts (Immediate response to actions)
  if (needsBroadcast) {
    notifyClients();
    needsBroadcast = false;
  }
}
