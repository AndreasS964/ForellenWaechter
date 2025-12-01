/*
 * ═══════════════════════════════════════════════════════════════════════════════════
 * ForellenWächter v1.4 - LTE Remote Edition
 * IoT Monitoring System mit Mobilfunk-Anbindung für Forellenzucht
 * ═══════════════════════════════════════════════════════════════════════════════════
 * 
 * NEU in v1.4:
 * - 📡 LTE/4G Mobilfunk (SIM7600E) für Remote-Zugriff
 * - 📧 E-Mail Alarme bei kritischen Werten
 * - 🫧 Dissolved Oxygen Sensor (optional)
 * - 🎨 Komplett überarbeitetes Dashboard
 * - 📊 Erweiterte Datenvisualisierung
 * - 🔄 OTA Updates vorbereitet
 * - 🐟 Fischdatenbank-Vorbereitung
 * 
 * Hardware:
 * - ESP32 DevKit v1 / LILYGO T-SIM7600E
 * - SIM7600E LTE Modul
 * - 2x DS18B20 Temperatursensoren
 * - pH-Sensor, TDS-Sensor
 * - DFRobot Dissolved Oxygen Sensor (optional)
 * - Float Switch, 4-Kanal Relais
 * - SD-Karte für Logging
 * 
 * Stromversorgung: Wasserkraft-Turbine (~200W)
 * 
 * Open Source Projekt
 * Lizenz: MIT
 * Repository: https://github.com/AndreasS964/ForellenWaechter
 * ═══════════════════════════════════════════════════════════════════════════════════
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>
#include <EEPROM.h>
#include <esp_task_wdt.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <time.h>

// ═══════════════════════════════════════════════════════════════════════════════════
// KONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════════════

// --- Betriebsmodus ---
#define TEST_MODE false              // Fake-Werte für Tests
#define DEBUG_MODE true              // Serial-Ausgabe
#define WATCHDOG_TIMEOUT 120         // Sekunden

// --- Feature Toggles ---
#define ENABLE_LTE true              // LTE Mobilfunk
#define ENABLE_WIFI true             // WiFi (parallel zu LTE)
#define ENABLE_EMAIL_ALERTS true     // E-Mail Benachrichtigungen
#define ENABLE_DO_SENSOR false       // Dissolved Oxygen Sensor
#define ENABLE_SD_LOGGING true       // SD-Karten Logging
#define ENABLE_OTA true              // Over-The-Air Updates

// --- WiFi (lokaler Zugriff) ---
const char* AP_SSID = "ForellenWaechter";
const char* AP_PASSWORD = "YourPassword123";  // ÄNDERN!
const char* STA_SSID = "";           // Optional: Heimnetz
const char* STA_PASSWORD = "";
const char* MDNS_NAME = "forellenwaechter";
const char* OTA_PASSWORD = "forellenadmin123";  // ÄNDERN! OTA Update Passwort

// --- LTE Konfiguration ---
#define LTE_APN "internet"           // APN deines Providers (z.B. "internet.t-mobile")
#define LTE_USER ""                  // APN Username (meist leer)
#define LTE_PASS ""                  // APN Passwort (meist leer)

// --- E-Mail Konfiguration (über HTTP Webhook) ---
// Nutze einen kostenlosen Service wie EmailJS, Mailgun, oder eigenen Server
const char* EMAIL_WEBHOOK_URL = "https://maker.ifttt.com/trigger/forellen_alarm/with/key/YOUR_IFTTT_KEY";
const char* EMAIL_RECIPIENT = "your-email@example.com";  // ÄNDERN!
#define EMAIL_COOLDOWN_MIN 30        // Min. Minuten zwischen E-Mails

// --- Sensor Grenzwerte (Regenbogenforelle) ---
struct TroutParameters {
  float tempMin = 8.0;
  float tempMax = 14.0;
  float tempCritical = 16.0;
  float phMin = 6.5;
  float phMax = 8.5;
  float tdsMax = 500;
  float doMin = 6.0;                 // mg/L Sauerstoff
  float doOptimal = 9.0;
} troutParams;

// --- Zeitintervalle (ms) ---
#define SENSOR_INTERVAL 5000         // Sensor-Abfrage
#define LOG_INTERVAL 300000          // SD-Logging (5 min)
#define LTE_CHECK_INTERVAL 60000     // LTE Status (1 min)
#define HISTORY_INTERVAL 300000      // Chart-History (5 min)
#define NTP_SYNC_INTERVAL 3600000    // Zeit-Sync (1h)

// ═══════════════════════════════════════════════════════════════════════════════════
// PIN DEFINITIONEN
// ═══════════════════════════════════════════════════════════════════════════════════

// Temperatursensoren
#define ONE_WIRE_BUS 4               // DS18B20 Data Pin

// Analog Sensoren
#define PH_PIN 34
#define TDS_PIN 35
#define DO_PIN 32                    // Dissolved Oxygen (optional)

// Digital I/O
#define WATER_LEVEL_PIN 33
#define STATUS_LED 2
#define ALARM_LED 13                 // Externe Alarm-LED (rot)
#define BUZZER_PIN 15

// Relais (Active LOW)
#define RELAY_AERATION 26
#define RELAY_ALARM 27
#define RELAY_SPARE1 14
#define RELAY_SPARE2 12

// SD-Karte
#define SD_CS 5

// LTE Modul (SIM7600)
#define LTE_TX 17
#define LTE_RX 16
#define LTE_PWR 25                   // Power Key (nicht bei allen Modulen nötig)

// ═══════════════════════════════════════════════════════════════════════════════════
// GLOBALE OBJEKTE
// ═══════════════════════════════════════════════════════════════════════════════════

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensors(&oneWire);
WebServer server(80);
HardwareSerial LTESerial(1);

DeviceAddress waterTempAddr, airTempAddr;
bool waterSensorFound = false;
bool airSensorFound = false;

// ═══════════════════════════════════════════════════════════════════════════════════
// FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════════════

String getHTML();
String getCSS();
String getJS();

// ═══════════════════════════════════════════════════════════════════════════════════
// DATENSTRUKTUREN
// ═══════════════════════════════════════════════════════════════════════════════════

// Aktuelle Sensordaten
struct SensorData {
  float waterTemp = 0;
  float airTemp = 0;
  float ph = 7.0;
  float tds = 0;
  float dissolvedOxygen = 0;         // mg/L
  bool waterLevelOK = true;
  bool aerationActive = false;
  bool alarmActive = false;
  String alarmReason = "";
  unsigned long timestamp = 0;
} sensors;

// Systemstatus
struct SystemStatus {
  bool wifiConnected = false;
  bool lteConnected = false;
  bool sdCardOK = false;
  int lteSignal = 0;                 // 0-31 (CSQ)
  String lteOperator = "";
  String publicIP = "";
  unsigned long uptime = 0;
  unsigned long lastEmailSent = 0;
  int alarmCount = 0;
  int dailyAlarms = 0;
  String firmwareVersion = "1.4.1";
} sysStatus;

// Kalibrierungsdaten (2-Punkt Kalibrierung)
#define EEPROM_SIZE 256
#define EEPROM_MAGIC 0xF155  // "FISS" (Fisch) als Magic Number

struct CalibrationData {
  uint16_t magic = EEPROM_MAGIC;

  // pH Kalibrierung (2-Punkt)
  float ph_slope = 3.5;              // Standard-Steigung
  float ph_offset = 0.0;             // Standard-Offset
  bool ph_calibrated = false;
  int ph_buffer1_adc = 0;            // pH 4.0 Pufferlösung ADC-Wert
  int ph_buffer2_adc = 0;            // pH 7.0 Pufferlösung ADC-Wert
  float ph_buffer1_value = 4.0;
  float ph_buffer2_value = 7.0;

  // TDS Kalibrierung
  float tds_factor = 0.5;            // Standard-Faktor
  bool tds_calibrated = false;
  int tds_reference_adc = 0;         // z.B. 1413 µS/cm Lösung
  float tds_reference_value = 1413.0;

  // DO Kalibrierung (optional)
  float do_slope = 1.0;
  float do_offset = 0.0;
  bool do_calibrated = false;

  // Checksumme
  uint8_t checksum = 0;
} calibration;

// Historie für Charts
#define HISTORY_SIZE 288             // 24h bei 5min Intervall
struct HistoryBuffer {
  float waterTemp[HISTORY_SIZE];
  float airTemp[HISTORY_SIZE];
  float ph[HISTORY_SIZE];
  float tds[HISTORY_SIZE];
  float dissolvedOxygen[HISTORY_SIZE];
  unsigned long timestamp[HISTORY_SIZE];
  int index = 0;
  bool full = false;
} history;

// Timing
unsigned long lastSensorRead = 0;
unsigned long lastLogWrite = 0;
unsigned long lastLTECheck = 0;
unsigned long lastHistoryUpdate = 0;
unsigned long lastNTPSync = 0;
unsigned long startTime = 0;

// ═══════════════════════════════════════════════════════════════════════════════════
// SETUP
// ═══════════════════════════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  delay(500);

  printBanner();
  startTime = millis();

  initWatchdog();
  esp_task_wdt_reset();  // Watchdog zurücksetzen nach Init

  initPins();
  initEEPROM();
  loadCalibration();
  initSensors();
  initSDCard();
  esp_task_wdt_reset();  // Watchdog zurücksetzen nach Sensor-Init

  if (ENABLE_WIFI) {
    initWiFi();
    esp_task_wdt_reset();  // Watchdog zurücksetzen nach WiFi-Init
    if (ENABLE_OTA) {
      initOTA();
      esp_task_wdt_reset();  // Watchdog zurücksetzen nach OTA-Init
    }
  }

  if (ENABLE_LTE) {
    initLTE();
    esp_task_wdt_reset();  // Watchdog zurücksetzen nach LTE-Init
  }

  initWebServer();
  esp_task_wdt_reset();  // Watchdog zurücksetzen nach WebServer-Init

  // Erste Messung
  readAllSensors();
  esp_task_wdt_reset();  // Watchdog zurücksetzen nach Sensor-Read

  // NTP-Sync wird später in loop() durchgeführt (nicht in setup(), um Watchdog zu vermeiden)
  lastNTPSync = millis() - NTP_SYNC_INTERVAL + 30000; // Erstes Sync nach 30 Sekunden

  Serial.println("\n✅ ForellenWächter v1.4 bereit!");
  Serial.println("══════════════════════════════════════════════\n");
  
  // Startup-Benachrichtigung
  if (ENABLE_EMAIL_ALERTS && (sysStatus.lteConnected || sysStatus.wifiConnected)) {
    // sendEmail("System gestartet", "ForellenWächter wurde neu gestartet.");
  }
}

void printBanner() {
  Serial.println("\n");
  Serial.println("╔══════════════════════════════════════════════════════╗");
  Serial.println("║   🐟 ForellenWächter v1.4 - LTE Remote Edition       ║");
  Serial.println("║   IoT Monitoring System für Aquakultur              ║");
  Serial.println("╚══════════════════════════════════════════════════════╝\n");
  
  if (TEST_MODE) Serial.println("⚠️  TEST MODE AKTIV");
  Serial.printf("Features: WiFi=%d LTE=%d Email=%d DO=%d SD=%d\n",
    ENABLE_WIFI, ENABLE_LTE, ENABLE_EMAIL_ALERTS, ENABLE_DO_SENSOR, ENABLE_SD_LOGGING);
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
  // LEDs
  pinMode(STATUS_LED, OUTPUT);
  pinMode(ALARM_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Inputs
  pinMode(WATER_LEVEL_PIN, INPUT_PULLUP);
  
  // Relais (alle AUS = HIGH bei Active LOW)
  pinMode(RELAY_AERATION, OUTPUT);
  pinMode(RELAY_ALARM, OUTPUT);
  pinMode(RELAY_SPARE1, OUTPUT);
  pinMode(RELAY_SPARE2, OUTPUT);
  
  digitalWrite(RELAY_AERATION, HIGH);
  digitalWrite(RELAY_ALARM, HIGH);
  digitalWrite(RELAY_SPARE1, HIGH);
  digitalWrite(RELAY_SPARE2, HIGH);
  
  // Startup-Sequenz
  for (int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED, HIGH);
    delay(100);
    digitalWrite(STATUS_LED, LOW);
    delay(100);
  }
  
  Serial.println("✅ Pins initialisiert");
}

void initSensors() {
  // DS18B20
  tempSensors.begin();
  int count = tempSensors.getDeviceCount();
  Serial.printf("✅ DS18B20 Sensoren: %d\n", count);
  
  if (count >= 1) {
    tempSensors.getAddress(waterTempAddr, 0);
    waterSensorFound = true;
  }
  if (count >= 2) {
    tempSensors.getAddress(airTempAddr, 1);
    airSensorFound = true;
  }
  
  tempSensors.setResolution(12);
  tempSensors.setWaitForConversion(false);
  
  // DO Sensor Initialisierung (falls aktiviert)
  if (ENABLE_DO_SENSOR) {
    pinMode(DO_PIN, INPUT);
    Serial.println("✅ DO-Sensor aktiviert");
  }
}

void initSDCard() {
  if (!ENABLE_SD_LOGGING) return;
  
  if (!SD.begin(SD_CS)) {
    Serial.println("⚠️  SD-Karte nicht gefunden");
    sysStatus.sdCardOK = false;
    return;
  }
  
  sysStatus.sdCardOK = true;
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("✅ SD-Karte: %lluMB\n", cardSize);
  
  // Verzeichnisse anlegen
  if (!SD.exists("/logs")) SD.mkdir("/logs");
  if (!SD.exists("/data")) SD.mkdir("/data");
}

void initWiFi() {
  Serial.println("📶 WiFi wird gestartet...");
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.printf("✅ AP: %s (IP: %s)\n", AP_SSID, WiFi.softAPIP().toString().c_str());
  
  // AP-Mode ist immer aktiv -> WiFi "connected" für lokalen Zugriff
  sysStatus.wifiConnected = true;
  
  if (strlen(STA_SSID) > 0) {
    WiFi.begin(STA_SSID, STA_PASSWORD);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("\n✅ WiFi verbunden: %s\n", WiFi.localIP().toString().c_str());
    } else {
      Serial.println("\n⚠️  WiFi STA nicht verbunden (AP läuft weiter)");
    }
  }
  
  if (MDNS.begin(MDNS_NAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("✅ mDNS: http://%s.local\n", MDNS_NAME);
  }
}

void initOTA() {
  Serial.println("🔄 OTA Updates aktiviert...");

  ArduinoOTA.setHostname(MDNS_NAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "Sketch" : "Filesystem";
    Serial.println("\n🔄 OTA Update gestartet: " + type);

    // SD-Karte sicher beenden
    if (sysStatus.sdCardOK) {
      SD.end();
    }
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\n✅ OTA Update abgeschlossen!");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static unsigned int lastPercent = 0;
    unsigned int percent = (progress / (total / 100));
    if (percent != lastPercent && percent % 10 == 0) {
      Serial.printf("   Progress: %u%%\n", percent);
      lastPercent = percent;
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("❌ OTA Fehler [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  Serial.println("✅ OTA bereit!");
}

// ═══════════════════════════════════════════════════════════════════════════════════
// EEPROM & KALIBRIERUNG
// ═══════════════════════════════════════════════════════════════════════════════════

void initEEPROM() {
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("⚠️  EEPROM Init fehlgeschlagen!");
    return;
  }
  Serial.println("✅ EEPROM initialisiert");
}

uint8_t calculateChecksum(const CalibrationData& data) {
  uint8_t checksum = 0;
  const uint8_t* ptr = (const uint8_t*)&data;
  for (size_t i = 0; i < sizeof(CalibrationData) - 1; i++) {  // -1 um checksum selbst auszulassen
    checksum ^= ptr[i];
  }
  return checksum;
}

void loadCalibration() {
  Serial.println("📂 Lade Kalibrierungsdaten...");

  EEPROM.get(0, calibration);

  if (calibration.magic != EEPROM_MAGIC) {
    Serial.println("   Keine gültigen Daten, verwende Standardwerte");
    calibration.magic = EEPROM_MAGIC;
    saveCalibration();
    return;
  }

  uint8_t checksum = calculateChecksum(calibration);
  if (checksum != calibration.checksum) {
    Serial.println("⚠️  Checksumme ungültig, verwende Standardwerte");
    return;
  }

  Serial.println("✅ Kalibrierungsdaten geladen");
  if (calibration.ph_calibrated) {
    Serial.println("   pH: kalibriert ✓");
  }
  if (calibration.tds_calibrated) {
    Serial.println("   TDS: kalibriert ✓");
  }
  if (ENABLE_DO_SENSOR && calibration.do_calibrated) {
    Serial.println("   DO: kalibriert ✓");
  }
}

void saveCalibration() {
  calibration.checksum = calculateChecksum(calibration);
  EEPROM.put(0, calibration);
  EEPROM.commit();
  Serial.println("💾 Kalibrierungsdaten gespeichert");
}

// ═══════════════════════════════════════════════════════════════════════════════════
// LTE FUNKTIONEN
// ═══════════════════════════════════════════════════════════════════════════════════

void initLTE() {
  Serial.println("📡 LTE wird initialisiert...");
  
  LTESerial.begin(115200, SERIAL_8N1, LTE_RX, LTE_TX);
  delay(1000);
  
  // Modul aufwecken (falls nötig)
  sendATCommand("AT", 1000);
  
  // Echo aus
  sendATCommand("ATE0", 500);
  
  // Modul-Info
  String moduleInfo = sendATCommand("ATI", 1000);
  Serial.printf("   Modul: %s\n", moduleInfo.c_str());
  
  // SIM-Status prüfen
  String simStatus = sendATCommand("AT+CPIN?", 1000);
  if (simStatus.indexOf("READY") == -1) {
    Serial.println("⚠️  SIM-Karte nicht bereit!");
    return;
  }
  
  // Netzwerk-Registrierung
  sendATCommand("AT+CREG=1", 500);
  delay(2000);
  
  // APN konfigurieren
  String apnCmd = "AT+CGDCONT=1,\"IP\",\"" + String(LTE_APN) + "\"";
  sendATCommand(apnCmd.c_str(), 1000);
  
  // PDP Context aktivieren
  sendATCommand("AT+CGACT=1,1", 5000);
  
  // Verbindung prüfen
  checkLTEConnection();
  
  if (sysStatus.lteConnected) {
    Serial.println("✅ LTE verbunden!");
    Serial.printf("   Operator: %s\n", sysStatus.lteOperator.c_str());
    Serial.printf("   Signal: %d/31\n", sysStatus.lteSignal);
  } else {
    Serial.println("⚠️  LTE Verbindung fehlgeschlagen");
  }
}

String sendATCommand(const char* cmd, int timeout) {
  LTESerial.println(cmd);
  
  String response = "";
  unsigned long start = millis();
  
  while (millis() - start < timeout) {
    while (LTESerial.available()) {
      char c = LTESerial.read();
      response += c;
    }
    delay(10);
  }
  
  response.trim();
  
  if (DEBUG_MODE && strlen(cmd) > 0) {
    // Serial.printf("AT> %s -> %s\n", cmd, response.c_str());
  }
  
  return response;
}

void checkLTEConnection() {
  // Signalstärke
  String csq = sendATCommand("AT+CSQ", 1000);
  int signalStart = csq.indexOf(": ") + 2;
  int signalEnd = csq.indexOf(",");
  if (signalStart > 1 && signalEnd > signalStart) {
    sysStatus.lteSignal = csq.substring(signalStart, signalEnd).toInt();
  }
  
  // Operator
  String cops = sendATCommand("AT+COPS?", 1000);
  int opStart = cops.indexOf("\"") + 1;
  int opEnd = cops.indexOf("\"", opStart);
  if (opStart > 0 && opEnd > opStart) {
    sysStatus.lteOperator = cops.substring(opStart, opEnd);
  }
  
  // IP-Adresse
  String cgpaddr = sendATCommand("AT+CGPADDR=1", 1000);
  int ipStart = cgpaddr.indexOf("\"") + 1;
  int ipEnd = cgpaddr.indexOf("\"", ipStart);
  if (ipStart > 0 && ipEnd > ipStart) {
    sysStatus.publicIP = cgpaddr.substring(ipStart, ipEnd);
    sysStatus.lteConnected = true;
  } else {
    sysStatus.lteConnected = false;
  }
}

bool sendHTTPRequest(const char* url, const char* payload) {
  // WiFi STA verbunden? (AP reicht nicht für externe URLs)
  bool wifiStaConnected = (WiFi.status() == WL_CONNECTED);
  
  if (!sysStatus.lteConnected && !wifiStaConnected) {
    if (DEBUG_MODE) Serial.println("⚠️  Kein Internet für HTTP Request");
    return false;
  }
  
  // Versuche erst WiFi STA, dann LTE
  if (wifiStaConnected) {
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);  // 10 Sekunden Timeout
    
    int httpCode = http.POST(payload);
    http.end();
    
    if (DEBUG_MODE) Serial.printf("📡 WiFi HTTP: %d\n", httpCode);
    return httpCode >= 200 && httpCode < 300;
  }
  
  // LTE HTTP Request über AT-Befehle (SIM7600)
  if (sysStatus.lteConnected) {
    if (DEBUG_MODE) Serial.println("📡 LTE HTTP Request...");
    
    // HTTP Service starten
    sendATCommand("AT+HTTPINIT", 2000);
    delay(100);
    
    // URL setzen
    String urlCmd = "AT+HTTPPARA=\"URL\",\"" + String(url) + "\"";
    sendATCommand(urlCmd.c_str(), 1000);
    
    // Content-Type setzen
    sendATCommand("AT+HTTPPARA=\"CONTENT\",\"application/json\"", 500);
    
    // Datenlänge angeben und Daten senden
    String dataCmd = "AT+HTTPDATA=" + String(strlen(payload)) + ",10000";
    String dataResponse = sendATCommand(dataCmd.c_str(), 2000);
    
    // Warten auf DOWNLOAD prompt
    if (dataResponse.indexOf("DOWNLOAD") != -1) {
      LTESerial.print(payload);
      delay(1000);
    }
    
    // POST Request ausführen (1 = POST)
    String result = sendATCommand("AT+HTTPACTION=1", 15000);
    
    // HTTP Service beenden
    sendATCommand("AT+HTTPTERM", 1000);
    
    // Prüfen ob 200 OK
    bool success = result.indexOf(",200,") != -1;
    if (DEBUG_MODE) Serial.printf("📡 LTE HTTP: %s\n", success ? "OK" : "FAIL");
    return success;
  }
  
  return false;
}

// ═══════════════════════════════════════════════════════════════════════════════════
// E-MAIL FUNKTIONEN
// ═══════════════════════════════════════════════════════════════════════════════════

void sendEmailAlert(String subject, String message) {
  if (!ENABLE_EMAIL_ALERTS) return;
  
  // Cooldown prüfen (overflow-sicher)
  unsigned long cooldownMs = (unsigned long)EMAIL_COOLDOWN_MIN * 60UL * 1000UL;
  if (sysStatus.lastEmailSent > 0 && (millis() - sysStatus.lastEmailSent) < cooldownMs) {
    if (DEBUG_MODE) Serial.println("📧 E-Mail Cooldown aktiv");
    return;
  }
  
  // JSON Payload für IFTTT/Webhook
  StaticJsonDocument<512> doc;
  doc["value1"] = subject;
  doc["value2"] = message;
  doc["value3"] = getTimestamp();
  
  String payload;
  serializeJson(doc, payload);
  
  if (sendHTTPRequest(EMAIL_WEBHOOK_URL, payload.c_str())) {
    sysStatus.lastEmailSent = millis();
    Serial.printf("📧 E-Mail gesendet: %s\n", subject.c_str());
    logEvent("EMAIL_SENT", subject);
  } else {
    Serial.println("⚠️  E-Mail Versand fehlgeschlagen");
  }
}

void checkAndSendAlerts() {
  if (!sensors.alarmActive) return;
  
  String subject = "🚨 ForellenWächter ALARM";
  String message = "ALARM: " + sensors.alarmReason + "\n\n";
  message += "Aktuelle Werte:\n";
  message += "- Wassertemperatur: " + String(sensors.waterTemp, 1) + "°C\n";
  message += "- Lufttemperatur: " + String(sensors.airTemp, 1) + "°C\n";
  message += "- pH-Wert: " + String(sensors.ph, 2) + "\n";
  message += "- TDS: " + String(sensors.tds, 0) + " ppm\n";
  if (ENABLE_DO_SENSOR) {
    message += "- Sauerstoff: " + String(sensors.dissolvedOxygen, 1) + " mg/L\n";
  }
  message += "- Wasserlevel: " + String(sensors.waterLevelOK ? "OK" : "NIEDRIG") + "\n";
  
  sendEmailAlert(subject, message);
}

// ═══════════════════════════════════════════════════════════════════════════════════
// SENSOR FUNKTIONEN
// ═══════════════════════════════════════════════════════════════════════════════════

void readAllSensors() {
  if (TEST_MODE) {
    generateTestData();
    return;
  }
  
  readTemperatures();
  readPH();
  readTDS();
  readWaterLevel();
  
  if (ENABLE_DO_SENSOR) {
    readDissolvedOxygen();
  }
  
  sensors.timestamp = millis();
  
  if (DEBUG_MODE) {
    printSensorValues();
  }
}

void generateTestData() {
  static float phase = 0;
  phase += 0.1;
  
  sensors.waterTemp = 11.5 + sin(phase) * 2;
  sensors.airTemp = 18.0 + sin(phase * 0.5) * 5;
  sensors.ph = 7.2 + sin(phase * 0.3) * 0.5;
  sensors.tds = 180 + sin(phase * 0.7) * 50;
  sensors.dissolvedOxygen = 8.5 + sin(phase * 0.4) * 1.5;
  sensors.waterLevelOK = true;
  sensors.timestamp = millis();
}

void readTemperatures() {
  tempSensors.requestTemperatures();
  delay(100);
  
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
  // Mehrere Messungen für Stabilität
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(PH_PIN);
    delay(10);
  }
  float rawValue = sum / 10.0;

  // Kalibrierte Messung verwenden
  if (calibration.ph_calibrated) {
    sensors.ph = (calibration.ph_slope * rawValue) + calibration.ph_offset;
  } else {
    // Fallback: Standard-Kalibrierung
    float voltage = rawValue * 3.3 / 4095.0;
    sensors.ph = 7.0 + (2.5 - voltage) * 3.5;
  }

  sensors.ph = constrain(sensors.ph, 0.0, 14.0);
}

void readTDS() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(TDS_PIN);
    delay(10);
  }
  float rawValue = sum / 10.0;

  // Kalibrierte Messung verwenden
  if (calibration.tds_calibrated) {
    // Einfache Faktoren-Multiplikation mit Temperaturkompensation
    float tempCoeff = 1.0 + 0.02 * (sensors.waterTemp - 25.0);
    sensors.tds = (rawValue * calibration.tds_factor) / tempCoeff;
  } else {
    // Fallback: Standard-Formel
    float voltage = rawValue * 3.3 / 4095.0;
    float tempCoeff = 1.0 + 0.02 * (sensors.waterTemp - 25.0);
    float compVoltage = voltage / tempCoeff;
    sensors.tds = (133.42 * pow(compVoltage, 3) -
                   255.86 * pow(compVoltage, 2) +
                   857.39 * compVoltage) * 0.5;
  }

  sensors.tds = constrain(sensors.tds, 0.0, 1000.0);
}

void readDissolvedOxygen() {
  // DFRobot Gravity DO Sensor
  // Kalibrierung erforderlich!
  
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(DO_PIN);
    delay(10);
  }
  float rawValue = sum / 10.0;
  float voltage = rawValue * 3.3 / 4095.0;
  
  // DO Berechnung (vereinfacht, muss kalibriert werden!)
  // Sättigungsspannung bei Kalibrierpunkt
  const float V_SATURATION = 1.5;  // Anpassen!
  const float DO_SATURATION = 9.09; // mg/L bei 20°C
  
  // Temperaturkompensation für Sättigung
  float tempCompensation = 1.0 - 0.024 * (sensors.waterTemp - 20.0);
  float doSaturation = DO_SATURATION * tempCompensation;
  
  sensors.dissolvedOxygen = (voltage / V_SATURATION) * doSaturation;
  sensors.dissolvedOxygen = constrain(sensors.dissolvedOxygen, 0.0, 20.0);
}

void readWaterLevel() {
  sensors.waterLevelOK = digitalRead(WATER_LEVEL_PIN) == LOW;
}

void printSensorValues() {
  Serial.println("────────────────────────────────────────────────");
  Serial.printf("💧 Wasser: %.1f°C  ", sensors.waterTemp);
  Serial.printf("🌡️ Luft: %.1f°C\n", sensors.airTemp);
  Serial.printf("🧪 pH: %.2f       ", sensors.ph);
  Serial.printf("📊 TDS: %.0f ppm\n", sensors.tds);
  if (ENABLE_DO_SENSOR) {
    Serial.printf("🫧 O₂: %.1f mg/L\n", sensors.dissolvedOxygen);
  }
  Serial.printf("🌊 Level: %s     ", sensors.waterLevelOK ? "OK" : "NIEDRIG!");
  Serial.printf("💨 Belüftung: %s\n", sensors.aerationActive ? "AN" : "AUS");
  if (sensors.alarmActive) {
    Serial.printf("🚨 ALARM: %s\n", sensors.alarmReason.c_str());
  }
}

// ═══════════════════════════════════════════════════════════════════════════════════
// ALARM & STEUERUNG
// ═══════════════════════════════════════════════════════════════════════════════════

void checkAlarms() {
  bool alarm = false;
  String reasons = "";
  
  // Temperatur
  if (sensors.waterTemp > troutParams.tempCritical) {
    alarm = true;
    reasons += "Temp KRITISCH (" + String(sensors.waterTemp, 1) + "°C); ";
  } else if (sensors.waterTemp < troutParams.tempMin) {
    alarm = true;
    reasons += "Temp niedrig (" + String(sensors.waterTemp, 1) + "°C); ";
  } else if (sensors.waterTemp > troutParams.tempMax) {
    alarm = true;
    reasons += "Temp hoch (" + String(sensors.waterTemp, 1) + "°C); ";
  }
  
  // pH
  if (sensors.ph < troutParams.phMin) {
    alarm = true;
    reasons += "pH niedrig (" + String(sensors.ph, 2) + "); ";
  } else if (sensors.ph > troutParams.phMax) {
    alarm = true;
    reasons += "pH hoch (" + String(sensors.ph, 2) + "); ";
  }
  
  // TDS
  if (sensors.tds > troutParams.tdsMax) {
    alarm = true;
    reasons += "TDS hoch (" + String(sensors.tds, 0) + "ppm); ";
  }
  
  // Sauerstoff
  if (ENABLE_DO_SENSOR && sensors.dissolvedOxygen < troutParams.doMin) {
    alarm = true;
    reasons += "O2 niedrig (" + String(sensors.dissolvedOxygen, 1) + "mg/L); ";
  }
  
  // Wasserlevel
  if (!sensors.waterLevelOK) {
    alarm = true;
    reasons += "Wasserlevel NIEDRIG; ";
  }
  
  // Trailing "; " entfernen
  if (reasons.length() > 2) {
    reasons = reasons.substring(0, reasons.length() - 2);
  }
  
  // Status aktualisieren
  bool wasAlarm = sensors.alarmActive;
  sensors.alarmActive = alarm;
  sensors.alarmReason = reasons;
  
  // Bei neuem Alarm
  if (alarm && !wasAlarm) {
    sysStatus.alarmCount++;
    sysStatus.dailyAlarms++;
    soundAlarm();
    logEvent("ALARM", reasons);
    checkAndSendAlerts();
  }
  
  // Alarm-LED
  digitalWrite(ALARM_LED, alarm ? HIGH : LOW);
}

void controlAeration() {
  bool shouldActivate = false;
  
  // Automatische Belüftung bei:
  if (sensors.waterTemp > troutParams.tempMax) {
    shouldActivate = true;
  }
  
  if (ENABLE_DO_SENSOR && sensors.dissolvedOxygen < troutParams.doOptimal) {
    shouldActivate = true;
  }
  
  if (sensors.ph < troutParams.phMin || sensors.ph > troutParams.phMax) {
    shouldActivate = true;
  }
  
  sensors.aerationActive = shouldActivate;
  digitalWrite(RELAY_AERATION, shouldActivate ? LOW : HIGH);
}

void soundAlarm() {
  // 3x kurzer Piep
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════════
// HISTORIE & LOGGING
// ═══════════════════════════════════════════════════════════════════════════════════

void updateHistory() {
  history.waterTemp[history.index] = sensors.waterTemp;
  history.airTemp[history.index] = sensors.airTemp;
  history.ph[history.index] = sensors.ph;
  history.tds[history.index] = sensors.tds;
  history.dissolvedOxygen[history.index] = sensors.dissolvedOxygen;
  history.timestamp[history.index] = millis();
  
  history.index = (history.index + 1) % HISTORY_SIZE;
  if (history.index == 0) {
    history.full = true;
  }
}

void logToSD() {
  if (!ENABLE_SD_LOGGING || !sysStatus.sdCardOK) return;
  
  String filename = "/logs/" + getDateString() + ".csv";
  bool newFile = !SD.exists(filename);
  
  File file = SD.open(filename, FILE_APPEND);
  if (!file) return;
  
  // Header für neue Datei
  if (newFile) {
    String header = "Timestamp,WaterTemp,AirTemp,pH,TDS,";
    if (ENABLE_DO_SENSOR) header += "DO,";
    header += "WaterLevel,Aeration,Alarm,AlarmReason";
    file.println(header);
  }
  
  // Daten
  String line = getTimestamp() + ",";
  line += String(sensors.waterTemp, 2) + ",";
  line += String(sensors.airTemp, 2) + ",";
  line += String(sensors.ph, 2) + ",";
  line += String(sensors.tds, 0) + ",";
  if (ENABLE_DO_SENSOR) {
    line += String(sensors.dissolvedOxygen, 2) + ",";
  }
  line += String(sensors.waterLevelOK ? 1 : 0) + ",";
  line += String(sensors.aerationActive ? 1 : 0) + ",";
  line += String(sensors.alarmActive ? 1 : 0) + ",";
  line += "\"" + sensors.alarmReason + "\"";
  
  file.println(line);
  file.close();
}

void logEvent(String eventType, String value) {
  if (!ENABLE_SD_LOGGING || !sysStatus.sdCardOK) return;
  
  File file = SD.open("/logs/events.log", FILE_APPEND);
  if (file) {
    file.printf("%s,%s,%s\n", getTimestamp().c_str(), eventType.c_str(), value.c_str());
    file.close();
  }
}

// ═══════════════════════════════════════════════════════════════════════════════════
// ZEIT FUNKTIONEN
// ═══════════════════════════════════════════════════════════════════════════════════

void syncTime() {
  // NTP funktioniert nur über WiFi STA (nicht über LTE AT-Commands)
  // Für LTE müsste man AT+CCLK verwenden
  if (WiFi.status() != WL_CONNECTED) {
    if (DEBUG_MODE) Serial.println("⚠️  NTP benötigt WiFi STA Verbindung");
    return;
  }
  
  configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");
  Serial.println("🕐 Zeitsynchronisation...");

  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 2000)) {  // Timeout reduziert: 2s statt 10s
    Serial.printf("✅ Zeit: %02d:%02d:%02d %02d.%02d.%04d\n",
      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
      timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  } else {
    Serial.println("⚠️  NTP Sync fehlgeschlagen");
  }
}

String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return String(millis() / 1000);
  }
  
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

String getDateString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "nodate";
  }
  
  char buffer[12];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
  return String(buffer);
}

// ═══════════════════════════════════════════════════════════════════════════════════
// STATUS LED
// ═══════════════════════════════════════════════════════════════════════════════════

void updateStatusLED() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  unsigned long interval;
  
  if (sensors.alarmActive) {
    interval = 150;  // Schnelles Blinken bei Alarm
  } else if (!sysStatus.lteConnected && !sysStatus.wifiConnected) {
    interval = 500;  // Mittel bei keine Verbindung
  } else {
    interval = 2000; // Langsam wenn alles OK
  }
  
  if (millis() - lastBlink >= interval) {
    ledState = !ledState;
    digitalWrite(STATUS_LED, ledState);
    lastBlink = millis();
  }
}

// ═══════════════════════════════════════════════════════════════════════════════════
// HAUPTSCHLEIFE
// ═══════════════════════════════════════════════════════════════════════════════════

void loop() {
  esp_task_wdt_reset();

  unsigned long now = millis();
  sysStatus.uptime = (now - startTime) / 1000;

  // OTA Updates
  if (ENABLE_OTA && ENABLE_WIFI) {
    ArduinoOTA.handle();
    esp_task_wdt_reset();
  }

  // WebServer
  server.handleClient();
  esp_task_wdt_reset();

  // Sensoren auslesen
  if (now - lastSensorRead >= SENSOR_INTERVAL) {
    readAllSensors();
    checkAlarms();
    controlAeration();
    lastSensorRead = now;
    esp_task_wdt_reset();
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

  // LTE Status prüfen und ggf. reconnect
  if (ENABLE_LTE && now - lastLTECheck >= LTE_CHECK_INTERVAL) {
    bool wasConnected = sysStatus.lteConnected;
    checkLTEConnection();

    // Reconnect versuchen wenn Verbindung verloren
    if (wasConnected && !sysStatus.lteConnected) {
      Serial.println("⚠️  LTE Verbindung verloren, versuche Reconnect...");
      initLTE();
      esp_task_wdt_reset();
    }

    lastLTECheck = now;
  }

  // Zeit synchronisieren
  if (now - lastNTPSync >= NTP_SYNC_INTERVAL) {
    syncTime();
    lastNTPSync = now;
    esp_task_wdt_reset();
  }

  // Tägliche Zähler zurücksetzen (um Mitternacht)
  static int lastDay = -1;
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 1000)) {  // 1 Sekunde Timeout
    if (timeinfo.tm_mday != lastDay) {
      sysStatus.dailyAlarms = 0;
      lastDay = timeinfo.tm_mday;
      logEvent("DAILY_RESET", "Tägliche Zähler zurückgesetzt");
    }
  }
  
  // Memory-Check (alle 5 Minuten)
  static unsigned long lastMemCheck = 0;
  if (now - lastMemCheck >= 300000) {
    uint32_t freeHeap = ESP.getFreeHeap();
    if (freeHeap < 20000) {
      Serial.printf("⚠️  Niedriger Speicher: %u Bytes\n", freeHeap);
      logEvent("LOW_MEMORY", String(freeHeap));
    }
    lastMemCheck = now;
  }
  
  // Webserver
  server.handleClient();
  
  // Status LED
  updateStatusLED();
}

// ═══════════════════════════════════════════════════════════════════════════════════
// WEBSERVER SETUP
// ═══════════════════════════════════════════════════════════════════════════════════

void initWebServer() {
  // Hauptseite
  server.on("/", HTTP_GET, handleRoot);
  
  // API Endpunkte
  server.on("/api/sensors", HTTP_GET, handleAPISensors);
  server.on("/api/status", HTTP_GET, handleAPIStatus);
  server.on("/api/history", HTTP_GET, handleAPIHistory);
  server.on("/api/settings", HTTP_GET, handleAPISettings);
  server.on("/api/settings", HTTP_POST, handleAPISettingsPost);
  server.on("/api/relay", HTTP_POST, handleAPIRelay);
  server.on("/api/test-email", HTTP_POST, handleAPITestEmail);
  server.on("/api/calibration", HTTP_GET, handleAPICalibrationGet);
  server.on("/api/calibration/ph", HTTP_POST, handleAPICalibrationPH);
  server.on("/api/calibration/tds", HTTP_POST, handleAPICalibrationTDS);
  server.on("/api/calibration/reset", HTTP_POST, handleAPICalibrationReset);
  
  // Statische Ressourcen
  server.on("/style.css", HTTP_GET, handleCSS);
  server.on("/app.js", HTTP_GET, handleJS);
  
  server.onNotFound([]() {
    server.send(404, "text/plain", "Nicht gefunden");
  });
  
  server.begin();
  Serial.println("✅ Webserver gestartet");
}

// API Handler
void handleAPISensors() {
  StaticJsonDocument<512> doc;
  doc["waterTemp"] = sensors.waterTemp;
  doc["airTemp"] = sensors.airTemp;
  doc["ph"] = sensors.ph;
  doc["tds"] = sensors.tds;
  doc["dissolvedOxygen"] = sensors.dissolvedOxygen;
  doc["waterLevel"] = sensors.waterLevelOK;
  doc["aeration"] = sensors.aerationActive;
  doc["alarm"] = sensors.alarmActive;
  doc["alarmReason"] = sensors.alarmReason;
  doc["timestamp"] = sensors.timestamp;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleAPIStatus() {
  StaticJsonDocument<512> doc;
  doc["uptime"] = sysStatus.uptime;
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["wifiConnected"] = sysStatus.wifiConnected;
  doc["wifiRSSI"] = WiFi.RSSI();
  doc["lteConnected"] = sysStatus.lteConnected;
  doc["lteSignal"] = sysStatus.lteSignal;
  doc["lteOperator"] = sysStatus.lteOperator;
  doc["publicIP"] = sysStatus.publicIP;
  doc["sdCard"] = sysStatus.sdCardOK;
  doc["alarmCount"] = sysStatus.alarmCount;
  doc["dailyAlarms"] = sysStatus.dailyAlarms;
  doc["firmware"] = sysStatus.firmwareVersion;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleAPIHistory() {
  // Kompakte Antwort für Performance
  String json = "{";
  int count = history.full ? HISTORY_SIZE : history.index;
  int start = history.full ? history.index : 0;
  
  // Nur jeden 3. Wert für schnellere Übertragung (96 Punkte = 8h)
  json += "\"waterTemp\":[";
  for (int i = 0; i < count; i += 3) {
    int idx = (start + i) % HISTORY_SIZE;
    if (i > 0) json += ",";
    json += String(history.waterTemp[idx], 1);
  }
  
  json += "],\"airTemp\":[";
  for (int i = 0; i < count; i += 3) {
    int idx = (start + i) % HISTORY_SIZE;
    if (i > 0) json += ",";
    json += String(history.airTemp[idx], 1);
  }
  
  json += "],\"ph\":[";
  for (int i = 0; i < count; i += 3) {
    int idx = (start + i) % HISTORY_SIZE;
    if (i > 0) json += ",";
    json += String(history.ph[idx], 2);
  }
  
  json += "],\"tds\":[";
  for (int i = 0; i < count; i += 3) {
    int idx = (start + i) % HISTORY_SIZE;
    if (i > 0) json += ",";
    json += String(history.tds[idx], 0);
  }
  
  if (ENABLE_DO_SENSOR) {
    json += "],\"do\":[";
    for (int i = 0; i < count; i += 3) {
      int idx = (start + i) % HISTORY_SIZE;
      if (i > 0) json += ",";
      json += String(history.dissolvedOxygen[idx], 1);
    }
  }
  
  json += "]}";
  
  server.send(200, "application/json", json);
}

void handleAPISettings() {
  StaticJsonDocument<256> doc;
  doc["tempMin"] = troutParams.tempMin;
  doc["tempMax"] = troutParams.tempMax;
  doc["tempCritical"] = troutParams.tempCritical;
  doc["phMin"] = troutParams.phMin;
  doc["phMax"] = troutParams.phMax;
  doc["tdsMax"] = troutParams.tdsMax;
  doc["doMin"] = troutParams.doMin;
  doc["doOptimal"] = troutParams.doOptimal;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleAPISettingsPost() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (error) {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    if (doc.containsKey("tempMin")) troutParams.tempMin = doc["tempMin"];
    if (doc.containsKey("tempMax")) troutParams.tempMax = doc["tempMax"];
    if (doc.containsKey("tempCritical")) troutParams.tempCritical = doc["tempCritical"];
    if (doc.containsKey("phMin")) troutParams.phMin = doc["phMin"];
    if (doc.containsKey("phMax")) troutParams.phMax = doc["phMax"];
    if (doc.containsKey("tdsMax")) troutParams.tdsMax = doc["tdsMax"];
    if (doc.containsKey("doMin")) troutParams.doMin = doc["doMin"];
    if (doc.containsKey("doOptimal")) troutParams.doOptimal = doc["doOptimal"];
    
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    server.send(400, "application/json", "{\"error\":\"No data\"}");
  }
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
    case 2: pin = RELAY_ALARM; break;
    case 3: pin = RELAY_SPARE1; break;
    case 4: pin = RELAY_SPARE2; break;
  }
  
  if (pin < 0) {
    server.send(400, "application/json", "{\"error\":\"Invalid relay\"}");
    return;
  }
  
  digitalWrite(pin, state ? LOW : HIGH);
  server.send(200, "application/json", "{\"success\":true}");
}

void handleAPITestEmail() {
  sendEmailAlert("Test-Email", "Dies ist eine Test-Nachricht vom ForellenWächter.");
  server.send(200, "application/json", "{\"success\":true}");
}

// ═══════════════════════════════════════════════════════════════════════════════════
// KALIBRIERUNGS-API
// ═══════════════════════════════════════════════════════════════════════════════════

void handleAPICalibrationGet() {
  StaticJsonDocument<512> doc;

  doc["ph"]["calibrated"] = calibration.ph_calibrated;
  doc["ph"]["slope"] = calibration.ph_slope;
  doc["ph"]["offset"] = calibration.ph_offset;
  doc["ph"]["buffer1_adc"] = calibration.ph_buffer1_adc;
  doc["ph"]["buffer2_adc"] = calibration.ph_buffer2_adc;
  doc["ph"]["buffer1_value"] = calibration.ph_buffer1_value;
  doc["ph"]["buffer2_value"] = calibration.ph_buffer2_value;

  doc["tds"]["calibrated"] = calibration.tds_calibrated;
  doc["tds"]["factor"] = calibration.tds_factor;
  doc["tds"]["reference_adc"] = calibration.tds_reference_adc;
  doc["tds"]["reference_value"] = calibration.tds_reference_value;

  if (ENABLE_DO_SENSOR) {
    doc["do"]["calibrated"] = calibration.do_calibrated;
    doc["do"]["slope"] = calibration.do_slope;
    doc["do"]["offset"] = calibration.do_offset;
  }

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleAPICalibrationPH() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  int step = doc["step"];  // 1 oder 2
  float buffer_value = doc["buffer_value"];  // z.B. 4.0 oder 7.0
  int adc_reading = analogRead(PH_PIN);

  if (step == 1) {
    calibration.ph_buffer1_adc = adc_reading;
    calibration.ph_buffer1_value = buffer_value;
    Serial.printf("pH Kalibrierung Schritt 1: ADC=%d, pH=%.1f\n", adc_reading, buffer_value);
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Schritt 1 gespeichert\",\"adc\":" + String(adc_reading) + "}");
  }
  else if (step == 2) {
    calibration.ph_buffer2_adc = adc_reading;
    calibration.ph_buffer2_value = buffer_value;

    // Berechne Slope und Offset (y = mx + b)
    float m = (calibration.ph_buffer2_value - calibration.ph_buffer1_value) /
              (calibration.ph_buffer2_adc - calibration.ph_buffer1_adc);
    float b = calibration.ph_buffer1_value - (m * calibration.ph_buffer1_adc);

    calibration.ph_slope = m;
    calibration.ph_offset = b;
    calibration.ph_calibrated = true;

    saveCalibration();

    Serial.printf("pH Kalibrierung Schritt 2: ADC=%d, pH=%.1f\n", adc_reading, buffer_value);
    Serial.printf("✅ pH kalibriert: Slope=%.4f, Offset=%.4f\n", m, b);

    server.send(200, "application/json", "{\"success\":true,\"message\":\"Kalibrierung abgeschlossen\",\"slope\":" + String(m, 4) + ",\"offset\":" + String(b, 4) + "}");
  }
  else {
    server.send(400, "application/json", "{\"error\":\"Invalid step (1 or 2)\"}");
  }
}

void handleAPICalibrationTDS() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  float reference_value = doc["reference_value"];  // z.B. 1413 ppm (1413 µS/cm Lösung)
  int adc_reading = analogRead(TDS_PIN);

  calibration.tds_reference_adc = adc_reading;
  calibration.tds_reference_value = reference_value;
  calibration.tds_factor = reference_value / adc_reading;
  calibration.tds_calibrated = true;

  saveCalibration();

  Serial.printf("✅ TDS kalibriert: ADC=%d, Referenz=%.0f ppm, Faktor=%.4f\n",
                adc_reading, reference_value, calibration.tds_factor);

  server.send(200, "application/json", "{\"success\":true,\"message\":\"TDS kalibriert\",\"factor\":" + String(calibration.tds_factor, 4) + "}");
}

void handleAPICalibrationReset() {
  calibration.ph_calibrated = false;
  calibration.ph_slope = 3.5;
  calibration.ph_offset = 0.0;

  calibration.tds_calibrated = false;
  calibration.tds_factor = 0.5;

  calibration.do_calibrated = false;
  calibration.do_slope = 1.0;
  calibration.do_offset = 0.0;

  saveCalibration();

  Serial.println("🔄 Kalibrierung zurückgesetzt");
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Kalibrierung zur\\u00fcckgesetzt\"}");
}

void handleCSS() {
  server.send(200, "text/css", getCSS());
}

void handleJS() {
  server.send(200, "application/javascript", getJS());
}

// HTML, CSS, JS werden in separater Datei definiert (zu lang für hier)
void handleRoot() {
  server.send(200, "text/html", getHTML());
}

// ═══════════════════════════════════════════════════════════════════════════════════
// HTML DASHBOARD
// ═══════════════════════════════════════════════════════════════════════════════════

String getHTML() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ForellenWächter v1.4</title>
  <link rel="icon" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='.9em' font-size='90'>🐟</text></svg>">
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    :root {
      --primary: #0ea5e9;
      --primary-dark: #0284c7;
      --secondary: #10b981;
      --danger: #ef4444;
      --warning: #f59e0b;
      --dark: #1e293b;
      --darker: #0f172a;
      --light: #f8fafc;
      --glass: rgba(255,255,255,0.1);
      --glass-border: rgba(255,255,255,0.2);
    }
    
    * { box-sizing: border-box; margin: 0; padding: 0; }
    
    body {
      font-family: 'Segoe UI', system-ui, -apple-system, sans-serif;
      background: linear-gradient(135deg, var(--darker) 0%, #1a365d 50%, #134e4a 100%);
      min-height: 100vh;
      color: var(--light);
    }
    
    .container {
      max-width: 1400px;
      margin: 0 auto;
      padding: 20px;
    }
    
    /* Header */
    header {
      text-align: center;
      padding: 30px 0;
      margin-bottom: 30px;
    }
    
    .logo {
      font-size: 3.5em;
      margin-bottom: 10px;
      animation: swim 3s ease-in-out infinite;
    }
    
    @keyframes swim {
      0%, 100% { transform: translateX(0) rotate(0deg); }
      25% { transform: translateX(10px) rotate(5deg); }
      75% { transform: translateX(-10px) rotate(-5deg); }
    }
    
    header h1 {
      font-size: 2em;
      font-weight: 300;
      letter-spacing: 2px;
      text-transform: uppercase;
      background: linear-gradient(90deg, var(--primary), var(--secondary));
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      background-clip: text;
    }
    
    header .subtitle {
      color: rgba(255,255,255,0.6);
      font-size: 0.9em;
      margin-top: 5px;
    }
    
    /* Status Bar */
    .status-bar {
      display: flex;
      justify-content: center;
      flex-wrap: wrap;
      gap: 15px;
      margin-bottom: 30px;
    }
    
    .status-pill {
      display: flex;
      align-items: center;
      gap: 8px;
      padding: 10px 20px;
      background: var(--glass);
      backdrop-filter: blur(10px);
      border: 1px solid var(--glass-border);
      border-radius: 50px;
      font-size: 0.85em;
      transition: all 0.3s;
    }
    
    .status-pill:hover {
      background: rgba(255,255,255,0.15);
      transform: translateY(-2px);
    }
    
    .status-pill .dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: var(--secondary);
      animation: pulse 2s infinite;
    }
    
    .status-pill .dot.warning { background: var(--warning); }
    .status-pill .dot.danger { background: var(--danger); animation: pulse 0.5s infinite; }
    
    @keyframes pulse {
      0%, 100% { opacity: 1; transform: scale(1); }
      50% { opacity: 0.5; transform: scale(1.2); }
    }
    
    /* Cards Grid */
    .cards-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
      gap: 20px;
      margin-bottom: 30px;
    }
    
    .card {
      background: var(--glass);
      backdrop-filter: blur(10px);
      border: 1px solid var(--glass-border);
      border-radius: 20px;
      padding: 25px;
      text-align: center;
      transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
      position: relative;
      overflow: hidden;
    }
    
    .card::before {
      content: '';
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      height: 3px;
      background: linear-gradient(90deg, var(--primary), var(--secondary));
      opacity: 0;
      transition: opacity 0.3s;
    }
    
    .card:hover {
      transform: translateY(-5px);
      box-shadow: 0 20px 40px rgba(0,0,0,0.3);
    }
    
    .card:hover::before { opacity: 1; }
    
    .card.ok::before { background: var(--secondary); opacity: 1; }
    .card.warning::before { background: var(--warning); opacity: 1; }
    .card.danger::before { background: var(--danger); opacity: 1; animation: pulse 0.5s infinite; }
    
    .card .icon {
      font-size: 2.5em;
      margin-bottom: 15px;
      filter: drop-shadow(0 4px 6px rgba(0,0,0,0.3));
    }
    
    .card .value {
      font-size: 2.5em;
      font-weight: 700;
      background: linear-gradient(135deg, #fff, rgba(255,255,255,0.8));
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      background-clip: text;
    }
    
    .card .unit {
      font-size: 0.8em;
      opacity: 0.7;
      font-weight: 300;
    }
    
    .card .label {
      font-size: 0.8em;
      text-transform: uppercase;
      letter-spacing: 1px;
      color: rgba(255,255,255,0.6);
      margin-top: 10px;
    }
    
    .card .range {
      font-size: 0.7em;
      color: rgba(255,255,255,0.4);
      margin-top: 5px;
    }
    
    /* Charts */
    .charts-section {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
      gap: 20px;
      margin-bottom: 30px;
    }
    
    .chart-card {
      background: var(--glass);
      backdrop-filter: blur(10px);
      border: 1px solid var(--glass-border);
      border-radius: 20px;
      padding: 25px;
    }

    .chart-container {
      position: relative;
      height: 300px;
      max-height: 300px;
      width: 100%;
      overflow: hidden;
    }

    .chart-card h3 {
      font-size: 1em;
      font-weight: 500;
      margin-bottom: 20px;
      display: flex;
      align-items: center;
      gap: 10px;
    }
    
    .chart-tabs {
      display: flex;
      gap: 10px;
      margin-left: auto;
    }
    
    .chart-tab {
      padding: 5px 12px;
      background: rgba(255,255,255,0.1);
      border: none;
      border-radius: 15px;
      color: white;
      font-size: 0.75em;
      cursor: pointer;
      transition: all 0.3s;
    }
    
    .chart-tab:hover { background: rgba(255,255,255,0.2); }
    .chart-tab.active { background: var(--primary); }
    
    /* System Info */
    .system-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px;
      margin-bottom: 30px;
    }
    
    .info-card {
      background: var(--glass);
      backdrop-filter: blur(10px);
      border: 1px solid var(--glass-border);
      border-radius: 20px;
      padding: 25px;
    }
    
    .info-card h3 {
      font-size: 0.9em;
      text-transform: uppercase;
      letter-spacing: 1px;
      color: rgba(255,255,255,0.6);
      margin-bottom: 20px;
      display: flex;
      align-items: center;
      gap: 10px;
    }
    
    .info-row {
      display: flex;
      justify-content: space-between;
      padding: 10px 0;
      border-bottom: 1px solid rgba(255,255,255,0.1);
    }
    
    .info-row:last-child { border-bottom: none; }
    
    .info-label { color: rgba(255,255,255,0.6); }
    .info-value { font-weight: 500; }
    
    /* Controls */
    .controls-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
      gap: 15px;
    }
    
    .relay-btn {
      padding: 15px;
      background: rgba(255,255,255,0.1);
      border: 1px solid rgba(255,255,255,0.2);
      border-radius: 15px;
      color: white;
      cursor: pointer;
      transition: all 0.3s;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 8px;
    }
    
    .relay-btn:hover { background: rgba(255,255,255,0.2); }
    .relay-btn.active { background: var(--secondary); border-color: var(--secondary); }
    
    .relay-btn .icon { font-size: 1.5em; }
    .relay-btn .name { font-size: 0.8em; }
    
    /* Alarm Banner */
    .alarm-banner {
      display: none;
      background: linear-gradient(90deg, var(--danger), #dc2626);
      padding: 15px 25px;
      border-radius: 15px;
      margin-bottom: 20px;
      animation: alarm-pulse 1s infinite;
    }
    
    .alarm-banner.show { display: flex; align-items: center; gap: 15px; }
    
    @keyframes alarm-pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.8; }
    }
    
    .alarm-banner .icon { font-size: 1.5em; }
    .alarm-banner .text { flex: 1; }
    .alarm-banner .reason { font-weight: 700; }
    
    /* Footer */
    footer {
      text-align: center;
      padding: 30px;
      color: rgba(255,255,255,0.4);
      font-size: 0.85em;
    }
    
    footer a { color: var(--primary); text-decoration: none; }
    footer a:hover { text-decoration: underline; }
    
    /* Responsive */
    @media (max-width: 768px) {
      .container { padding: 15px; }
      header h1 { font-size: 1.5em; }
      .cards-grid { grid-template-columns: repeat(2, 1fr); }
      .charts-section { grid-template-columns: 1fr; }
      .chart-card { min-width: 0; }
      .chart-container { height: 250px; max-height: 250px; }
    }
    
    /* Loading */
    .loading {
      display: flex;
      justify-content: center;
      align-items: center;
      padding: 50px;
    }
    
    .spinner {
      width: 40px;
      height: 40px;
      border: 3px solid rgba(255,255,255,0.1);
      border-top-color: var(--primary);
      border-radius: 50%;
      animation: spin 1s linear infinite;
    }
    
    @keyframes spin { to { transform: rotate(360deg); } }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <div class="logo">🐟</div>
      <h1>ForellenWächter</h1>
      <p class="subtitle">IoT Monitoring für Aquakultur • Powered by Wasserkraft ⚡</p>
    </header>
    
    <div class="alarm-banner" id="alarmBanner">
      <span class="icon">🚨</span>
      <div class="text">
        <div class="reason" id="alarmReason">ALARM</div>
        <div>Sofortige Überprüfung erforderlich!</div>
      </div>
    </div>
    
    <div class="status-bar">
      <div class="status-pill">
        <span class="dot" id="dotWifi"></span>
        <span id="statusWifi">WiFi: --</span>
      </div>
      <div class="status-pill">
        <span class="dot" id="dotLTE"></span>
        <span id="statusLTE">LTE: --</span>
      </div>
      <div class="status-pill">
        <span class="dot" id="dotSystem"></span>
        <span id="statusUptime">Uptime: --</span>
      </div>
    </div>
    
    <div class="cards-grid">
      <div class="card" id="cardWater">
        <div class="icon">💧</div>
        <div class="value"><span id="waterTemp">--</span><span class="unit">°C</span></div>
        <div class="label">Wassertemperatur</div>
        <div class="range">Optimal: 8-14°C</div>
      </div>
      
      <div class="card" id="cardAir">
        <div class="icon">🌡️</div>
        <div class="value"><span id="airTemp">--</span><span class="unit">°C</span></div>
        <div class="label">Lufttemperatur</div>
        <div class="range">Umgebung</div>
      </div>
      
      <div class="card" id="cardPH">
        <div class="icon">🧪</div>
        <div class="value"><span id="phValue">--</span></div>
        <div class="label">pH-Wert</div>
        <div class="range">Optimal: 6.5-8.5</div>
      </div>
      
      <div class="card" id="cardTDS">
        <div class="icon">📊</div>
        <div class="value"><span id="tdsValue">--</span><span class="unit">ppm</span></div>
        <div class="label">TDS</div>
        <div class="range">Max: 500 ppm</div>
      </div>
      
      <div class="card" id="cardDO">
        <div class="icon">🫧</div>
        <div class="value"><span id="doValue">--</span><span class="unit">mg/L</span></div>
        <div class="label">Sauerstoff</div>
        <div class="range">Min: 6.0 mg/L</div>
      </div>
      
      <div class="card" id="cardLevel">
        <div class="icon">🌊</div>
        <div class="value" id="waterLevel">--</div>
        <div class="label">Wasserlevel</div>
        <div class="range">Float Switch</div>
      </div>
    </div>
    
    <div class="charts-section">
      <div class="chart-card">
        <h3>
          📈 Temperaturverlauf
          <div class="chart-tabs">
            <button class="chart-tab active" onclick="setRange('temp', 24)">24h</button>
            <button class="chart-tab" onclick="setRange('temp', 168)">7d</button>
          </div>
        </h3>
        <div class="chart-container">
          <canvas id="tempChart"></canvas>
        </div>
      </div>

      <div class="chart-card">
        <h3>
          🧪 Wasserqualität
          <div class="chart-tabs">
            <button class="chart-tab active" onclick="setRange('quality', 24)">24h</button>
            <button class="chart-tab" onclick="setRange('quality', 168)">7d</button>
          </div>
        </h3>
        <div class="chart-container">
          <canvas id="qualityChart"></canvas>
        </div>
      </div>
    </div>
    
    <div class="system-grid">
      <div class="info-card">
        <h3>📡 Verbindung</h3>
        <div class="info-row">
          <span class="info-label">WiFi RSSI</span>
          <span class="info-value" id="wifiRSSI">-- dBm</span>
        </div>
        <div class="info-row">
          <span class="info-label">LTE Signal</span>
          <span class="info-value" id="lteSignal">--/31</span>
        </div>
        <div class="info-row">
          <span class="info-label">Operator</span>
          <span class="info-value" id="lteOperator">--</span>
        </div>
        <div class="info-row">
          <span class="info-label">Public IP</span>
          <span class="info-value" id="publicIP">--</span>
        </div>
      </div>
      
      <div class="info-card">
        <h3>⚙️ System</h3>
        <div class="info-row">
          <span class="info-label">Firmware</span>
          <span class="info-value" id="firmware">v1.4.0</span>
        </div>
        <div class="info-row">
          <span class="info-label">Free Heap</span>
          <span class="info-value" id="freeHeap">-- KB</span>
        </div>
        <div class="info-row">
          <span class="info-label">SD-Karte</span>
          <span class="info-value" id="sdCard">--</span>
        </div>
        <div class="info-row">
          <span class="info-label">Alarme heute</span>
          <span class="info-value" id="dailyAlarms">0</span>
        </div>
      </div>
      
      <div class="info-card">
        <h3>🎛️ Steuerung</h3>
        <div class="controls-grid">
          <button class="relay-btn" id="relay1" onclick="toggleRelay(1)">
            <span class="icon">💨</span>
            <span class="name">Belüftung</span>
          </button>
          <button class="relay-btn" id="relay2" onclick="toggleRelay(2)">
            <span class="icon">🔔</span>
            <span class="name">Alarm</span>
          </button>
          <button class="relay-btn" id="relay3" onclick="toggleRelay(3)">
            <span class="icon">⚡</span>
            <span class="name">Reserve 1</span>
          </button>
          <button class="relay-btn" id="relay4" onclick="toggleRelay(4)">
            <span class="icon">⚡</span>
            <span class="name">Reserve 2</span>
          </button>
        </div>
      </div>
    </div>
    
    <footer>
      ForellenWächter v1.4 LTE Edition •
      <a href="/api/sensors">API</a> •
      © 2024 Andreas
    </footer>
  </div>
  
  <script>
    let tempChart, qualityChart;
    let relayStates = [false, false, false, false];
    
    // Charts initialisieren
    function initCharts() {
      const defaultOptions = {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: {
            position: 'top',
            labels: { color: 'rgba(255,255,255,0.8)', padding: 20 }
          }
        },
        scales: {
          x: {
            grid: { color: 'rgba(255,255,255,0.1)' },
            ticks: { color: 'rgba(255,255,255,0.6)' }
          },
          y: {
            grid: { color: 'rgba(255,255,255,0.1)' },
            ticks: { color: 'rgba(255,255,255,0.6)' }
          }
        }
      };
      
      tempChart = new Chart(document.getElementById('tempChart'), {
        type: 'line',
        data: {
          labels: [],
          datasets: [{
            label: 'Wasser °C',
            data: [],
            borderColor: '#0ea5e9',
            backgroundColor: 'rgba(14,165,233,0.1)',
            fill: true,
            tension: 0.4,
            borderWidth: 2
          }, {
            label: 'Luft °C',
            data: [],
            borderColor: '#f59e0b',
            backgroundColor: 'rgba(245,158,11,0.1)',
            fill: true,
            tension: 0.4,
            borderWidth: 2
          }]
        },
        options: defaultOptions
      });
      
      qualityChart = new Chart(document.getElementById('qualityChart'), {
        type: 'line',
        data: {
          labels: [],
          datasets: [{
            label: 'pH',
            data: [],
            borderColor: '#a855f7',
            yAxisID: 'y',
            tension: 0.4,
            borderWidth: 2
          }, {
            label: 'O₂ mg/L',
            data: [],
            borderColor: '#10b981',
            yAxisID: 'y1',
            tension: 0.4,
            borderWidth: 2
          }]
        },
        options: {
          ...defaultOptions,
          scales: {
            ...defaultOptions.scales,
            y: { ...defaultOptions.scales.y, position: 'left', min: 5, max: 10 },
            y1: { ...defaultOptions.scales.y, position: 'right', min: 0, max: 15 }
          }
        }
      });
    }
    
    // Daten abrufen
    async function fetchSensors() {
      try {
        const res = await fetch('/api/sensors');
        const data = await res.json();
        updateSensorDisplay(data);
      } catch (e) {
        console.error('Fetch error:', e);
      }
    }
    
    async function fetchStatus() {
      try {
        const res = await fetch('/api/status');
        const data = await res.json();
        updateStatusDisplay(data);
      } catch (e) {}
    }
    
    async function fetchHistory() {
      try {
        const res = await fetch('/api/history');
        const data = await res.json();
        updateCharts(data);
      } catch (e) {}
    }
    
    // Anzeige aktualisieren
    function updateSensorDisplay(data) {
      document.getElementById('waterTemp').textContent = data.waterTemp.toFixed(1);
      document.getElementById('airTemp').textContent = data.airTemp.toFixed(1);
      document.getElementById('phValue').textContent = data.ph.toFixed(2);
      document.getElementById('tdsValue').textContent = Math.round(data.tds);
      document.getElementById('doValue').textContent = data.dissolvedOxygen.toFixed(1);
      document.getElementById('waterLevel').textContent = data.waterLevel ? 'OK' : 'NIEDRIG';
      
      // Karten-Status
      updateCardStatus('cardWater', data.waterTemp, 8, 14, 16);
      updateCardStatus('cardPH', data.ph, 6.5, 8.5);
      updateCardStatus('cardTDS', data.tds, 0, 500);
      updateCardStatus('cardDO', data.dissolvedOxygen, 6, 999);
      
      document.getElementById('cardLevel').className = 'card ' + (data.waterLevel ? 'ok' : 'danger');
      
      // Alarm Banner
      const banner = document.getElementById('alarmBanner');
      if (data.alarm) {
        banner.classList.add('show');
        document.getElementById('alarmReason').textContent = data.alarmReason || 'ALARM AKTIV';
      } else {
        banner.classList.remove('show');
      }
      
      // Belüftungs-Button
      relayStates[0] = data.aeration;
      document.getElementById('relay1').classList.toggle('active', data.aeration);
    }
    
    function updateCardStatus(id, value, min, max, critical) {
      const card = document.getElementById(id);
      if (!card) return;
      
      card.classList.remove('ok', 'warning', 'danger');
      
      if (critical && value > critical) {
        card.classList.add('danger');
      } else if (value < min || value > max) {
        card.classList.add('warning');
      } else {
        card.classList.add('ok');
      }
    }
    
    function updateStatusDisplay(data) {
      // WiFi
      const wifiDot = document.getElementById('dotWifi');
      document.getElementById('statusWifi').textContent = 
        data.wifiConnected ? `WiFi: ${data.wifiRSSI} dBm` : 'WiFi: Offline';
      wifiDot.className = 'dot ' + (data.wifiConnected ? '' : 'warning');
      
      // LTE
      const lteDot = document.getElementById('dotLTE');
      document.getElementById('statusLTE').textContent = 
        data.lteConnected ? `LTE: ${data.lteSignal}/31` : 'LTE: Offline';
      lteDot.className = 'dot ' + (data.lteConnected ? '' : 'warning');
      
      // System
      const uptime = formatUptime(data.uptime);
      document.getElementById('statusUptime').textContent = `Uptime: ${uptime}`;
      
      // Info-Felder
      document.getElementById('wifiRSSI').textContent = data.wifiRSSI + ' dBm';
      document.getElementById('lteSignal').textContent = data.lteSignal + '/31';
      document.getElementById('lteOperator').textContent = data.lteOperator || '--';
      document.getElementById('publicIP').textContent = data.publicIP || '--';
      document.getElementById('firmware').textContent = 'v' + data.firmware;
      document.getElementById('freeHeap').textContent = Math.round(data.freeHeap / 1024) + ' KB';
      document.getElementById('sdCard').textContent = data.sdCard ? 'OK' : 'Fehlt';
      document.getElementById('dailyAlarms').textContent = data.dailyAlarms;
    }
    
    function updateCharts(data) {
      // Labels: Wir zeigen jeden 3. Datenpunkt = 15 min Intervall
      const labels = data.waterTemp.map((_, i) => {
        const minutes = i * 15;  // Jeder Punkt = 15 min (weil API jeden 3. samplet)
        if (minutes >= 60) {
          return Math.floor(minutes / 60) + 'h';
        }
        return minutes + 'm';
      });
      
      tempChart.data.labels = labels;
      tempChart.data.datasets[0].data = data.waterTemp;
      tempChart.data.datasets[1].data = data.airTemp;
      tempChart.update('none');
      
      qualityChart.data.labels = labels;
      qualityChart.data.datasets[0].data = data.ph;
      qualityChart.data.datasets[1].data = data.do || [];
      qualityChart.update('none');
    }
    
    function formatUptime(seconds) {
      const d = Math.floor(seconds / 86400);
      const h = Math.floor((seconds % 86400) / 3600);
      const m = Math.floor((seconds % 3600) / 60);
      
      if (d > 0) return `${d}d ${h}h`;
      if (h > 0) return `${h}h ${m}m`;
      return `${m}m`;
    }
    
    // Relais steuern
    async function toggleRelay(num) {
      const newState = !relayStates[num - 1];
      
      try {
        const res = await fetch(`/api/relay?relay=${num}&state=${newState ? 1 : 0}`, {
          method: 'POST'
        });
        
        if (res.ok) {
          relayStates[num - 1] = newState;
          document.getElementById(`relay${num}`).classList.toggle('active', newState);
        }
      } catch (e) {
        console.error('Relay error:', e);
      }
    }
    
    function setRange(chart, hours) {
      // Tab-Status aktualisieren
      event.target.parentNode.querySelectorAll('.chart-tab').forEach(t => t.classList.remove('active'));
      event.target.classList.add('active');
      // Hier könnte Filterlogik implementiert werden
    }
    
    // Init
    initCharts();
    fetchSensors();
    fetchStatus();
    fetchHistory();
    
    setInterval(fetchSensors, 2000);
    setInterval(fetchStatus, 10000);
    setInterval(fetchHistory, 60000);
  </script>
</body>
</html>
)rawliteral";

  return html;
}

String getCSS() { return ""; }
String getJS() { return ""; }
