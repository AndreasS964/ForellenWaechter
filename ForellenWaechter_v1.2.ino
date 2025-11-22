/*
 * ForellenWächter v1.2 - Aquakultur Monitoring System
 * Entwickelt für Lucas Haug's Forellenzucht
 * 
 * Neue Features in v1.2:
 * - SD-Karten Datenlogger mit CSV-Export
 * - Chart.js Live-Visualisierung (24h Verlauf)
 * - Modernes Web-Interface Design
 * - Optimiert für Lucas Haug's Anlage
 * 
 * Hardware:
 * - ESP32 DevKit
 * - 4-Kanal Relais Modul
 * - DS18B20 Temperatursensoren (Wasser + Luft)
 * - pH Sensor (Analog)
 * - TDS Sensor (Analog)
 * - Wasserstands-Schalter (Digital)
 * - SD-Karten Modul (SPI) - NEU in v1.2
 * 
 * SD-Modul Pinbelegung:
 * CS   -> GPIO 5
 * MOSI -> GPIO 23
 * MISO -> GPIO 19
 * SCK  -> GPIO 18
 * 
 * Lizenz: MIT
 * Author: Andreas S.
 */

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>
#include <time.h>

// ========== KONFIGURATION ==========

// WiFi Einstellungen
const char* ap_ssid = "ForellenWaechter";
const char* ap_password = "Lucas2024";

// Optionale Station Mode Credentials
const char* sta_ssid = "";        // Dein WLAN Name (leer lassen für reinen AP Mode)
const char* sta_password = "";    // Dein WLAN Passwort

// Pin Definitionen
#define ONE_WIRE_BUS 4        // DS18B20 Datenleitung
#define PH_PIN 34             // pH Sensor (Analog)
#define TDS_PIN 35            // TDS Sensor (Analog)
#define WATER_LEVEL_PIN 25    // Wasserstandsschalter
#define RELAY_AERATION 26     // Relais 1: Belüftung
#define RELAY_2 27            // Relais 2: Reserve
#define RELAY_3 14            // Relais 3: Reserve
#define RELAY_4 12            // Relais 4: Reserve

// SD-Karte Pins
#define SD_CS 5               // SD Card Chip Select
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18

// Sensor Kalibrierung
#define PH_NEUTRAL_VOLTAGE 1500.0  // mV bei pH 7.0
#define PH_ACID_VOLTAGE 2032.0     // mV bei pH 4.0
#define TDS_VREF 3.3               // ESP32 ADC Referenzspannung
#define TDS_SCOUNT 30              // Anzahl Samples für Durchschnitt

// Grenzwerte
#define TEMP_MIN 12.0         // Minimale Wassertemperatur (°C)
#define TEMP_MAX 18.0         // Maximale Wassertemperatur (°C)
#define PH_MIN 6.5            // Minimaler pH-Wert
#define PH_MAX 8.5            // Maximaler pH-Wert
#define TDS_MAX 300           // Maximaler TDS-Wert (ppm)

// Timing
#define SENSOR_INTERVAL 5000      // Sensor-Ablesung alle 5 Sekunden
#define LOG_INTERVAL 600000       // SD-Logging alle 10 Minuten (600000ms)
#define CHART_DATA_POINTS 288     // 24h bei 5-Min Intervallen

// ========== GLOBALE VARIABLEN ==========

// Sensor Objekte
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress waterThermometer, airThermometer;

// Web Server & WebSocket
WebServer server(80);
WebSocketsServer webSocket(81);

// Sensor Werte
float waterTemp = 0.0;
float airTemp = 0.0;
float phValue = 0.0;
float tdsValue = 0.0;
bool waterLevel = false;
bool alarmActive = false;
bool aerationActive = false;

// Zeitstempel
unsigned long lastSensorRead = 0;
unsigned long lastLogWrite = 0;
unsigned long lastChartUpdate = 0;

// SD-Karte Status
bool sdCardAvailable = false;
String currentLogFile = "";

// Chart Daten (Ring-Buffer für letzte 24h)
struct ChartDataPoint {
  unsigned long timestamp;
  float waterTemp;
  float airTemp;
  float pH;
  float tds;
};

ChartDataPoint chartData[CHART_DATA_POINTS];
int chartDataIndex = 0;
int chartDataCount = 0;

// NTP Time
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;      // GMT+1 (Deutschland)
const int daylightOffset_sec = 3600;  // Sommerzeit

// ========== SETUP ==========

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=================================");
  Serial.println("ForellenWächter v1.2");
  Serial.println("Lucas Haug's Forellenzucht");
  Serial.println("=================================\n");

  // Pin Modes
  pinMode(WATER_LEVEL_PIN, INPUT_PULLUP);
  pinMode(RELAY_AERATION, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  
  // Relais initial aus (HIGH = aus bei active-low Relais)
  digitalWrite(RELAY_AERATION, HIGH);
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(RELAY_3, HIGH);
  digitalWrite(RELAY_4, HIGH);

  // DS18B20 initialisieren
  sensors.begin();
  Serial.print("Gefundene DS18B20 Sensoren: ");
  Serial.println(sensors.getDeviceCount());
  
  if (sensors.getDeviceCount() >= 2) {
    sensors.getAddress(waterThermometer, 0);
    sensors.getAddress(airThermometer, 1);
    Serial.println("✓ Temperatursensoren gefunden");
  } else {
    Serial.println("⚠ Warnung: Nicht alle Temperatursensoren gefunden!");
  }

  // SD-Karte initialisieren
  initSDCard();

  // WiFi starten
  setupWiFi();

  // Zeit synchronisieren
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Zeitsynchronisation gestartet...");

  // Web Server Routes
  setupWebServer();

  // WebSocket starten
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Chart Daten initialisieren
  memset(chartData, 0, sizeof(chartData));

  Serial.println("\n✓ System bereit!\n");
}

// ========== MAIN LOOP ==========

void loop() {
  server.handleClient();
  webSocket.loop();

  unsigned long currentMillis = millis();

  // Sensoren auslesen
  if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = currentMillis;
    readSensors();
    checkAlarms();
    sendWebSocketUpdate();
  }

  // SD-Karte Logging (alle 10 Minuten)
  if (currentMillis - lastLogWrite >= LOG_INTERVAL) {
    lastLogWrite = currentMillis;
    logDataToSD();
  }

  // Chart Daten aktualisieren (alle 5 Minuten)
  if (currentMillis - lastChartUpdate >= 300000) { // 5 Min
    lastChartUpdate = currentMillis;
    addChartDataPoint();
  }
}

// ========== SD-KARTE FUNKTIONEN ==========

void initSDCard() {
  Serial.print("Initialisiere SD-Karte... ");
  
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  
  if (!SD.begin(SD_CS)) {
    Serial.println("✗ FEHLER");
    Serial.println("⚠ SD-Karte nicht gefunden oder Fehler beim Initialisieren");
    Serial.println("  Bitte überprüfen:");
    Serial.println("  - SD-Karte korrekt eingesetzt?");
    Serial.println("  - SD-Karte FAT32 formatiert?");
    Serial.println("  - Verkabelung korrekt?");
    sdCardAvailable = false;
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("✗ Keine SD-Karte erkannt");
    sdCardAvailable = false;
    return;
  }

  Serial.println("✓ OK");
  Serial.print("  Kartentyp: ");
  if (cardType == CARD_MMC) Serial.println("MMC");
  else if (cardType == CARD_SD) Serial.println("SD");
  else if (cardType == CARD_SDHC) Serial.println("SDHC");
  else Serial.println("UNKNOWN");

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("  Kartengröße: %lluMB\n", cardSize);
  Serial.printf("  Freier Speicher: %lluMB\n", SD.totalBytes() / (1024 * 1024));

  sdCardAvailable = true;

  // Log-Verzeichnis erstellen
  if (!SD.exists("/logs")) {
    SD.mkdir("/logs");
    Serial.println("  Log-Verzeichnis erstellt");
  }

  // CSV Header in neue Datei schreiben wenn nötig
  createNewLogFile();
}

void createNewLogFile() {
  if (!sdCardAvailable) return;

  // Dateiname mit aktuellem Datum erstellen
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    currentLogFile = "/logs/data.csv";
  } else {
    char filename[32];
    strftime(filename, sizeof(filename), "/logs/%Y-%m-%d.csv", &timeinfo);
    currentLogFile = String(filename);
  }

  // Prüfen ob Datei schon existiert
  if (!SD.exists(currentLogFile.c_str())) {
    File file = SD.open(currentLogFile.c_str(), FILE_WRITE);
    if (file) {
      // CSV Header schreiben
      file.println("Zeitstempel,Wassertemperatur_C,Lufttemperatur_C,pH,TDS_ppm,Wasserstand,Alarm,Belueftung");
      file.close();
      Serial.println("  Neue Log-Datei erstellt: " + currentLogFile);
    } else {
      Serial.println("  ✗ Fehler beim Erstellen der Log-Datei");
    }
  }
}

void logDataToSD() {
  if (!sdCardAvailable) return;

  // Prüfen ob neue Datei für neuen Tag nötig ist
  static int lastDay = -1;
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    if (timeinfo.tm_mday != lastDay) {
      lastDay = timeinfo.tm_mday;
      createNewLogFile();
    }
  }

  File file = SD.open(currentLogFile.c_str(), FILE_APPEND);
  if (!file) {
    Serial.println("✗ Fehler beim Öffnen der Log-Datei");
    return;
  }

  // Zeitstempel
  char timestamp[32];
  if (getLocalTime(&timeinfo)) {
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
  } else {
    sprintf(timestamp, "%lu", millis() / 1000);
  }

  // CSV Zeile schreiben
  file.print(timestamp);
  file.print(",");
  file.print(waterTemp, 2);
  file.print(",");
  file.print(airTemp, 2);
  file.print(",");
  file.print(phValue, 2);
  file.print(",");
  file.print(tdsValue, 0);
  file.print(",");
  file.print(waterLevel ? "OK" : "NIEDRIG");
  file.print(",");
  file.print(alarmActive ? "JA" : "NEIN");
  file.print(",");
  file.println(aerationActive ? "AN" : "AUS");

  file.close();
  Serial.println("✓ Daten geloggt: " + String(timestamp));
}

// ========== CHART DATEN ==========

void addChartDataPoint() {
  chartData[chartDataIndex].timestamp = millis();
  chartData[chartDataIndex].waterTemp = waterTemp;
  chartData[chartDataIndex].airTemp = airTemp;
  chartData[chartDataIndex].pH = phValue;
  chartData[chartDataIndex].tds = tdsValue;

  chartDataIndex = (chartDataIndex + 1) % CHART_DATA_POINTS;
  if (chartDataCount < CHART_DATA_POINTS) {
    chartDataCount++;
  }
}

String getChartDataJSON(int hours) {
  int pointsNeeded = (hours * 60) / 5; // 5-Minuten Intervalle
  if (pointsNeeded > chartDataCount) pointsNeeded = chartDataCount;

  String json = "{";
  json += "\"labels\":[";
  
  // Zeitlabels
  for (int i = 0; i < pointsNeeded; i++) {
    int index = (chartDataIndex - pointsNeeded + i + CHART_DATA_POINTS) % CHART_DATA_POINTS;
    if (i > 0) json += ",";
    
    // Relative Zeit in Minuten (negativ = Vergangenheit)
    int minutesAgo = -(pointsNeeded - i) * 5;
    json += "\"" + String(minutesAgo) + "\"";
  }
  
  json += "],\"waterTemp\":[";
  
  // Wassertemperatur Daten
  for (int i = 0; i < pointsNeeded; i++) {
    int index = (chartDataIndex - pointsNeeded + i + CHART_DATA_POINTS) % CHART_DATA_POINTS;
    if (i > 0) json += ",";
    json += String(chartData[index].waterTemp, 1);
  }
  
  json += "],\"airTemp\":[";
  
  // Lufttemperatur Daten
  for (int i = 0; i < pointsNeeded; i++) {
    int index = (chartDataIndex - pointsNeeded + i + CHART_DATA_POINTS) % CHART_DATA_POINTS;
    if (i > 0) json += ",";
    json += String(chartData[index].airTemp, 1);
  }
  
  json += "],\"pH\":[";
  
  // pH Daten
  for (int i = 0; i < pointsNeeded; i++) {
    int index = (chartDataIndex - pointsNeeded + i + CHART_DATA_POINTS) % CHART_DATA_POINTS;
    if (i > 0) json += ",";
    json += String(chartData[index].pH, 2);
  }
  
  json += "],\"tds\":[";
  
  // TDS Daten
  for (int i = 0; i < pointsNeeded; i++) {
    int index = (chartDataIndex - pointsNeeded + i + CHART_DATA_POINTS) % CHART_DATA_POINTS;
    if (i > 0) json += ",";
    json += String(chartData[index].tds, 0);
  }
  
  json += "]}";
  return json;
}

// ========== SENSOR FUNKTIONEN ==========

void readSensors() {
  // Temperatur auslesen
  sensors.requestTemperatures();
  waterTemp = sensors.getTempC(waterThermometer);
  airTemp = sensors.getTempC(airThermometer);

  // pH Sensor auslesen
  int phRaw = 0;
  for (int i = 0; i < 10; i++) {
    phRaw += analogRead(PH_PIN);
    delay(10);
  }
  phRaw = phRaw / 10;
  float phVoltage = phRaw * (3.3 / 4095.0) * 1000.0; // in mV
  phValue = 7.0 - ((phVoltage - PH_NEUTRAL_VOLTAGE) / (PH_ACID_VOLTAGE - PH_NEUTRAL_VOLTAGE)) * 3.0;

  // TDS Sensor auslesen
  int tdsBuffer[TDS_SCOUNT];
  for (int i = 0; i < TDS_SCOUNT; i++) {
    tdsBuffer[i] = analogRead(TDS_PIN);
    delay(10);
  }
  
  // Median berechnen (robuster gegen Ausreißer)
  for (int i = 0; i < TDS_SCOUNT - 1; i++) {
    for (int j = i + 1; j < TDS_SCOUNT; j++) {
      if (tdsBuffer[i] > tdsBuffer[j]) {
        int temp = tdsBuffer[i];
        tdsBuffer[i] = tdsBuffer[j];
        tdsBuffer[j] = temp;
      }
    }
  }
  
  int tdsMedian = tdsBuffer[TDS_SCOUNT / 2];
  float tdsVoltage = tdsMedian * TDS_VREF / 4095.0;
  
  // Temperaturkompensation für TDS
  float compensationCoefficient = 1.0 + 0.02 * (waterTemp - 25.0);
  float compensationVoltage = tdsVoltage / compensationCoefficient;
  tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage 
            - 255.86 * compensationVoltage * compensationVoltage 
            + 857.39 * compensationVoltage) * 0.5;

  // Wasserstand auslesen (LOW = Wasser vorhanden bei Pull-Up)
  waterLevel = (digitalRead(WATER_LEVEL_PIN) == LOW);
}

void checkAlarms() {
  bool alarm = false;
  String alarmReason = "";

  // Wassertemperatur Check
  if (waterTemp < TEMP_MIN) {
    alarm = true;
    alarmReason = "Wassertemperatur zu niedrig: " + String(waterTemp, 1) + "°C";
  } else if (waterTemp > TEMP_MAX) {
    alarm = true;
    alarmReason = "Wassertemperatur zu hoch: " + String(waterTemp, 1) + "°C";
  }

  // pH Check
  if (phValue < PH_MIN) {
    alarm = true;
    alarmReason = "pH-Wert zu niedrig: " + String(phValue, 2);
  } else if (phValue > PH_MAX) {
    alarm = true;
    alarmReason = "pH-Wert zu hoch: " + String(phValue, 2);
  }

  // TDS Check
  if (tdsValue > TDS_MAX) {
    alarm = true;
    alarmReason = "TDS-Wert zu hoch: " + String(tdsValue, 0) + " ppm";
  }

  // Wasserstand Check
  if (!waterLevel) {
    alarm = true;
    alarmReason = "WARNUNG: Niedriger Wasserstand!";
  }

  // Alarm Status aktualisieren
  if (alarm != alarmActive) {
    alarmActive = alarm;
    if (alarm) {
      Serial.println("⚠ ALARM: " + alarmReason);
      // Automatische Belüftung bei Alarm aktivieren
      setAeration(true);
    } else {
      Serial.println("✓ Alarm beendet");
    }
  }
}

// ========== RELAIS STEUERUNG ==========

void setAeration(bool state) {
  aerationActive = state;
  digitalWrite(RELAY_AERATION, !state); // Invertiert für active-low Relais
  Serial.println("Belüftung: " + String(state ? "AN" : "AUS"));
}

// ========== WiFi SETUP ==========

void setupWiFi() {
  Serial.println("WiFi Konfiguration...");
  
  // Wenn Station Credentials vorhanden, versuche zu verbinden
  if (strlen(sta_ssid) > 0) {
    Serial.print("Verbinde mit WLAN: ");
    Serial.println(sta_ssid);
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(sta_ssid, sta_password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✓ WLAN verbunden!");
      Serial.print("  IP-Adresse: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\n⚠ WLAN Verbindung fehlgeschlagen");
    }
  } else {
    WiFi.mode(WIFI_AP);
  }
  
  // Access Point starten
  Serial.print("Starte Access Point: ");
  Serial.println(ap_ssid);
  WiFi.softAP(ap_ssid, ap_password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("✓ AP gestartet - IP-Adresse: ");
  Serial.println(IP);
  Serial.println("  Verbinde mit: " + String(ap_ssid));
  Serial.println("  Passwort: " + String(ap_password));
  Serial.println("  Web-Interface: http://" + IP.toString());
}

// ========== WEB SERVER ==========

void setupWebServer() {
  // Hauptseite
  server.on("/", HTTP_GET, handleRoot);
  
  // API Endpoints
  server.on("/api/data", HTTP_GET, handleAPIData);
  server.on("/api/chart", HTTP_GET, handleAPIChart);
  server.on("/api/control", HTTP_POST, handleAPIControl);
  server.on("/api/download", HTTP_GET, handleDownload);
  server.on("/api/sdstatus", HTTP_GET, handleSDStatus);
  
  server.begin();
  Serial.println("✓ Web Server gestartet");
}

void handleRoot() {
  String html = getHTML();
  server.send(200, "text/html", html);
}

void handleAPIData() {
  String json = "{";
  json += "\"waterTemp\":" + String(waterTemp, 2) + ",";
  json += "\"airTemp\":" + String(airTemp, 2) + ",";
  json += "\"pH\":" + String(phValue, 2) + ",";
  json += "\"tds\":" + String(tdsValue, 0) + ",";
  json += "\"waterLevel\":" + String(waterLevel ? "true" : "false") + ",";
  json += "\"alarm\":" + String(alarmActive ? "true" : "false") + ",";
  json += "\"aeration\":" + String(aerationActive ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleAPIChart() {
  int hours = 24; // Default 24 Stunden
  if (server.hasArg("hours")) {
    hours = server.arg("hours").toInt();
    if (hours < 1) hours = 1;
    if (hours > 24) hours = 24;
  }
  
  String json = getChartDataJSON(hours);
  server.send(200, "application/json", json);
}

void handleAPIControl() {
  if (!server.hasArg("action")) {
    server.send(400, "text/plain", "Missing action parameter");
    return;
  }

  String action = server.arg("action");
  
  if (action == "aeration_on") {
    setAeration(true);
    server.send(200, "text/plain", "Belüftung aktiviert");
  } else if (action == "aeration_off") {
    setAeration(false);
    server.send(200, "text/plain", "Belüftung deaktiviert");
  } else if (action == "reset_alarm") {
    alarmActive = false;
    server.send(200, "text/plain", "Alarm zurückgesetzt");
  } else {
    server.send(400, "text/plain", "Unknown action");
  }
}

void handleSDStatus() {
  String json = "{";
  json += "\"available\":" + String(sdCardAvailable ? "true" : "false") + ",";
  if (sdCardAvailable) {
    json += "\"currentFile\":\"" + currentLogFile + "\",";
    json += "\"totalMB\":" + String(SD.totalBytes() / (1024 * 1024)) + ",";
    json += "\"usedMB\":" + String(SD.usedBytes() / (1024 * 1024));
  } else {
    json += "\"error\":\"SD-Karte nicht verfügbar\"";
  }
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleDownload() {
  if (!sdCardAvailable) {
    server.send(503, "text/plain", "SD-Karte nicht verfügbar");
    return;
  }

  String filename = currentLogFile;
  if (server.hasArg("file")) {
    filename = "/logs/" + server.arg("file");
  }

  if (!SD.exists(filename.c_str())) {
    server.send(404, "text/plain", "Datei nicht gefunden");
    return;
  }

  File file = SD.open(filename.c_str(), FILE_READ);
  if (!file) {
    server.send(500, "text/plain", "Fehler beim Öffnen der Datei");
    return;
  }

  server.sendHeader("Content-Disposition", "attachment; filename=" + filename.substring(filename.lastIndexOf('/') + 1));
  server.streamFile(file, "text/csv");
  file.close();
}

// ========== WEBSOCKET ==========

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String message = String((char*)payload);
    
    if (message == "aeration_on") {
      setAeration(true);
    } else if (message == "aeration_off") {
      setAeration(false);
    } else if (message == "reset_alarm") {
      alarmActive = false;
    }
  }
}

void sendWebSocketUpdate() {
  String json = "{";
  json += "\"waterTemp\":" + String(waterTemp, 2) + ",";
  json += "\"airTemp\":" + String(airTemp, 2) + ",";
  json += "\"pH\":" + String(phValue, 2) + ",";
  json += "\"tds\":" + String(tdsValue, 0) + ",";
  json += "\"waterLevel\":" + String(waterLevel ? "true" : "false") + ",";
  json += "\"alarm\":" + String(alarmActive ? "true" : "false") + ",";
  json += "\"aeration\":" + String(aerationActive ? "true" : "false");
  json += "}";
  
  webSocket.broadcastTXT(json);
}

// ========== HTML INTERFACE ==========

String getHTML() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ForellenWächter - Lucas Haug</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js"></script>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        :root {
            --primary-blue: #1e3a8a;
            --secondary-blue: #3b82f6;
            --light-blue: #dbeafe;
            --success-green: #10b981;
            --warning-yellow: #f59e0b;
            --danger-red: #ef4444;
            --bg-gray: #f8fafc;
            --card-bg: #ffffff;
            --text-dark: #1e293b;
            --text-light: #64748b;
            --border: #e2e8f0;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
            background: linear-gradient(135deg, var(--light-blue) 0%, var(--bg-gray) 100%);
            color: var(--text-dark);
            min-height: 100vh;
            padding: 20px;
        }

        .container {
            max-width: 1400px;
            margin: 0 auto;
        }

        /* Header */
        .header {
            background: linear-gradient(135deg, var(--primary-blue) 0%, var(--secondary-blue) 100%);
            color: white;
            padding: 30px;
            border-radius: 20px;
            margin-bottom: 30px;
            box-shadow: 0 10px 30px rgba(30, 58, 138, 0.3);
            text-align: center;
        }

        .header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 15px;
        }

        .header .subtitle {
            font-size: 1.2em;
            opacity: 0.9;
            font-weight: 300;
        }

        .status-badge {
            display: inline-block;
            padding: 8px 20px;
            border-radius: 50px;
            font-weight: 600;
            font-size: 0.9em;
            margin-top: 15px;
        }

        .status-ok {
            background: var(--success-green);
            color: white;
        }

        .status-alarm {
            background: var(--danger-red);
            color: white;
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.7; }
        }

        /* Cards Grid */
        .cards-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }

        .card {
            background: var(--card-bg);
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.07);
            transition: transform 0.2s, box-shadow 0.2s;
        }

        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 8px 15px rgba(0, 0, 0, 0.1);
        }

        .card-header {
            display: flex;
            align-items: center;
            gap: 12px;
            margin-bottom: 15px;
            color: var(--text-light);
            font-size: 0.9em;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }

        .card-icon {
            font-size: 1.5em;
        }

        .card-value {
            font-size: 2.5em;
            font-weight: 700;
            color: var(--primary-blue);
            margin-bottom: 5px;
        }

        .card-unit {
            color: var(--text-light);
            font-size: 0.9em;
        }

        .card-status {
            margin-top: 10px;
            padding: 8px 15px;
            border-radius: 8px;
            font-size: 0.85em;
            font-weight: 600;
            text-align: center;
        }

        .status-normal {
            background: #d1fae5;
            color: var(--success-green);
        }

        .status-warning {
            background: #fed7aa;
            color: var(--warning-yellow);
        }

        .status-critical {
            background: #fee2e2;
            color: var(--danger-red);
        }

        /* Charts Section */
        .charts-section {
            background: var(--card-bg);
            border-radius: 15px;
            padding: 30px;
            margin-bottom: 30px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.07);
        }

        .charts-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 20px;
            flex-wrap: wrap;
            gap: 15px;
        }

        .charts-header h2 {
            color: var(--primary-blue);
            font-size: 1.5em;
        }

        .time-buttons {
            display: flex;
            gap: 10px;
        }

        .time-btn {
            padding: 8px 16px;
            border: 2px solid var(--secondary-blue);
            background: white;
            color: var(--secondary-blue);
            border-radius: 8px;
            cursor: pointer;
            font-weight: 600;
            transition: all 0.2s;
        }

        .time-btn:hover, .time-btn.active {
            background: var(--secondary-blue);
            color: white;
        }

        .chart-container {
            height: 300px;
            margin-bottom: 30px;
        }

        /* Controls Section */
        .controls-section {
            background: var(--card-bg);
            border-radius: 15px;
            padding: 30px;
            margin-bottom: 30px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.07);
        }

        .controls-section h2 {
            color: var(--primary-blue);
            margin-bottom: 20px;
            font-size: 1.5em;
        }

        .control-group {
            display: flex;
            gap: 15px;
            flex-wrap: wrap;
            margin-bottom: 20px;
        }

        .btn {
            padding: 12px 24px;
            border: none;
            border-radius: 10px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s;
            font-size: 1em;
        }

        .btn-primary {
            background: var(--secondary-blue);
            color: white;
        }

        .btn-primary:hover {
            background: var(--primary-blue);
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(59, 130, 246, 0.3);
        }

        .btn-success {
            background: var(--success-green);
            color: white;
        }

        .btn-success:hover {
            background: #059669;
            transform: translateY(-2px);
        }

        .btn-danger {
            background: var(--danger-red);
            color: white;
        }

        .btn-danger:hover {
            background: #dc2626;
            transform: translateY(-2px);
        }

        /* SD Card Section */
        .sd-section {
            background: var(--card-bg);
            border-radius: 15px;
            padding: 30px;
            margin-bottom: 30px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.07);
        }

        .sd-section h2 {
            color: var(--primary-blue);
            margin-bottom: 20px;
            font-size: 1.5em;
        }

        .sd-status {
            display: flex;
            align-items: center;
            gap: 10px;
            margin-bottom: 15px;
            padding: 15px;
            background: var(--light-blue);
            border-radius: 10px;
        }

        .sd-icon {
            font-size: 2em;
        }

        .sd-info {
            flex: 1;
        }

        .sd-info p {
            margin: 5px 0;
            color: var(--text-light);
        }

        /* Footer */
        .footer {
            text-align: center;
            padding: 20px;
            color: var(--text-light);
            font-size: 0.9em;
        }

        /* Responsive */
        @media (max-width: 768px) {
            .header h1 {
                font-size: 1.8em;
            }

            .cards-grid {
                grid-template-columns: 1fr;
            }

            .charts-header {
                flex-direction: column;
                align-items: flex-start;
            }

            .time-buttons {
                width: 100%;
                justify-content: space-between;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <!-- Header -->
        <div class="header">
            <h1>
                <span style="font-size: 1.2em;">🐟</span>
                ForellenWächter
            </h1>
            <div class="subtitle">Lucas Haug's Forellenzucht</div>
            <div id="systemStatus" class="status-badge status-ok">System OK</div>
        </div>

        <!-- Sensor Cards -->
        <div class="cards-grid">
            <!-- Wassertemperatur -->
            <div class="card">
                <div class="card-header">
                    <span class="card-icon">💧</span>
                    <span>Wassertemperatur</span>
                </div>
                <div class="card-value" id="waterTemp">--</div>
                <div class="card-unit">°C</div>
                <div id="waterTempStatus" class="card-status status-normal">Normal</div>
            </div>

            <!-- Lufttemperatur -->
            <div class="card">
                <div class="card-header">
                    <span class="card-icon">🌡️</span>
                    <span>Lufttemperatur</span>
                </div>
                <div class="card-value" id="airTemp">--</div>
                <div class="card-unit">°C</div>
                <div id="airTempStatus" class="card-status status-normal">Normal</div>
            </div>

            <!-- pH-Wert -->
            <div class="card">
                <div class="card-header">
                    <span class="card-icon">⚗️</span>
                    <span>pH-Wert</span>
                </div>
                <div class="card-value" id="pH">--</div>
                <div class="card-unit">pH</div>
                <div id="pHStatus" class="card-status status-normal">Normal</div>
            </div>

            <!-- TDS -->
            <div class="card">
                <div class="card-header">
                    <span class="card-icon">💎</span>
                    <span>Wasserqualität (TDS)</span>
                </div>
                <div class="card-value" id="tds">--</div>
                <div class="card-unit">ppm</div>
                <div id="tdsStatus" class="card-status status-normal">Normal</div>
            </div>

            <!-- Wasserstand -->
            <div class="card">
                <div class="card-header">
                    <span class="card-icon">🌊</span>
                    <span>Wasserstand</span>
                </div>
                <div class="card-value" id="waterLevel" style="font-size: 1.8em;">--</div>
                <div id="waterLevelStatus" class="card-status status-normal">Normal</div>
            </div>

            <!-- Belüftung -->
            <div class="card">
                <div class="card-header">
                    <span class="card-icon">🫧</span>
                    <span>Belüftung</span>
                </div>
                <div class="card-value" id="aeration" style="font-size: 1.8em;">--</div>
                <div id="aerationStatus" class="card-status status-normal">Aus</div>
            </div>
        </div>

        <!-- Charts Section -->
        <div class="charts-section">
            <div class="charts-header">
                <h2>📊 Datenverläufe</h2>
                <div class="time-buttons">
                    <button class="time-btn" onclick="updateCharts(6)">6h</button>
                    <button class="time-btn" onclick="updateCharts(12)">12h</button>
                    <button class="time-btn active" onclick="updateCharts(24)">24h</button>
                </div>
            </div>

            <div class="chart-container">
                <canvas id="tempChart"></canvas>
            </div>

            <div class="chart-container">
                <canvas id="phChart"></canvas>
            </div>

            <div class="chart-container">
                <canvas id="tdsChart"></canvas>
            </div>
        </div>

        <!-- Controls -->
        <div class="controls-section">
            <h2>⚙️ Steuerung</h2>
            <div class="control-group">
                <button class="btn btn-success" onclick="controlAeration('on')">
                    Belüftung AN
                </button>
                <button class="btn btn-danger" onclick="controlAeration('off')">
                    Belüftung AUS
                </button>
                <button class="btn btn-primary" onclick="resetAlarm()">
                    Alarm Zurücksetzen
                </button>
            </div>
        </div>

        <!-- SD Card Section -->
        <div class="sd-section">
            <h2>💾 Datenaufzeichnung</h2>
            <div id="sdStatus" class="sd-status">
                <span class="sd-icon">📝</span>
                <div class="sd-info">
                    <p><strong>Status:</strong> <span id="sdStatusText">Prüfe...</span></p>
                    <p><strong>Aktuelle Datei:</strong> <span id="sdCurrentFile">--</span></p>
                    <p><strong>Speicher:</strong> <span id="sdStorage">--</span></p>
                </div>
            </div>
            <button class="btn btn-primary" onclick="downloadData()">
                📥 CSV-Daten Herunterladen
            </button>
        </div>

        <!-- Footer -->
        <div class="footer">
            <p>ForellenWächter v1.2 • Entwickelt für Lucas Haug</p>
            <p style="margin-top: 5px; opacity: 0.7;">MIT Lizenz • Andreas S. (AndreasS964)</p>
        </div>
    </div>

    <script>
        // WebSocket Verbindung
        let ws;
        let tempChart, phChart, tdsChart;
        let currentTimeRange = 24;

        function connectWebSocket() {
            const wsUrl = 'ws://' + window.location.hostname + ':81';
            ws = new WebSocket(wsUrl);

            ws.onopen = () => {
                console.log('WebSocket verbunden');
            };

            ws.onmessage = (event) => {
                const data = JSON.parse(event.data);
                updateDisplay(data);
            };

            ws.onclose = () => {
                console.log('WebSocket getrennt, versuche Neuverbindung...');
                setTimeout(connectWebSocket, 3000);
            };

            ws.onerror = (error) => {
                console.error('WebSocket Fehler:', error);
            };
        }

        function updateDisplay(data) {
            // Wassertemperatur
            document.getElementById('waterTemp').textContent = data.waterTemp.toFixed(1);
            updateStatus('waterTempStatus', data.waterTemp, 12.0, 18.0);

            // Lufttemperatur
            document.getElementById('airTemp').textContent = data.airTemp.toFixed(1);
            updateStatus('airTempStatus', data.airTemp, -10, 30);

            // pH
            document.getElementById('pH').textContent = data.pH.toFixed(2);
            updateStatus('pHStatus', data.pH, 6.5, 8.5);

            // TDS
            document.getElementById('tds').textContent = Math.round(data.tds);
            updateStatus('tdsStatus', data.tds, 0, 300);

            // Wasserstand
            document.getElementById('waterLevel').textContent = data.waterLevel ? 'OK' : 'NIEDRIG';
            const wlStatus = document.getElementById('waterLevelStatus');
            if (data.waterLevel) {
                wlStatus.textContent = 'Normal';
                wlStatus.className = 'card-status status-normal';
            } else {
                wlStatus.textContent = 'NIEDRIG';
                wlStatus.className = 'card-status status-critical';
            }

            // Belüftung
            document.getElementById('aeration').textContent = data.aeration ? 'AN' : 'AUS';
            const aerStatus = document.getElementById('aerationStatus');
            if (data.aeration) {
                aerStatus.textContent = 'Aktiv';
                aerStatus.className = 'card-status status-normal';
            } else {
                aerStatus.textContent = 'Aus';
                aerStatus.className = 'card-status status-warning';
            }

            // System Status
            const systemStatus = document.getElementById('systemStatus');
            if (data.alarm) {
                systemStatus.textContent = '⚠ ALARM';
                systemStatus.className = 'status-badge status-alarm';
            } else {
                systemStatus.textContent = '✓ System OK';
                systemStatus.className = 'status-badge status-ok';
            }
        }

        function updateStatus(elementId, value, min, max) {
            const element = document.getElementById(elementId);
            if (value < min || value > max) {
                element.textContent = 'Warnung';
                element.className = 'card-status status-critical';
            } else {
                element.textContent = 'Normal';
                element.className = 'card-status status-normal';
            }
        }

        function controlAeration(action) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send('aeration_' + action);
            }
        }

        function resetAlarm() {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send('reset_alarm');
            }
        }

        function updateCharts(hours) {
            currentTimeRange = hours;
            
            // Button Aktivierung
            document.querySelectorAll('.time-btn').forEach(btn => {
                btn.classList.remove('active');
            });
            event.target.classList.add('active');

            fetch('/api/chart?hours=' + hours)
                .then(response => response.json())
                .then(data => {
                    updateChartData(data);
                });
        }

        function updateChartData(data) {
            // Temperatur Chart
            tempChart.data.labels = data.labels;
            tempChart.data.datasets[0].data = data.waterTemp;
            tempChart.data.datasets[1].data = data.airTemp;
            tempChart.update();

            // pH Chart
            phChart.data.labels = data.labels;
            phChart.data.datasets[0].data = data.pH;
            phChart.update();

            // TDS Chart
            tdsChart.data.labels = data.labels;
            tdsChart.data.datasets[0].data = data.tds;
            tdsChart.update();
        }

        function initCharts() {
            const commonOptions = {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        display: true,
                        position: 'top'
                    }
                },
                scales: {
                    x: {
                        title: {
                            display: true,
                            text: 'Zeit (Minuten)'
                        }
                    }
                }
            };

            // Temperatur Chart
            const tempCtx = document.getElementById('tempChart').getContext('2d');
            tempChart = new Chart(tempCtx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'Wassertemperatur (°C)',
                        data: [],
                        borderColor: 'rgb(59, 130, 246)',
                        backgroundColor: 'rgba(59, 130, 246, 0.1)',
                        tension: 0.4
                    }, {
                        label: 'Lufttemperatur (°C)',
                        data: [],
                        borderColor: 'rgb(245, 158, 11)',
                        backgroundColor: 'rgba(245, 158, 11, 0.1)',
                        tension: 0.4
                    }]
                },
                options: {
                    ...commonOptions,
                    scales: {
                        ...commonOptions.scales,
                        y: {
                            title: {
                                display: true,
                                text: 'Temperatur (°C)'
                            }
                        }
                    }
                }
            });

            // pH Chart
            const phCtx = document.getElementById('phChart').getContext('2d');
            phChart = new Chart(phCtx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'pH-Wert',
                        data: [],
                        borderColor: 'rgb(16, 185, 129)',
                        backgroundColor: 'rgba(16, 185, 129, 0.1)',
                        tension: 0.4
                    }]
                },
                options: {
                    ...commonOptions,
                    scales: {
                        ...commonOptions.scales,
                        y: {
                            title: {
                                display: true,
                                text: 'pH'
                            },
                            min: 6,
                            max: 9
                        }
                    }
                }
            });

            // TDS Chart
            const tdsCtx = document.getElementById('tdsChart').getContext('2d');
            tdsChart = new Chart(tdsCtx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'TDS (ppm)',
                        data: [],
                        borderColor: 'rgb(139, 92, 246)',
                        backgroundColor: 'rgba(139, 92, 246, 0.1)',
                        tension: 0.4
                    }]
                },
                options: {
                    ...commonOptions,
                    scales: {
                        ...commonOptions.scales,
                        y: {
                            title: {
                                display: true,
                                text: 'ppm'
                            }
                        }
                    }
                }
            });
        }

        function updateSDStatus() {
            fetch('/api/sdstatus')
                .then(response => response.json())
                .then(data => {
                    const statusText = document.getElementById('sdStatusText');
                    const currentFile = document.getElementById('sdCurrentFile');
                    const storage = document.getElementById('sdStorage');

                    if (data.available) {
                        statusText.textContent = '✓ Aktiv';
                        statusText.style.color = 'var(--success-green)';
                        currentFile.textContent = data.currentFile;
                        storage.textContent = data.usedMB + ' MB / ' + data.totalMB + ' MB';
                    } else {
                        statusText.textContent = '✗ Nicht verfügbar';
                        statusText.style.color = 'var(--danger-red)';
                        currentFile.textContent = '--';
                        storage.textContent = '--';
                    }
                });
        }

        function downloadData() {
            window.location.href = '/api/download';
        }

        // Initialisierung
        window.addEventListener('load', () => {
            connectWebSocket();
            initCharts();
            updateCharts(24);
            updateSDStatus();
            
            // SD Status alle 30 Sekunden aktualisieren
            setInterval(updateSDStatus, 30000);
            
            // Charts alle 5 Minuten aktualisieren
            setInterval(() => updateCharts(currentTimeRange), 300000);
        });
    </script>
</body>
</html>
)rawliteral";

  return html;
}
