/*
 * ═══════════════════════════════════════════════════════════════════════════
 * ForellenWächter v1.3 - Deep Sleep Edition
 * Ultra-Stromsparmodus für Batteriebetrieb
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 * WICHTIG: Diese Version ist für BATTERIEBETRIEB optimiert!
 * 
 * Für Netzbetrieb mit Web-Dashboard → v1.2 Full Edition verwenden!
 * 
 * Features:
 * - Deep Sleep Mode (ESP32 schläft zwischen Messungen)
 * - SD-Karten Datenlogging
 * - Battery Monitoring mit Spannungsteiler
 * - Software Watchdog
 * - Alarm per Summer/Buzzer
 * 
 * NICHT enthalten (zu stromhungrig):
 * - WiFi / WebSocket / Dashboard
 * - Live-Charts
 * 
 * Stromverbrauch:
 * - Aktiv: ~80mA für 30 Sekunden
 * - Deep Sleep: ~10µA
 * - Durchschnitt bei 10 Min Intervall: ~0.4mA
 * 
 * Batterie-Laufzeit (30Ah):
 * - v1.2 (24/7 WiFi): ~4 Tage
 * - v1.3 (Deep Sleep): ~8 MONATE!
 * 
 * Autor: Andreas
 * Lizenz: MIT
 * ═══════════════════════════════════════════════════════════════════════════
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>
#include <esp_sleep.h>
#include <esp_task_wdt.h>
#include <driver/rtc_io.h>

// ═══════════════════════════════════════════════════════════════════════════
// KONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

// --- Deep Sleep Einstellungen ---
#define DEEP_SLEEP_ENABLED true       // false = Normal Mode (für Tests)
#define SLEEP_DURATION_SEC 600        // 10 Minuten (600 Sek)
#define MEASUREMENT_TIME_SEC 30       // Zeit für Messung (30 Sek)
#define WATCHDOG_TIMEOUT_SEC 60       // Watchdog Timeout

// --- Betriebsmodus ---
#define TEST_MODE false               // true = Fake-Werte
#define DEBUG_SERIAL true             // Serial-Ausgabe (verbraucht Strom!)

// --- Batterie ---
#define BATTERY_PIN 36                // ADC für Batterie-Spannung
#define BATTERY_R1 10000.0            // Oberer Widerstand (Ohm)
#define BATTERY_R2 3300.0             // Unterer Widerstand (Ohm)
#define BATTERY_FULL 12.6             // 100% Spannung
#define BATTERY_EMPTY 10.5            // 0% Spannung
#define BATTERY_WARNING 11.5          // Warnung

// --- Sensor Grenzwerte ---
#define TEMP_MIN 8.0
#define TEMP_MAX 14.0
#define TEMP_CRITICAL 15.5
#define PH_MIN 6.5
#define PH_MAX 8.5
#define TDS_MAX 500

// ═══════════════════════════════════════════════════════════════════════════
// PIN DEFINITIONEN
// ═══════════════════════════════════════════════════════════════════════════

#define ONE_WIRE_BUS 4
#define PH_PIN 34
#define TDS_PIN 35
#define WATER_LEVEL_PIN 33
#define RELAY_AERATION 26
#define BUZZER_PIN 27                 // Alarm-Summer
#define STATUS_LED 2
#define SD_CS 5

// ═══════════════════════════════════════════════════════════════════════════
// RTC MEMORY (überlebt Deep Sleep!)
// ═══════════════════════════════════════════════════════════════════════════

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int alarmCount = 0;
RTC_DATA_ATTR float lastWaterTemp = 0;
RTC_DATA_ATTR float lastBatteryVolt = 12.0;
RTC_DATA_ATTR bool lowBatteryMode = false;

// ═══════════════════════════════════════════════════════════════════════════
// GLOBALE OBJEKTE
// ═══════════════════════════════════════════════════════════════════════════

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensors(&oneWire);
DeviceAddress waterTempAddr, airTempAddr;

// Sensor Werte
float waterTemp = 0;
float airTemp = 0;
float phValue = 7.0;
float tdsValue = 0;
bool waterLevelOK = true;
float batteryVoltage = 12.0;
int batteryPercent = 100;
bool alarmActive = false;

// ═══════════════════════════════════════════════════════════════════════════
// SETUP (läuft nach JEDEM Deep Sleep!)
// ═══════════════════════════════════════════════════════════════════════════

void setup() {
  bootCount++;
  
  // Serial nur wenn DEBUG_SERIAL aktiviert
  if (DEBUG_SERIAL) {
    Serial.begin(115200);
    delay(100);
    printWakeupInfo();
  }
  
  // Watchdog initialisieren
  initWatchdog();
  
  // Pins konfigurieren
  initPins();
  
  // Batterie zuerst prüfen!
  readBattery();
  
  if (batteryPercent < 5) {
    // Kritisch niedrige Batterie - Emergency Sleep
    emergencySleep();
    return;
  }
  
  // Low Battery Mode
  if (batteryVoltage < BATTERY_WARNING && !lowBatteryMode) {
    lowBatteryMode = true;
    logEvent("LOW_BATTERY", batteryVoltage);
  }
  
  // Sensoren initialisieren
  initSensors();
  
  // Sensoren auslesen
  readAllSensors();
  
  // Alarme prüfen
  checkAlarms();
  
  // Daten loggen
  logToSD();
  
  // Status ausgeben
  if (DEBUG_SERIAL) {
    printStatus();
  }
  
  // Watchdog füttern
  esp_task_wdt_reset();
  
  // In Deep Sleep gehen
  if (DEEP_SLEEP_ENABLED) {
    goToSleep();
  } else {
    // Normal Mode - Loop läuft
    if (DEBUG_SERIAL) {
      Serial.println("Normal Mode - Loop aktiv");
    }
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// LOOP (nur im Normal Mode)
// ═══════════════════════════════════════════════════════════════════════════

void loop() {
  if (DEEP_SLEEP_ENABLED) {
    // Sollte nicht erreicht werden
    goToSleep();
    return;
  }
  
  // Normal Mode für Tests
  static unsigned long lastRead = 0;
  
  if (millis() - lastRead >= 5000) {
    esp_task_wdt_reset();
    readAllSensors();
    checkAlarms();
    printStatus();
    lastRead = millis();
  }
  
  delay(100);
}

// ═══════════════════════════════════════════════════════════════════════════
// INITIALISIERUNG
// ═══════════════════════════════════════════════════════════════════════════

void initWatchdog() {
  esp_task_wdt_config_t wdtConfig = {
    .timeout_ms = WATCHDOG_TIMEOUT_SEC * 1000,
    .idle_core_mask = 0,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdtConfig);
  esp_task_wdt_add(NULL);
}

void initPins() {
  pinMode(STATUS_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(WATER_LEVEL_PIN, INPUT_PULLUP);
  pinMode(RELAY_AERATION, OUTPUT);
  pinMode(BATTERY_PIN, INPUT);
  
  // Alles aus
  digitalWrite(STATUS_LED, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RELAY_AERATION, HIGH);  // LOW-aktiv
  
  // Kurzer LED-Blitz
  digitalWrite(STATUS_LED, HIGH);
  delay(50);
  digitalWrite(STATUS_LED, LOW);
}

void initSensors() {
  tempSensors.begin();
  int count = tempSensors.getDeviceCount();
  
  if (count >= 1) {
    tempSensors.getAddress(waterTempAddr, 0);
  }
  if (count >= 2) {
    tempSensors.getAddress(airTempAddr, 1);
  }
  
  tempSensors.setResolution(12);
  tempSensors.setWaitForConversion(true);
}

// ═══════════════════════════════════════════════════════════════════════════
// SENSOR FUNKTIONEN
// ═══════════════════════════════════════════════════════════════════════════

void readAllSensors() {
  if (TEST_MODE) {
    // Fake-Werte für Tests
    waterTemp = 11.5 + (bootCount % 10) * 0.2;
    airTemp = 18.0 + (bootCount % 5) * 0.5;
    phValue = 7.2;
    tdsValue = 180;
    waterLevelOK = true;
    return;
  }
  
  // Temperaturen
  tempSensors.requestTemperatures();
  
  float temp = tempSensors.getTempCByIndex(0);
  if (temp > -50 && temp < 85) {
    waterTemp = temp;
    lastWaterTemp = temp;  // In RTC Memory speichern
  }
  
  temp = tempSensors.getTempCByIndex(1);
  if (temp > -50 && temp < 85) {
    airTemp = temp;
  }
  
  // pH
  int rawPH = analogRead(PH_PIN);
  float voltage = rawPH * 3.3 / 4095.0;
  phValue = 7.0 + (2.5 - voltage) * 3.5;
  phValue = constrain(phValue, 0.0, 14.0);
  
  // TDS
  int rawTDS = analogRead(TDS_PIN);
  float voltageTDS = rawTDS * 3.3 / 4095.0;
  float tempCoeff = 1.0 + 0.02 * (waterTemp - 25.0);
  float compVoltage = voltageTDS / tempCoeff;
  tdsValue = (133.42 * pow(compVoltage, 3) - 
              255.86 * pow(compVoltage, 2) + 
              857.39 * compVoltage) * 0.5;
  tdsValue = constrain(tdsValue, 0.0, 1000.0);
  
  // Wasserlevel
  waterLevelOK = digitalRead(WATER_LEVEL_PIN) == LOW;
}

void readBattery() {
  // Mehrere Messungen für Stabilität
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(BATTERY_PIN);
    delay(5);
  }
  float rawValue = sum / 10.0;
  
  // Spannung berechnen (Spannungsteiler)
  float adcVoltage = rawValue * 3.3 / 4095.0;
  batteryVoltage = adcVoltage * (BATTERY_R1 + BATTERY_R2) / BATTERY_R2;
  
  // Prozent berechnen
  batteryPercent = map(batteryVoltage * 100, 
                       BATTERY_EMPTY * 100, 
                       BATTERY_FULL * 100, 
                       0, 100);
  batteryPercent = constrain(batteryPercent, 0, 100);
  
  lastBatteryVolt = batteryVoltage;
}

// ═══════════════════════════════════════════════════════════════════════════
// ALARM SYSTEM
// ═══════════════════════════════════════════════════════════════════════════

void checkAlarms() {
  alarmActive = false;
  String alarmReason = "";
  
  // Temperatur
  if (waterTemp > TEMP_CRITICAL) {
    alarmActive = true;
    alarmReason = "TEMP_CRITICAL";
  } else if (waterTemp < TEMP_MIN || waterTemp > TEMP_MAX) {
    alarmActive = true;
    alarmReason = "TEMP_WARNING";
  }
  
  // pH
  if (phValue < PH_MIN || phValue > PH_MAX) {
    alarmActive = true;
    alarmReason = "PH_WARNING";
  }
  
  // TDS
  if (tdsValue > TDS_MAX) {
    alarmActive = true;
    alarmReason = "TDS_HIGH";
  }
  
  // Wasserlevel
  if (!waterLevelOK) {
    alarmActive = true;
    alarmReason = "WATER_LOW";
  }
  
  // Batterie
  if (batteryPercent < 20) {
    alarmActive = true;
    alarmReason = "BATTERY_LOW";
  }
  
  // Alarm auslösen
  if (alarmActive) {
    alarmCount++;
    soundAlarm();
    logEvent("ALARM", alarmReason);
    
    // Belüftung aktivieren bei Temperatur-Alarm
    if (waterTemp > TEMP_MAX) {
      digitalWrite(RELAY_AERATION, LOW);
    }
  } else {
    digitalWrite(RELAY_AERATION, HIGH);
  }
}

void soundAlarm() {
  // 3x kurzer Piep
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// SD-KARTE LOGGING
// ═══════════════════════════════════════════════════════════════════════════

void logToSD() {
  if (!SD.begin(SD_CS)) {
    if (DEBUG_SERIAL) Serial.println("SD-Karte nicht gefunden");
    return;
  }
  
  // Dateiname basierend auf Boot-Count
  int fileNum = bootCount / 288;  // Neue Datei alle 24h (bei 5min Intervall)
  char filename[24];
  snprintf(filename, sizeof(filename), "/log_%04d.csv", fileNum);
  
  bool newFile = !SD.exists(filename);
  File file = SD.open(filename, FILE_APPEND);
  
  if (!file) {
    if (DEBUG_SERIAL) Serial.println("Kann Datei nicht öffnen");
    return;
  }
  
  // Header für neue Datei
  if (newFile) {
    file.println("Boot,WaterTemp,AirTemp,pH,TDS,Level,Battery,BattV,Alarm");
  }
  
  // Daten schreiben
  file.printf("%d,%.2f,%.2f,%.2f,%.0f,%d,%d,%.2f,%d\n",
    bootCount,
    waterTemp,
    airTemp,
    phValue,
    tdsValue,
    waterLevelOK ? 1 : 0,
    batteryPercent,
    batteryVoltage,
    alarmActive ? 1 : 0
  );
  
  file.close();
  SD.end();  // SD ausschalten für Stromsparen
}

void logEvent(String eventType, String value) {
  if (!SD.begin(SD_CS)) return;
  
  File file = SD.open("/events.log", FILE_APPEND);
  if (file) {
    file.printf("%d,%s,%s\n", bootCount, eventType.c_str(), value.c_str());
    file.close();
  }
  SD.end();
}

void logEvent(String eventType, float value) {
  logEvent(eventType, String(value, 2));
}

// ═══════════════════════════════════════════════════════════════════════════
// DEEP SLEEP
// ═══════════════════════════════════════════════════════════════════════════

void goToSleep() {
  if (DEBUG_SERIAL) {
    Serial.printf("\n💤 Schlafe für %d Minuten...\n", SLEEP_DURATION_SEC / 60);
    Serial.flush();
  }
  
  // GPIO für RTC isolieren (spart Strom)
  rtc_gpio_isolate(GPIO_NUM_12);
  rtc_gpio_isolate(GPIO_NUM_14);
  
  // Sleep-Timer setzen
  uint64_t sleepTime = (uint64_t)SLEEP_DURATION_SEC * 1000000ULL;
  
  // Im Low Battery Mode länger schlafen
  if (lowBatteryMode) {
    sleepTime *= 2;  // Doppelt so lange
  }
  
  esp_sleep_enable_timer_wakeup(sleepTime);
  
  // Deep Sleep starten
  esp_deep_sleep_start();
}

void emergencySleep() {
  // Kritischer Batterie-Zustand!
  if (DEBUG_SERIAL) {
    Serial.println("🚨 EMERGENCY: Batterie kritisch!");
    Serial.println("Gehe in 1-Stunden Sleep...");
    Serial.flush();
  }
  
  logEvent("EMERGENCY", "BATTERY_CRITICAL");
  
  // 1 Stunde schlafen
  esp_sleep_enable_timer_wakeup(3600ULL * 1000000ULL);
  esp_deep_sleep_start();
}

// ═══════════════════════════════════════════════════════════════════════════
// DEBUG AUSGABE
// ═══════════════════════════════════════════════════════════════════════════

void printWakeupInfo() {
  Serial.println("\n════════════════════════════════════════");
  Serial.println("  🐟 ForellenWächter v1.3 - Deep Sleep");
  Serial.println("════════════════════════════════════════");
  Serial.printf("Boot #%d\n", bootCount);
  
  esp_sleep_wakeup_cause_t wakeup = esp_sleep_get_wakeup_cause();
  switch (wakeup) {
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Aufgewacht durch: Timer");
      break;
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Aufgewacht durch: External 0");
      break;
    default:
      Serial.println("Aufgewacht durch: Reset/Power");
      break;
  }
}

void printStatus() {
  Serial.println("────────────────────────────────────────");
  Serial.printf("💧 Wasser: %.1f°C  ", waterTemp);
  Serial.printf("🌡️ Luft: %.1f°C\n", airTemp);
  Serial.printf("🧪 pH: %.2f       ", phValue);
  Serial.printf("📊 TDS: %.0f ppm\n", tdsValue);
  Serial.printf("🌊 Level: %s\n", waterLevelOK ? "OK" : "NIEDRIG!");
  Serial.printf("🔋 Batterie: %d%% (%.2fV)\n", batteryPercent, batteryVoltage);
  
  if (alarmActive) {
    Serial.println("🚨 ALARM AKTIV!");
  }
  
  if (lowBatteryMode) {
    Serial.println("⚠️  LOW BATTERY MODE");
  }
  
  Serial.printf("📈 Alarme gesamt: %d\n", alarmCount);
  Serial.println("────────────────────────────────────────");
}
