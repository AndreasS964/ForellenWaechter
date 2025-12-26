/*
 * ═══════════════════════════════════════════════════════════════════════════════════
 * ForellenWächter v1.6 - LTE Remote Edition (Refactored)
 * IoT Monitoring System mit Mobilfunk-Anbindung für Forellenzucht
 * ═══════════════════════════════════════════════════════════════════════════════════
 *
 * ÄNDERUNGEN in v1.6:
 * - ✅ HTML in separate Header-Dateien ausgelagert (PROGMEM)
 * - ✅ Streaming-Ausgabe statt String-Return (RAM-Ersparnis)
 * - ✅ Konfiguration in config.h zentralisiert
 * - ✅ Bessere Speicherverwaltung
 * - ✅ Watchdog-Bug behoben
 * 
 * Lizenz: MIT
 * Repository: https://github.com/AndreasS964/ForellenWaechter
 * ═══════════════════════════════════════════════════════════════════════════════════
 */

// ═══════════════════════════════════════════════════════════════════════════════════
// INCLUDES
// ═══════════════════════════════════════════════════════════════════════════════════

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

// Lokale Header-Dateien
#include "config.h"
#include "html_dashboard.h"
#include "html_dashboard_js.h"
#include "html_settings.h"

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
// DATENSTRUKTUREN
// ═══════════════════════════════════════════════════════════════════════════════════

// Sensor Grenzwerte
struct TroutParameters {
  float tempMin = DEFAULT_TEMP_MIN;
  float tempMax = DEFAULT_TEMP_MAX;
  float tempCritical = DEFAULT_TEMP_CRITICAL;
  float phMin = DEFAULT_PH_MIN;
  float phMax = DEFAULT_PH_MAX;
  float tdsMax = DEFAULT_TDS_MAX;
  float doMin = DEFAULT_DO_MIN;
  float doOptimal = DEFAULT_DO_OPTIMAL;
} troutParams;

// Aktuelle Sensordaten
struct SensorData {
  float waterTemp = 0;
  float airTemp = 0;
  float ph = 7.0;
  float tds = 0;
  float dissolvedOxygen = 0;
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
  int lteSignal = 0;
  String lteOperator = "";
  String publicIP = "";
  unsigned long uptime = 0;
  unsigned long lastEmailSent = 0;
  int alarmCount = 0;
  int dailyAlarms = 0;
  String firmwareVersion = FIRMWARE_VERSION;
} sysStatus;

// Kalibrierungsdaten
struct CalibrationData {
  uint16_t magic = EEPROM_MAGIC;
  float ph_slope = 3.5;
  float ph_offset = 0.0;
  bool ph_calibrated = false;
  int ph_buffer1_adc = 0;
  int ph_buffer2_adc = 0;
  float ph_buffer1_value = 4.0;
  float ph_buffer2_value = 7.0;
  float tds_factor = 0.5;
  bool tds_calibrated = false;
  int tds_reference_adc = 0;
  float tds_reference_value = 1413.0;
  float do_slope = 1.0;
  float do_offset = 0.0;
  bool do_calibrated = false;
  uint8_t checksum = 0;
} calibration;

// Historie für Charts
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

// Relay Modi: 0=Auto, 1=An, 2=Aus
uint8_t relayModes[4] = {2, 2, 2, 0};
bool relayStates[4] = {false, false, false, false};

// ═══════════════════════════════════════════════════════════════════════════════════
// FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════════════

void sendDashboardHTML();
void sendSettingsHTML();
void handleRoot();
void handleSettings();
void handleAPISensors();
void handleAPIStatus();
void handleAPIHistory();
void handleAPISettings();
void handleAPISettingsPost();
void handleAPIRelay();
void handleAPICalibrationGet();
void handleAPICalibrationPH();
void handleAPICalibrationTDS();
void handleAPICalibrationReset();

// ═══════════════════════════════════════════════════════════════════════════════════
// SETUP
// ═══════════════════════════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  delay(500);

  printBanner();
  startTime = millis();

  initWatchdog();
  esp_task_wdt_reset();

  initPins();
  initEEPROM();
  loadCalibration();
  initSensors();
  esp_task_wdt_reset();
  
  initSDCard();
  esp_task_wdt_reset();

  if (ENABLE_WIFI) {
    initWiFi();
    esp_task_wdt_reset();
    if (ENABLE_OTA) {
      initOTA();
      esp_task_wdt_reset();
    }
  }

  if (ENABLE_LTE) {
    initLTE();
    esp_task_wdt_reset();
  }

  initWebServer();
  esp_task_wdt_reset();

  readAllSensors();
  esp_task_wdt_reset();

  lastNTPSync = millis() - NTP_SYNC_INTERVAL + 30000;

  Serial.println("\n✅ ForellenWächter v1.6 bereit!");
  Serial.printf("   Free Heap: %u bytes\n", ESP.getFreeHeap());
  Serial.println("══════════════════════════════════════════════\n");
}

void printBanner() {
  Serial.println("\n");
  Serial.println("╔══════════════════════════════════════════════════════╗");
  Serial.println("║   🐟 ForellenWächter v1.6 - LTE Remote Edition       ║");
  Serial.println("║   Refactored: HTML ausgelagert, RAM optimiert        ║");
  Serial.println("╚══════════════════════════════════════════════════════╝\n");
  
  if (TEST_MODE) Serial.println("⚠️  TEST MODE AKTIV");
  Serial.printf("Features: WiFi=%d LTE=%d Email=%d DO=%d SD=%d OTA=%d\n",
    ENABLE_WIFI, ENABLE_LTE, ENABLE_EMAIL_ALERTS, ENABLE_DO_SENSOR, ENABLE_SD_LOGGING, ENABLE_OTA);
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
  // LEDs und Buzzer
  pinMode(STATUS_LED, OUTPUT);
  pinMode(ALARM_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Float Switch mit internem Pull-up (kein externer Widerstand nötig!)
  pinMode(WATER_LEVEL_PIN, INPUT_PULLUP);
  
  // OneWire Bus - Interner Pull-up aktivieren
  // Der ESP32 interne Pull-up (~45kΩ) reicht für kurze Kabel (<3m)
  // Für längere Kabel trotzdem externen 4.7kΩ verwenden
  pinMode(ONE_WIRE_BUS, INPUT_PULLUP);
  
  // Relais (alle AUS beim Start)
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);

  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, LOW);
  digitalWrite(RELAY_3, LOW);
  digitalWrite(RELAY_4, LOW);
  
  // Startup-Blinken
  for (int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED, HIGH);
    delay(100);
    digitalWrite(STATUS_LED, LOW);
    delay(100);
  }
  
  Serial.println("✅ Pins initialisiert (interne Pull-ups aktiv)");
}

void initSensors() {
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
  
  if (!SD.exists("/logs")) SD.mkdir("/logs");
  if (!SD.exists("/data")) SD.mkdir("/data");
}

void initWiFi() {
  Serial.println("📶 WiFi wird gestartet...");
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.printf("✅ AP: %s (IP: %s)\n", AP_SSID, WiFi.softAPIP().toString().c_str());
  
  sysStatus.wifiConnected = true;
  
  if (strlen(STA_SSID) > 0) {
    WiFi.begin(STA_SSID, STA_PASSWORD);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
      if (attempts % 10 == 0) esp_task_wdt_reset();
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("\n✅ WiFi verbunden: %s\n", WiFi.localIP().toString().c_str());
    } else {
      Serial.println("\n⚠️  WiFi STA nicht verbunden");
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
    if (sysStatus.sdCardOK) SD.end();
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
  for (size_t i = 0; i < sizeof(CalibrationData) - 1; i++) {
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
  
  sendATCommand("AT", 1000);
  sendATCommand("ATE0", 500);
  
  String moduleInfo = sendATCommand("ATI", 1000);
  Serial.printf("   Modul: %s\n", moduleInfo.c_str());
  
  String simStatus = sendATCommand("AT+CPIN?", 1000);
  if (simStatus.indexOf("READY") == -1) {
    Serial.println("⚠️  SIM-Karte nicht bereit!");
    return;
  }
  
  sendATCommand("AT+CREG=1", 500);
  delay(2000);
  
  String apnCmd = "AT+CGDCONT=1,\"IP\",\"" + String(LTE_APN) + "\"";
  sendATCommand(apnCmd.c_str(), 1000);
  sendATCommand("AT+CGACT=1,1", 5000);
  
  checkLTEConnection();
  
  if (sysStatus.lteConnected) {
    Serial.println("✅ LTE verbunden!");
    Serial.printf("   Operator: %s, Signal: %d/31\n", sysStatus.lteOperator.c_str(), sysStatus.lteSignal);
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
  return response;
}

void checkLTEConnection() {
  String csq = sendATCommand("AT+CSQ", 1000);
  int signalStart = csq.indexOf(": ") + 2;
  int signalEnd = csq.indexOf(",");
  if (signalStart > 1 && signalEnd > signalStart) {
    sysStatus.lteSignal = csq.substring(signalStart, signalEnd).toInt();
  }
  
  String cops = sendATCommand("AT+COPS?", 1000);
  int opStart = cops.indexOf("\"") + 1;
  int opEnd = cops.indexOf("\"", opStart);
  if (opStart > 0 && opEnd > opStart) {
    sysStatus.lteOperator = cops.substring(opStart, opEnd);
  }
  
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
  bool wifiStaConnected = (WiFi.status() == WL_CONNECTED);
  
  if (!sysStatus.lteConnected && !wifiStaConnected) {
    return false;
  }
  
  if (wifiStaConnected) {
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);
    int httpCode = http.POST(payload);
    http.end();
    return httpCode >= 200 && httpCode < 300;
  }
  
  return false;
}

void sendEmailAlert(String subject, String message) {
  if (!ENABLE_EMAIL_ALERTS) return;
  
  unsigned long cooldownMs = (unsigned long)EMAIL_COOLDOWN_MIN * 60UL * 1000UL;
  if (sysStatus.lastEmailSent > 0 && (millis() - sysStatus.lastEmailSent) < cooldownMs) {
    return;
  }
  
  StaticJsonDocument<512> doc;
  doc["value1"] = subject;
  doc["value2"] = message;
  doc["value3"] = getTimestamp();
  
  String payload;
  serializeJson(doc, payload);
  
  if (sendHTTPRequest(EMAIL_WEBHOOK_URL, payload.c_str())) {
    sysStatus.lastEmailSent = millis();
    Serial.printf("📧 E-Mail gesendet: %s\n", subject.c_str());
  }
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
    if (temp > -50 && temp < 85) sensors.waterTemp = temp;
  }
  
  if (airSensorFound) {
    float temp = tempSensors.getTempC(airTempAddr);
    if (temp > -50 && temp < 85) sensors.airTemp = temp;
  }
}

void readPH() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(PH_PIN);
    delay(10);
  }
  float rawValue = sum / 10.0;

  if (calibration.ph_calibrated) {
    sensors.ph = (calibration.ph_slope * rawValue) + calibration.ph_offset;
  } else {
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

  if (calibration.tds_calibrated) {
    float tempCoeff = 1.0 + 0.02 * (sensors.waterTemp - 25.0);
    sensors.tds = (rawValue * calibration.tds_factor) / tempCoeff;
  } else {
    float voltage = rawValue * 3.3 / 4095.0;
    float tempCoeff = 1.0 + 0.02 * (sensors.waterTemp - 25.0);
    float compVoltage = voltage / tempCoeff;
    sensors.tds = (133.42 * pow(compVoltage, 3) - 255.86 * pow(compVoltage, 2) + 857.39 * compVoltage) * 0.5;
  }
  sensors.tds = constrain(sensors.tds, 0.0, 1000.0);
}

void readDissolvedOxygen() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(DO_PIN);
    delay(10);
  }
  float rawValue = sum / 10.0;
  float voltage = rawValue * 3.3 / 4095.0;
  
  const float V_SATURATION = 1.5;
  const float DO_SATURATION = 9.09;
  
  float tempCompensation = 1.0 - 0.024 * (sensors.waterTemp - 20.0);
  float doSaturation = DO_SATURATION * tempCompensation;
  
  sensors.dissolvedOxygen = (voltage / V_SATURATION) * doSaturation;
  sensors.dissolvedOxygen = constrain(sensors.dissolvedOxygen, 0.0, 20.0);
}

void readWaterLevel() {
  sensors.waterLevelOK = digitalRead(WATER_LEVEL_PIN) == LOW;
}

void printSensorValues() {
  Serial.printf("💧 %.1f°C | 🌡️ %.1f°C | 🧪 %.2f | 📊 %.0fppm | 🌊 %s\n",
    sensors.waterTemp, sensors.airTemp, sensors.ph, sensors.tds,
    sensors.waterLevelOK ? "OK" : "LOW");
}

// ═══════════════════════════════════════════════════════════════════════════════════
// ALARM & STEUERUNG
// ═══════════════════════════════════════════════════════════════════════════════════

void checkAlarms() {
  bool alarm = false;
  String reasons = "";
  
  if (sensors.waterTemp > troutParams.tempCritical) {
    alarm = true;
    reasons += "Temp KRITISCH; ";
  } else if (sensors.waterTemp < troutParams.tempMin || sensors.waterTemp > troutParams.tempMax) {
    alarm = true;
    reasons += "Temp außerhalb; ";
  }
  
  if (sensors.ph < troutParams.phMin || sensors.ph > troutParams.phMax) {
    alarm = true;
    reasons += "pH außerhalb; ";
  }
  
  if (sensors.tds > troutParams.tdsMax) {
    alarm = true;
    reasons += "TDS hoch; ";
  }
  
  if (ENABLE_DO_SENSOR && sensors.dissolvedOxygen < troutParams.doMin) {
    alarm = true;
    reasons += "O2 niedrig; ";
  }
  
  if (!sensors.waterLevelOK) {
    alarm = true;
    reasons += "Wasserlevel; ";
  }
  
  bool wasAlarm = sensors.alarmActive;
  sensors.alarmActive = alarm;
  sensors.alarmReason = reasons;
  
  if (alarm && !wasAlarm) {
    sysStatus.alarmCount++;
    sysStatus.dailyAlarms++;
    soundAlarm();
    
    String subject = "🚨 ForellenWächter ALARM";
    String message = "ALARM: " + reasons;
    sendEmailAlert(subject, message);
  }
  
  digitalWrite(ALARM_LED, alarm ? HIGH : LOW);
}

void controlAeration() {
  bool shouldActivate = false;

  if (sensors.waterTemp > troutParams.tempMax) shouldActivate = true;
  if (ENABLE_DO_SENSOR && sensors.dissolvedOxygen < troutParams.doOptimal) shouldActivate = true;
  if (sensors.ph < troutParams.phMin || sensors.ph > troutParams.phMax) shouldActivate = true;

  sensors.aerationActive = shouldActivate;
}

void updateRelays() {
  int pins[4] = {RELAY_1, RELAY_2, RELAY_3, RELAY_4};

  for (int i = 0; i < 4; i++) {
    bool targetState = false;

    switch (relayModes[i]) {
      case 0: // Auto
        if (i == 3) targetState = sensors.aerationActive;
        break;
      case 1: // An
        targetState = true;
        break;
      case 2: // Aus
        targetState = false;
        break;
    }

    relayStates[i] = targetState;
    digitalWrite(pins[i], targetState ? HIGH : LOW);
  }
}

void soundAlarm() {
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
  if (history.index == 0) history.full = true;
}

void logToSD() {
  if (!ENABLE_SD_LOGGING || !sysStatus.sdCardOK) return;
  
  String filename = "/logs/" + getDateString() + ".csv";
  bool newFile = !SD.exists(filename);
  
  File file = SD.open(filename, FILE_APPEND);
  if (!file) return;
  
  if (newFile) {
    file.println("Timestamp,WaterTemp,AirTemp,pH,TDS,WaterLevel,Aeration,Alarm");
  }
  
  String line = getTimestamp() + ",";
  line += String(sensors.waterTemp, 2) + ",";
  line += String(sensors.airTemp, 2) + ",";
  line += String(sensors.ph, 2) + ",";
  line += String(sensors.tds, 0) + ",";
  line += String(sensors.waterLevelOK ? 1 : 0) + ",";
  line += String(sensors.aerationActive ? 1 : 0) + ",";
  line += String(sensors.alarmActive ? 1 : 0);
  
  file.println(line);
  file.close();
}

// ═══════════════════════════════════════════════════════════════════════════════════
// ZEIT FUNKTIONEN
// ═══════════════════════════════════════════════════════════════════════════════════

void syncTime() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");
  Serial.println("🕐 Zeitsynchronisation...");

  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 2000)) {
    Serial.printf("✅ Zeit: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  }
}

String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return String(millis() / 1000);
  
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

String getDateString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "nodate";
  
  char buffer[12];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
  return String(buffer);
}

void updateStatusLED() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  unsigned long interval = sensors.alarmActive ? 150 : 
                           (!sysStatus.lteConnected && !sysStatus.wifiConnected) ? 500 : 2000;
  
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

  if (ENABLE_OTA && ENABLE_WIFI) {
    ArduinoOTA.handle();
  }

  server.handleClient();

  if (now - lastSensorRead >= SENSOR_INTERVAL) {
    readAllSensors();
    checkAlarms();
    controlAeration();
    updateRelays();
    lastSensorRead = now;
    esp_task_wdt_reset();
  }

  if (now - lastHistoryUpdate >= HISTORY_INTERVAL) {
    updateHistory();
    lastHistoryUpdate = now;
  }

  if (now - lastLogWrite >= LOG_INTERVAL) {
    logToSD();
    lastLogWrite = now;
  }

  if (ENABLE_LTE && now - lastLTECheck >= LTE_CHECK_INTERVAL) {
    checkLTEConnection();
    lastLTECheck = now;
  }

  if (now - lastNTPSync >= NTP_SYNC_INTERVAL) {
    syncTime();
    lastNTPSync = now;
  }

  // Tägliche Zähler zurücksetzen
  static int lastDay = -1;
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 100)) {
    if (timeinfo.tm_mday != lastDay) {
      sysStatus.dailyAlarms = 0;
      lastDay = timeinfo.tm_mday;
    }
  }
  
  updateStatusLED();
}

// ═══════════════════════════════════════════════════════════════════════════════════
// WEBSERVER - MIT STREAMING (RAM-OPTIMIERT!)
// ═══════════════════════════════════════════════════════════════════════════════════

void initWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/settings", HTTP_GET, handleSettings);
  server.on("/api/sensors", HTTP_GET, handleAPISensors);
  server.on("/api/status", HTTP_GET, handleAPIStatus);
  server.on("/api/history", HTTP_GET, handleAPIHistory);
  server.on("/api/settings", HTTP_GET, handleAPISettings);
  server.on("/api/settings", HTTP_POST, handleAPISettingsPost);
  server.on("/api/relay", HTTP_POST, handleAPIRelay);
  server.on("/api/calibration", HTTP_GET, handleAPICalibrationGet);
  server.on("/api/calibration/ph", HTTP_POST, handleAPICalibrationPH);
  server.on("/api/calibration/tds", HTTP_POST, handleAPICalibrationTDS);
  server.on("/api/calibration/reset", HTTP_POST, handleAPICalibrationReset);
  
  server.onNotFound([]() {
    server.send(404, "text/plain", "Nicht gefunden");
  });
  
  server.begin();
  Serial.println("✅ Webserver gestartet (Streaming-Modus)");
}

// ═══════════════════════════════════════════════════════════════════════════════════
// HTML STREAMING - Das ist der Schlüssel zur RAM-Ersparnis!
// Statt einen riesigen String zu erstellen, senden wir das HTML in Chunks.
// ═══════════════════════════════════════════════════════════════════════════════════

void sendDashboardHTML() {
  // Größe schätzen für Content-Length Header (optional, kann auch chunked sein)
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  
  // HTML in Chunks senden - jeder Chunk kommt direkt aus PROGMEM
  server.sendContent_P(DASHBOARD_HTML_HEAD);
  yield(); // Watchdog füttern
  esp_task_wdt_reset();
  
  server.sendContent_P(DASHBOARD_CSS);
  yield();
  esp_task_wdt_reset();
  
  server.sendContent_P(DASHBOARD_HTML_BODY1);
  yield();
  esp_task_wdt_reset();
  
  server.sendContent_P(DASHBOARD_HTML_BODY2);
  yield();
  esp_task_wdt_reset();
  
  server.sendContent_P(DASHBOARD_JS);
  yield();
  
  // Verbindung beenden
  server.sendContent("");
}

void sendSettingsHTML() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  
  server.sendContent_P(SETTINGS_HTML_HEAD);
  yield();
  esp_task_wdt_reset();
  
  server.sendContent_P(SETTINGS_HTML_BODY);
  yield();
  esp_task_wdt_reset();
  
  server.sendContent_P(SETTINGS_JS);
  yield();
  
  server.sendContent("");
}

void handleRoot() {
  sendDashboardHTML();
}

void handleSettings() {
  sendSettingsHTML();
}

// ═══════════════════════════════════════════════════════════════════════════════════
// API HANDLER
// ═══════════════════════════════════════════════════════════════════════════════════

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
  // Streaming für große History-Daten
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", "");
  
  int count = history.full ? HISTORY_SIZE : history.index;
  int start = history.full ? history.index : 0;
  
  server.sendContent("{\"waterTemp\":[");
  for (int i = 0; i < count; i += 3) {
    int idx = (start + i) % HISTORY_SIZE;
    if (i > 0) server.sendContent(",");
    server.sendContent(String(history.waterTemp[idx], 1));
    if (i % 30 == 0) { yield(); esp_task_wdt_reset(); }
  }
  
  server.sendContent("],\"airTemp\":[");
  for (int i = 0; i < count; i += 3) {
    int idx = (start + i) % HISTORY_SIZE;
    if (i > 0) server.sendContent(",");
    server.sendContent(String(history.airTemp[idx], 1));
    if (i % 30 == 0) { yield(); esp_task_wdt_reset(); }
  }
  
  server.sendContent("],\"ph\":[");
  for (int i = 0; i < count; i += 3) {
    int idx = (start + i) % HISTORY_SIZE;
    if (i > 0) server.sendContent(",");
    server.sendContent(String(history.ph[idx], 2));
    if (i % 30 == 0) { yield(); esp_task_wdt_reset(); }
  }
  
  server.sendContent("],\"tds\":[");
  for (int i = 0; i < count; i += 3) {
    int idx = (start + i) % HISTORY_SIZE;
    if (i > 0) server.sendContent(",");
    server.sendContent(String(history.tds[idx], 0));
    if (i % 30 == 0) { yield(); esp_task_wdt_reset(); }
  }
  
  if (ENABLE_DO_SENSOR) {
    server.sendContent("],\"do\":[");
    for (int i = 0; i < count; i += 3) {
      int idx = (start + i) % HISTORY_SIZE;
      if (i > 0) server.sendContent(",");
      server.sendContent(String(history.dissolvedOxygen[idx], 1));
      if (i % 30 == 0) { yield(); esp_task_wdt_reset(); }
    }
  }
  
  server.sendContent("]}");
  server.sendContent("");
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
    if (doc.containsKey("phMin")) troutParams.phMin = doc["phMin"];
    if (doc.containsKey("phMax")) troutParams.phMax = doc["phMax"];
    
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    server.send(400, "application/json", "{\"error\":\"No data\"}");
  }
}

void handleAPIRelay() {
  if (!server.hasArg("relay")) {
    server.send(400, "application/json", "{\"error\":\"Missing relay\"}");
    return;
  }

  int relay = server.arg("relay").toInt();
  if (relay < 1 || relay > 4) {
    server.send(400, "application/json", "{\"error\":\"Invalid relay\"}");
    return;
  }

  relayModes[relay - 1]++;
  if (relayModes[relay - 1] > 2) relayModes[relay - 1] = 0;
  updateRelays();

  String response = "{\"relay\":" + String(relay) + ",\"mode\":" + String(relayModes[relay - 1]) + "}";
  server.send(200, "application/json", response);
}

void handleAPICalibrationGet() {
  StaticJsonDocument<512> doc;
  doc["ph"]["calibrated"] = calibration.ph_calibrated;
  doc["ph"]["slope"] = calibration.ph_slope;
  doc["ph"]["offset"] = calibration.ph_offset;
  doc["tds"]["calibrated"] = calibration.tds_calibrated;
  doc["tds"]["factor"] = calibration.tds_factor;
  
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
  deserializeJson(doc, server.arg("plain"));

  int step = doc["step"];
  float buffer_value = doc["buffer_value"];
  int adc_reading = analogRead(PH_PIN);

  if (step == 1) {
    calibration.ph_buffer1_adc = adc_reading;
    calibration.ph_buffer1_value = buffer_value;
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Punkt 1 gespeichert\",\"adc\":" + String(adc_reading) + "}");
  }
  else if (step == 2) {
    calibration.ph_buffer2_adc = adc_reading;
    calibration.ph_buffer2_value = buffer_value;

    float m = (calibration.ph_buffer2_value - calibration.ph_buffer1_value) /
              (calibration.ph_buffer2_adc - calibration.ph_buffer1_adc);
    float b = calibration.ph_buffer1_value - (m * calibration.ph_buffer1_adc);

    calibration.ph_slope = m;
    calibration.ph_offset = b;
    calibration.ph_calibrated = true;
    saveCalibration();

    server.send(200, "application/json", "{\"success\":true,\"message\":\"Kalibrierung abgeschlossen\"}");
  }
}

void handleAPICalibrationTDS() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }

  StaticJsonDocument<256> doc;
  deserializeJson(doc, server.arg("plain"));

  float reference_value = doc["reference_value"];
  int adc_reading = analogRead(TDS_PIN);

  calibration.tds_reference_adc = adc_reading;
  calibration.tds_reference_value = reference_value;
  calibration.tds_factor = reference_value / adc_reading;
  calibration.tds_calibrated = true;
  saveCalibration();

  server.send(200, "application/json", "{\"success\":true,\"message\":\"TDS kalibriert\"}");
}

void handleAPICalibrationReset() {
  calibration.ph_calibrated = false;
  calibration.ph_slope = 3.5;
  calibration.ph_offset = 0.0;
  calibration.tds_calibrated = false;
  calibration.tds_factor = 0.5;
  saveCalibration();

  server.send(200, "application/json", "{\"success\":true,\"message\":\"Reset erfolgreich\"}");
}
