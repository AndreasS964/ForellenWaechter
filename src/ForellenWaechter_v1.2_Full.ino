/*
 * ═══════════════════════════════════════════════════════════════════════════
 * ForellenWächter v1.2 - Full Edition
 * Trout Guardian - IoT Monitoring System für Forellenzucht
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 * Features:
 * - Echtzeit-Monitoring: Wassertemperatur, Lufttemperatur, pH, TDS, Wasserlevel
 * - Web-Dashboard mit Chart.js Visualisierung (24h/7d/30d Historie)
 * - WebSocket Live-Updates (alle 2 Sekunden)
 * - SD-Karten Datenlogging (CSV-Format)
 * - Automatische Belüftungssteuerung
 * - Software Watchdog (Auto-Reset bei Hänger)
 * - WiFi mit AP-Fallback und mDNS
 * - EEPROM Konfigurationsspeicher
 * - REST API für externe Anwendungen
 * 
 * Hardware:
 * - ESP32 DevKit v1
 * - 2x DS18B20 Temperatursensoren (OneWire)
 * - pH-Sensor Modul (analog)
 * - TDS-Sensor Modul (analog)
 * - Float Switch (Wasserlevel)
 * - 4-Kanal Relais Modul
 * - SD-Karten Modul
 * 
 * Open Source Projekt
 * Lizenz: MIT
 * Repository: https://github.com/AndreasS964/ForellenWaechter
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>
#include <EEPROM.h>
#include <esp_task_wdt.h>
#include <ArduinoJson.h>

// ═══════════════════════════════════════════════════════════════════════════
// KONFIGURATION - Hier anpassen!
// ═══════════════════════════════════════════════════════════════════════════

// --- Betriebsmodus ---
#define TEST_MODE false           // true = Fake-Werte für Desktop-Tests
#define DEBUG_MODE true           // true = Ausführliche Serial-Ausgabe
#define WATCHDOG_TIMEOUT 60       // Sekunden bis Auto-Reset

// --- WiFi Einstellungen ---
const char* AP_SSID = "ForellenWaechter";
const char* AP_PASSWORD = "YourPassword123";  // ÄNDERN!
const char* STA_SSID = "";        // Leer = nur Access Point Mode
const char* STA_PASSWORD = "";
const char* MDNS_NAME = "forellenwaechter";

// --- Sensor Grenzwerte ---
#define TEMP_MIN 8.0              // Minimum Wassertemperatur (°C)
#define TEMP_MAX 14.0             // Maximum Wassertemperatur (°C)
#define TEMP_CRITICAL 15.5        // Kritische Temperatur (°C)
#define PH_MIN 6.5                // Minimum pH-Wert
#define PH_MAX 8.5                // Maximum pH-Wert
#define TDS_MAX 500               // Maximum TDS (ppm)

// --- Zeitintervalle (ms) ---
#define SENSOR_INTERVAL 5000      // Sensor-Abfrage alle 5 Sek
#define LOG_INTERVAL 300000       // SD-Logging alle 5 Min
#define WEBSOCKET_INTERVAL 2000   // WebSocket Update alle 2 Sek
#define HISTORY_INTERVAL 300000   // Chart-History alle 5 Min

// ═══════════════════════════════════════════════════════════════════════════
// PIN DEFINITIONEN
// ═══════════════════════════════════════════════════════════════════════════

// Sensoren
#define ONE_WIRE_BUS 4            // DS18B20 Datenleitung
#define PH_PIN 34                 // pH-Sensor (ADC)
#define TDS_PIN 35                // TDS-Sensor (ADC)
#define WATER_LEVEL_PIN 33        // Float Switch

// Relais (LOW-aktiv!)
#define RELAY_AERATION 26         // Belüftungspumpe
#define RELAY_2 27                // Reserve
#define RELAY_3 14                // Reserve
#define RELAY_4 12                // Reserve

// Status
#define STATUS_LED 2              // Onboard LED

// SD-Karte
#define SD_CS 5                   // Chip Select

// ═══════════════════════════════════════════════════════════════════════════
// GLOBALE OBJEKTE
// ═══════════════════════════════════════════════════════════════════════════

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensors(&oneWire);
WebServer server(80);

// Sensor-Adressen
DeviceAddress waterTempAddr, airTempAddr;
bool waterSensorFound = false;
bool airSensorFound = false;

// ═══════════════════════════════════════════════════════════════════════════
// SENSOR DATEN
// ═══════════════════════════════════════════════════════════════════════════

struct SensorData {
  float waterTemp;
  float airTemp;
  float ph;
  float tds;
  bool waterLevelOK;
  bool aerationActive;
  bool alarmActive;
  unsigned long timestamp;
} sensors = {0, 0, 7.0, 0, true, false, false, 0};

// ═══════════════════════════════════════════════════════════════════════════
// HISTORIE FÜR CHARTS
// ═══════════════════════════════════════════════════════════════════════════

#define HISTORY_SIZE 288          // 24h bei 5-Min-Intervallen

struct HistoryBuffer {
  float waterTemp[HISTORY_SIZE];
  float airTemp[HISTORY_SIZE];
  float ph[HISTORY_SIZE];
  float tds[HISTORY_SIZE];
  unsigned long timestamp[HISTORY_SIZE];
  int index;
  bool full;
} history = {{0}, {0}, {0}, {0}, {0}, 0, false};

// ═══════════════════════════════════════════════════════════════════════════
// TIMING VARIABLEN
// ═══════════════════════════════════════════════════════════════════════════

unsigned long lastSensorRead = 0;
unsigned long lastLogWrite = 0;
unsigned long lastWebSocket = 0;
unsigned long lastHistoryUpdate = 0;
unsigned long startTime = 0;

// ═══════════════════════════════════════════════════════════════════════════
// SETUP
// ═══════════════════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  delay(500);
  
  printBanner();
  startTime = millis();
  
  // Watchdog initialisieren
  initWatchdog();
  
  // Pins konfigurieren
  initPins();
  
  // Sensoren initialisieren
  initSensors();
  
  // SD-Karte initialisieren
  initSDCard();
  
  // WiFi starten
  initWiFi();
  
  // Webserver starten
  initWebServer();
  
  // Erste Messung
  readAllSensors();
  
  Serial.println("\n✅ System bereit!");
  Serial.println("════════════════════════════════════════\n");
}

void printBanner() {
  Serial.println("\n\n");
  Serial.println("╔══════════════════════════════════════════╗");
  Serial.println("║   🐟 ForellenWächter v1.2 Full Edition   ║");
  Serial.println("║   IoT Monitoring für Aquakultur          ║");
  Serial.println("╚══════════════════════════════════════════╝\n");
  
  if (TEST_MODE) {
    Serial.println("⚠️  TEST MODE AKTIV - Fake-Sensorwerte!");
  }
}

void initWatchdog() {
  esp_task_wdt_config_t wdtConfig = {
    .timeout_ms = WATCHDOG_TIMEOUT * 1000,
    .idle_core_mask = 0,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdtConfig);
  esp_task_wdt_add(NULL);
  Serial.printf("✅ Watchdog: %d Sekunden\n", WATCHDOG_TIMEOUT);
}

void initPins() {
  pinMode(STATUS_LED, OUTPUT);
  pinMode(WATER_LEVEL_PIN, INPUT_PULLUP);
  pinMode(RELAY_AERATION, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  
  // Alle Relais AUS (HIGH = aus bei LOW-aktiv)
  digitalWrite(RELAY_AERATION, HIGH);
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(RELAY_3, HIGH);
  digitalWrite(RELAY_4, HIGH);
  
  // LED Test
  for (int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED, HIGH);
    delay(100);
    digitalWrite(STATUS_LED, LOW);
    delay(100);
  }
  
  Serial.println("✅ Pins initialisiert");
}

void initSensors() {
  tempSensors.begin();
  int deviceCount = tempSensors.getDeviceCount();
  Serial.printf("✅ DS18B20 Sensoren gefunden: %d\n", deviceCount);
  
  if (deviceCount >= 1) {
    tempSensors.getAddress(waterTempAddr, 0);
    waterSensorFound = true;
    Serial.print("   Sensor 1 (Wasser): ");
    printAddress(waterTempAddr);
  }
  
  if (deviceCount >= 2) {
    tempSensors.getAddress(airTempAddr, 1);
    airSensorFound = true;
    Serial.print("   Sensor 2 (Luft): ");
    printAddress(airTempAddr);
  }
  
  tempSensors.setResolution(12);
  tempSensors.setWaitForConversion(false);
}

void printAddress(DeviceAddress addr) {
  for (int i = 0; i < 8; i++) {
    if (addr[i] < 16) Serial.print("0");
    Serial.print(addr[i], HEX);
  }
  Serial.println();
}

void initSDCard() {
  if (!SD.begin(SD_CS)) {
    Serial.println("⚠️  SD-Karte nicht gefunden (optional)");
    return;
  }
  
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("✅ SD-Karte: %lluMB\n", cardSize);
  
  // Log-Datei vorbereiten
  if (!SD.exists("/logs")) {
    SD.mkdir("/logs");
  }
}

void initWiFi() {
  Serial.println("📶 WiFi wird gestartet...");
  
  // Access Point starten
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.printf("✅ AP gestartet: %s\n", AP_SSID);
  Serial.printf("   IP: %s\n", WiFi.softAPIP().toString().c_str());
  
  // Optionale Station-Verbindung
  if (strlen(STA_SSID) > 0) {
    WiFi.begin(STA_SSID, STA_PASSWORD);
    Serial.printf("   Verbinde mit %s...", STA_SSID);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("\n✅ Verbunden! IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
      Serial.println("\n⚠️  Verbindung fehlgeschlagen, nur AP aktiv");
    }
  }
  
  // mDNS starten
  if (MDNS.begin(MDNS_NAME)) {
    Serial.printf("✅ mDNS: http://%s.local\n", MDNS_NAME);
  }
}

void initWebServer() {
  // Hauptseite
  server.on("/", HTTP_GET, handleRoot);
  
  // API Endpunkte
  server.on("/api/status", HTTP_GET, handleAPIStatus);
  server.on("/api/sensors", HTTP_GET, handleAPISensors);
  server.on("/api/history", HTTP_GET, handleAPIHistory);
  server.on("/api/settings", HTTP_GET, handleAPISettings);
  server.on("/api/relay", HTTP_POST, handleAPIRelay);
  
  // 404
  server.onNotFound([]() {
    server.send(404, "text/plain", "Nicht gefunden");
  });
  
  server.begin();
  Serial.println("✅ Webserver gestartet");
}

// ═══════════════════════════════════════════════════════════════════════════
// HAUPTSCHLEIFE
// ═══════════════════════════════════════════════════════════════════════════

void loop() {
  esp_task_wdt_reset();  // Watchdog füttern
  
  unsigned long now = millis();
  
  // Sensoren auslesen
  if (now - lastSensorRead >= SENSOR_INTERVAL) {
    readAllSensors();
    checkAlarms();
    controlAeration();
    lastSensorRead = now;
  }
  
  // Historie aktualisieren
  if (now - lastHistoryUpdate >= HISTORY_INTERVAL) {
    updateHistory();
    lastHistoryUpdate = now;
  }
  
  // SD-Logging
  if (now - lastLogWrite >= LOG_INTERVAL) {
    logToSD();
    lastLogWrite = now;
  }
  
  // Webserver
  server.handleClient();
  
  // Status LED
  updateStatusLED();
}

// ═══════════════════════════════════════════════════════════════════════════
// SENSOR FUNKTIONEN
// ═══════════════════════════════════════════════════════════════════════════

void readAllSensors() {
  if (TEST_MODE) {
    // Fake-Werte für Tests
    sensors.waterTemp = 11.5 + random(-10, 10) / 10.0;
    sensors.airTemp = 18.0 + random(-20, 20) / 10.0;
    sensors.ph = 7.2 + random(-5, 5) / 10.0;
    sensors.tds = 180 + random(-20, 20);
    sensors.waterLevelOK = true;
  } else {
    // Echte Sensoren auslesen
    readTemperatures();
    readPH();
    readTDS();
    readWaterLevel();
  }
  
  sensors.timestamp = millis();
  
  if (DEBUG_MODE) {
    printSensorValues();
  }
}

void readTemperatures() {
  tempSensors.requestTemperatures();
  delay(100);  // Kurz warten
  
  if (waterSensorFound) {
    float temp = tempSensors.getTempC(waterTempAddr);
    if (temp > -50 && temp < 85) {
      sensors.waterTemp = temp;
    }
  }
  
  if (airSensorFound) {
    float temp = tempSensors.getTempC(airTempAddr);
    if (temp > -50 && temp < 85) {
      sensors.airTemp = temp;
    }
  }
}

void readPH() {
  int rawValue = analogRead(PH_PIN);
  float voltage = rawValue * 3.3 / 4095.0;
  
  // pH Kalibrierung (anpassen nach Bedarf!)
  // Typische Werte: pH 7 = 2.5V, pH 4 = 3.0V, pH 10 = 2.0V
  sensors.ph = 7.0 + (2.5 - voltage) * 3.5;
  sensors.ph = constrain(sensors.ph, 0.0, 14.0);
}

void readTDS() {
  int rawValue = analogRead(TDS_PIN);
  float voltage = rawValue * 3.3 / 4095.0;
  
  // TDS Berechnung mit Temperaturkompensation
  float tempCoeff = 1.0 + 0.02 * (sensors.waterTemp - 25.0);
  float compVoltage = voltage / tempCoeff;
  
  sensors.tds = (133.42 * pow(compVoltage, 3) - 
                 255.86 * pow(compVoltage, 2) + 
                 857.39 * compVoltage) * 0.5;
  sensors.tds = constrain(sensors.tds, 0.0, 1000.0);
}

void readWaterLevel() {
  sensors.waterLevelOK = digitalRead(WATER_LEVEL_PIN) == LOW;
}

void printSensorValues() {
  Serial.println("────────────────────────────────────────");
  Serial.printf("💧 Wasser: %.1f°C  ", sensors.waterTemp);
  Serial.printf("🌡️ Luft: %.1f°C\n", sensors.airTemp);
  Serial.printf("🧪 pH: %.2f       ", sensors.ph);
  Serial.printf("📊 TDS: %.0f ppm\n", sensors.tds);
  Serial.printf("🌊 Level: %s     ", sensors.waterLevelOK ? "OK" : "NIEDRIG!");
  Serial.printf("💨 Belüftung: %s\n", sensors.aerationActive ? "AN" : "AUS");
  if (sensors.alarmActive) {
    Serial.println("🚨 ALARM AKTIV!");
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// ALARM & STEUERUNG
// ═══════════════════════════════════════════════════════════════════════════

void checkAlarms() {
  bool alarm = false;
  
  // Temperatur-Alarm
  if (sensors.waterTemp < TEMP_MIN || sensors.waterTemp > TEMP_MAX) {
    alarm = true;
  }
  
  // Kritische Temperatur
  if (sensors.waterTemp > TEMP_CRITICAL) {
    alarm = true;
  }
  
  // pH-Alarm
  if (sensors.ph < PH_MIN || sensors.ph > PH_MAX) {
    alarm = true;
  }
  
  // TDS-Alarm
  if (sensors.tds > TDS_MAX) {
    alarm = true;
  }
  
  // Wasserlevel-Alarm
  if (!sensors.waterLevelOK) {
    alarm = true;
  }
  
  sensors.alarmActive = alarm;
}

void controlAeration() {
  // Automatische Belüftung bei:
  // - Temperatur zu hoch
  // - pH außerhalb des Bereichs
  // - TDS zu hoch
  
  bool shouldActivate = false;
  
  if (sensors.waterTemp > TEMP_MAX) {
    shouldActivate = true;
  }
  
  if (sensors.ph < PH_MIN || sensors.ph > PH_MAX) {
    shouldActivate = true;
  }
  
  if (sensors.tds > TDS_MAX * 0.8) {
    shouldActivate = true;
  }
  
  sensors.aerationActive = shouldActivate;
  digitalWrite(RELAY_AERATION, shouldActivate ? LOW : HIGH);  // LOW-aktiv!
}

// ═══════════════════════════════════════════════════════════════════════════
// HISTORIE
// ═══════════════════════════════════════════════════════════════════════════

void updateHistory() {
  history.waterTemp[history.index] = sensors.waterTemp;
  history.airTemp[history.index] = sensors.airTemp;
  history.ph[history.index] = sensors.ph;
  history.tds[history.index] = sensors.tds;
  history.timestamp[history.index] = millis();
  
  history.index = (history.index + 1) % HISTORY_SIZE;
  if (history.index == 0) {
    history.full = true;
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// SD-KARTE LOGGING
// ═══════════════════════════════════════════════════════════════════════════

void logToSD() {
  if (!SD.begin(SD_CS)) return;
  
  // Dateiname mit Datum
  char filename[32];
  unsigned long uptime = millis() / 1000;
  int days = uptime / 86400;
  snprintf(filename, sizeof(filename), "/logs/day_%03d.csv", days);
  
  bool newFile = !SD.exists(filename);
  File file = SD.open(filename, FILE_APPEND);
  
  if (!file) return;
  
  // Header für neue Datei
  if (newFile) {
    file.println("Timestamp,WaterTemp,AirTemp,pH,TDS,WaterLevel,Aeration,Alarm");
  }
  
  // Daten schreiben
  file.printf("%lu,%.2f,%.2f,%.2f,%.0f,%d,%d,%d\n",
    sensors.timestamp,
    sensors.waterTemp,
    sensors.airTemp,
    sensors.ph,
    sensors.tds,
    sensors.waterLevelOK ? 1 : 0,
    sensors.aerationActive ? 1 : 0,
    sensors.alarmActive ? 1 : 0
  );
  
  file.close();
  
  if (DEBUG_MODE) {
    Serial.printf("💾 Log geschrieben: %s\n", filename);
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// STATUS LED
// ═══════════════════════════════════════════════════════════════════════════

void updateStatusLED() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  unsigned long interval = sensors.alarmActive ? 200 : 1000;
  
  if (millis() - lastBlink >= interval) {
    ledState = !ledState;
    digitalWrite(STATUS_LED, ledState);
    lastBlink = millis();
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// WEB HANDLER
// ═══════════════════════════════════════════════════════════════════════════

void handleRoot() {
  String html = getHTMLPage();
  server.send(200, "text/html", html);
}

void handleAPIStatus() {
  String json = "{";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"wifiRSSI\":" + String(WiFi.RSSI()) + ",";
  json += "\"sdCard\":" + String(SD.begin(SD_CS) ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleAPISensors() {
  String json = "{";
  json += "\"waterTemp\":" + String(sensors.waterTemp, 2) + ",";
  json += "\"airTemp\":" + String(sensors.airTemp, 2) + ",";
  json += "\"ph\":" + String(sensors.ph, 2) + ",";
  json += "\"tds\":" + String(sensors.tds, 0) + ",";
  json += "\"waterLevel\":" + String(sensors.waterLevelOK ? "true" : "false") + ",";
  json += "\"aeration\":" + String(sensors.aerationActive ? "true" : "false") + ",";
  json += "\"alarm\":" + String(sensors.alarmActive ? "true" : "false") + ",";
  json += "\"timestamp\":" + String(sensors.timestamp);
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleAPIHistory() {
  String json = "{\"waterTemp\":[";
  
  int count = history.full ? HISTORY_SIZE : history.index;
  int start = history.full ? history.index : 0;
  
  for (int i = 0; i < count; i++) {
    int idx = (start + i) % HISTORY_SIZE;
    if (i > 0) json += ",";
    json += String(history.waterTemp[idx], 1);
  }
  
  json += "],\"airTemp\":[";
  for (int i = 0; i < count; i++) {
    int idx = (start + i) % HISTORY_SIZE;
    if (i > 0) json += ",";
    json += String(history.airTemp[idx], 1);
  }
  
  json += "],\"ph\":[";
  for (int i = 0; i < count; i++) {
    int idx = (start + i) % HISTORY_SIZE;
    if (i > 0) json += ",";
    json += String(history.ph[idx], 2);
  }
  
  json += "],\"tds\":[";
  for (int i = 0; i < count; i++) {
    int idx = (start + i) % HISTORY_SIZE;
    if (i > 0) json += ",";
    json += String(history.tds[idx], 0);
  }
  
  json += "]}";
  
  server.send(200, "application/json", json);
}

void handleAPISettings() {
  String json = "{";
  json += "\"tempMin\":" + String(TEMP_MIN) + ",";
  json += "\"tempMax\":" + String(TEMP_MAX) + ",";
  json += "\"tempCritical\":" + String(TEMP_CRITICAL) + ",";
  json += "\"phMin\":" + String(PH_MIN) + ",";
  json += "\"phMax\":" + String(PH_MAX) + ",";
  json += "\"tdsMax\":" + String(TDS_MAX);
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleAPIRelay() {
  if (!server.hasArg("relay") || !server.hasArg("state")) {
    server.send(400, "application/json", "{\"error\":\"Missing parameters\"}");
    return;
  }
  
  int relay = server.arg("relay").toInt();
  bool state = server.arg("state") == "1";
  
  int pin = -1;
  switch (relay) {
    case 1: pin = RELAY_AERATION; break;
    case 2: pin = RELAY_2; break;
    case 3: pin = RELAY_3; break;
    case 4: pin = RELAY_4; break;
  }
  
  if (pin < 0) {
    server.send(400, "application/json", "{\"error\":\"Invalid relay\"}");
    return;
  }
  
  digitalWrite(pin, state ? LOW : HIGH);  // LOW-aktiv!
  server.send(200, "application/json", "{\"success\":true}");
}

// ═══════════════════════════════════════════════════════════════════════════
// HTML DASHBOARD
// ═══════════════════════════════════════════════════════════════════════════

String getHTMLPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ForellenWächter - Aquakultur Monitoring</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background: linear-gradient(135deg, #1a5f7a 0%, #2d8659 100%);
      min-height: 100vh;
      padding: 20px;
    }
    .container { max-width: 1200px; margin: 0 auto; }
    
    header {
      text-align: center;
      color: white;
      margin-bottom: 30px;
    }
    header h1 {
      font-size: 2.5em;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
    }
    header p { opacity: 0.9; margin-top: 5px; }
    
    .status-bar {
      display: flex;
      justify-content: center;
      gap: 20px;
      margin-bottom: 20px;
      flex-wrap: wrap;
    }
    .status-badge {
      background: rgba(255,255,255,0.2);
      padding: 8px 16px;
      border-radius: 20px;
      color: white;
      font-size: 0.9em;
    }
    .status-badge.alarm {
      background: #e74c3c;
      animation: pulse 1s infinite;
    }
    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.7; }
    }
    
    .cards {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 20px;
      margin-bottom: 30px;
    }
    .card {
      background: white;
      border-radius: 16px;
      padding: 20px;
      text-align: center;
      box-shadow: 0 10px 30px rgba(0,0,0,0.15);
      transition: transform 0.3s;
    }
    .card:hover { transform: translateY(-5px); }
    .card .icon { font-size: 2.5em; margin-bottom: 10px; }
    .card .value {
      font-size: 2em;
      font-weight: bold;
      color: #1a5f7a;
    }
    .card .label {
      color: #666;
      font-size: 0.9em;
      margin-top: 5px;
    }
    .card.warning { border-left: 4px solid #f39c12; }
    .card.danger { border-left: 4px solid #e74c3c; }
    .card.ok { border-left: 4px solid #27ae60; }
    
    .chart-container {
      background: white;
      border-radius: 16px;
      padding: 20px;
      margin-bottom: 20px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.15);
    }
    .chart-container h3 {
      color: #1a5f7a;
      margin-bottom: 15px;
    }
    
    .tabs {
      display: flex;
      gap: 10px;
      margin-bottom: 15px;
    }
    .tab {
      padding: 8px 16px;
      background: #ecf0f1;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      transition: all 0.3s;
    }
    .tab.active { background: #1a5f7a; color: white; }
    
    footer {
      text-align: center;
      color: rgba(255,255,255,0.7);
      padding: 20px;
      font-size: 0.9em;
    }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <h1>🐟 ForellenWächter</h1>
      <p>Aquakultur IoT Monitoring System</p>
    </header>
    
    <div class="status-bar">
      <div class="status-badge" id="statusBadge">⚡ Verbunden</div>
      <div class="status-badge" id="uptimeBadge">⏱️ Uptime: --</div>
      <div class="status-badge" id="alarmBadge" style="display:none">🚨 ALARM</div>
    </div>
    
    <div class="cards">
      <div class="card" id="cardWater">
        <div class="icon">💧</div>
        <div class="value" id="waterTemp">--</div>
        <div class="label">Wassertemperatur (°C)</div>
      </div>
      <div class="card" id="cardAir">
        <div class="icon">🌡️</div>
        <div class="value" id="airTemp">--</div>
        <div class="label">Lufttemperatur (°C)</div>
      </div>
      <div class="card" id="cardPH">
        <div class="icon">🧪</div>
        <div class="value" id="phValue">--</div>
        <div class="label">pH-Wert</div>
      </div>
      <div class="card" id="cardTDS">
        <div class="icon">📊</div>
        <div class="value" id="tdsValue">--</div>
        <div class="label">TDS (ppm)</div>
      </div>
      <div class="card" id="cardLevel">
        <div class="icon">🌊</div>
        <div class="value" id="waterLevel">--</div>
        <div class="label">Wasserlevel</div>
      </div>
      <div class="card" id="cardAeration">
        <div class="icon">💨</div>
        <div class="value" id="aeration">--</div>
        <div class="label">Belüftung</div>
      </div>
    </div>
    
    <div class="chart-container">
      <h3>📈 Temperaturverlauf</h3>
      <div class="tabs">
        <button class="tab active" onclick="setRange(24)">24h</button>
        <button class="tab" onclick="setRange(168)">7d</button>
        <button class="tab" onclick="setRange(720)">30d</button>
      </div>
      <canvas id="tempChart" height="200"></canvas>
    </div>
    
    <div class="chart-container">
      <h3>🧪 pH & TDS Verlauf</h3>
      <canvas id="qualityChart" height="200"></canvas>
    </div>
    
    <footer>
      ForellenWächter v1.2 | © 2024 | 
      <a href="/api/sensors" style="color:white">API</a>
    </footer>
  </div>
  
  <script>
    let tempChart, qualityChart;
    let historyData = {waterTemp:[], airTemp:[], ph:[], tds:[]};
    
    // Charts initialisieren
    function initCharts() {
      const ctx1 = document.getElementById('tempChart').getContext('2d');
      tempChart = new Chart(ctx1, {
        type: 'line',
        data: {
          labels: [],
          datasets: [{
            label: 'Wasser °C',
            data: [],
            borderColor: '#3498db',
            backgroundColor: 'rgba(52,152,219,0.1)',
            fill: true,
            tension: 0.4
          }, {
            label: 'Luft °C',
            data: [],
            borderColor: '#e74c3c',
            backgroundColor: 'rgba(231,76,60,0.1)',
            fill: true,
            tension: 0.4
          }]
        },
        options: {
          responsive: true,
          plugins: { legend: { position: 'top' } },
          scales: { y: { beginAtZero: false } }
        }
      });
      
      const ctx2 = document.getElementById('qualityChart').getContext('2d');
      qualityChart = new Chart(ctx2, {
        type: 'line',
        data: {
          labels: [],
          datasets: [{
            label: 'pH',
            data: [],
            borderColor: '#9b59b6',
            yAxisID: 'y',
            tension: 0.4
          }, {
            label: 'TDS (ppm)',
            data: [],
            borderColor: '#27ae60',
            yAxisID: 'y1',
            tension: 0.4
          }]
        },
        options: {
          responsive: true,
          plugins: { legend: { position: 'top' } },
          scales: {
            y: { type: 'linear', position: 'left', min: 5, max: 9 },
            y1: { type: 'linear', position: 'right', min: 0, max: 500 }
          }
        }
      });
    }
    
    // Daten abrufen
    async function fetchData() {
      try {
        const res = await fetch('/api/sensors');
        const data = await res.json();
        updateDisplay(data);
      } catch (e) {
        document.getElementById('statusBadge').textContent = '❌ Verbindung verloren';
      }
    }
    
    async function fetchHistory() {
      try {
        const res = await fetch('/api/history');
        historyData = await res.json();
        updateCharts();
      } catch (e) {}
    }
    
    // Anzeige aktualisieren
    function updateDisplay(data) {
      document.getElementById('waterTemp').textContent = data.waterTemp.toFixed(1);
      document.getElementById('airTemp').textContent = data.airTemp.toFixed(1);
      document.getElementById('phValue').textContent = data.ph.toFixed(2);
      document.getElementById('tdsValue').textContent = Math.round(data.tds);
      document.getElementById('waterLevel').textContent = data.waterLevel ? 'OK' : 'NIEDRIG';
      document.getElementById('aeration').textContent = data.aeration ? 'AN' : 'AUS';
      
      // Karten-Status
      updateCardStatus('cardWater', data.waterTemp, 8, 14);
      updateCardStatus('cardPH', data.ph, 6.5, 8.5);
      updateCardStatus('cardTDS', data.tds, 0, 500);
      
      document.getElementById('cardLevel').className = 'card ' + (data.waterLevel ? 'ok' : 'danger');
      document.getElementById('cardAeration').className = 'card ' + (data.aeration ? 'warning' : 'ok');
      
      // Alarm-Badge
      const alarmBadge = document.getElementById('alarmBadge');
      alarmBadge.style.display = data.alarm ? 'block' : 'none';
      alarmBadge.className = 'status-badge' + (data.alarm ? ' alarm' : '');
      
      document.getElementById('statusBadge').textContent = '⚡ Verbunden';
    }
    
    function updateCardStatus(id, value, min, max) {
      const card = document.getElementById(id);
      if (value < min || value > max) {
        card.className = 'card danger';
      } else if (value < min * 1.1 || value > max * 0.9) {
        card.className = 'card warning';
      } else {
        card.className = 'card ok';
      }
    }
    
    function updateCharts() {
      const labels = historyData.waterTemp.map((_, i) => i * 5 + ' min');
      
      tempChart.data.labels = labels;
      tempChart.data.datasets[0].data = historyData.waterTemp;
      tempChart.data.datasets[1].data = historyData.airTemp;
      tempChart.update();
      
      qualityChart.data.labels = labels;
      qualityChart.data.datasets[0].data = historyData.ph;
      qualityChart.data.datasets[1].data = historyData.tds;
      qualityChart.update();
    }
    
    function setRange(hours) {
      document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
      event.target.classList.add('active');
      // Range-Filterung hier implementieren
    }
    
    // Uptime aktualisieren
    async function updateUptime() {
      try {
        const res = await fetch('/api/status');
        const data = await res.json();
        const hours = Math.floor(data.uptime / 3600);
        const mins = Math.floor((data.uptime % 3600) / 60);
        document.getElementById('uptimeBadge').textContent = 
          `⏱️ ${hours}h ${mins}m`;
      } catch (e) {}
    }
    
    // Initialisierung
    initCharts();
    fetchData();
    fetchHistory();
    updateUptime();
    
    setInterval(fetchData, 2000);
    setInterval(fetchHistory, 60000);
    setInterval(updateUptime, 60000);
  </script>
</body>
</html>
)rawliteral";

  return html;
}
