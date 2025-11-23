/*
 * ═══════════════════════════════════════════════════════════
 *  ForellenWächter v2.0 - Aquakultur Monitoring System
 * ═══════════════════════════════════════════════════════════
 *
 *  Entwickelt für Lucas Haug's Forellenzucht
 *  Off-Grid optimiert mit modernisiertem UI
 *
 *  PlatformIO Version
 *
 *  Lizenz: MIT
 *  Author: Andreas S. (AndreasS964)
 *  Version: 2.0.0
 * ═══════════════════════════════════════════════════════════
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <SD.h>
#include <SPI.h>
#include <time.h>

// Module einbinden
#include "config.h"
#include "power_management.h"
#include "sensors.h"
#include "webserver.h"

// Optional: MQTT
#if MQTT_ENABLED
#include <PubSubClient.h>
WiFiClient mqttWiFiClient;
PubSubClient mqttClient(mqttWiFiClient);
#endif

// ========== GLOBALE OBJEKTE ==========

PowerManager powerManager;
SensorManager sensorManager;

// ========== GLOBALE VARIABLEN ==========

// Relais Status
bool aerationActive = false;
bool alarmActive = false;

// Timing
unsigned long lastSensorRead = 0;
unsigned long lastLogWrite = 0;
unsigned long lastWebSocketUpdate = 0;
unsigned long lastStatsReset = 0;

// SD-Karte
bool sdCardAvailable = false;
String currentLogFile = "";

// ========== SETUP ==========

void setup() {
    // Serial Debug
    #if ENABLE_SERIAL_DEBUG
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);
    Serial.println("\n\n");
    Serial.println("═══════════════════════════════════════");
    Serial.println("   ForellenWächter v2.0");
    Serial.println("   Lucas Haug's Forellenzucht");
    Serial.println("   Off-Grid Optimiert");
    Serial.println("═══════════════════════════════════════");
    Serial.printf("Build: %s\n", BUILD_DATE);
    Serial.println();
    #endif

    // Relais Pins konfigurieren
    pinMode(RELAY_AERATION, OUTPUT);
    pinMode(RELAY_2, OUTPUT);
    pinMode(RELAY_3, OUTPUT);
    pinMode(RELAY_4, OUTPUT);

    // Relais initial aus (HIGH = aus bei active-low)
    digitalWrite(RELAY_AERATION, HIGH);
    digitalWrite(RELAY_2, HIGH);
    digitalWrite(RELAY_3, HIGH);
    digitalWrite(RELAY_4, HIGH);

    // Energie-Management initialisieren
    powerManager.begin();

    // Sensoren initialisieren
    if (!sensorManager.begin()) {
        Serial.println("⚠ WARNUNG: Sensor-Initialisierung fehlgeschlagen!");
    }

    // SD-Karte initialisieren (optional)
    #if LOG_TO_SD
    initSDCard();
    #endif

    // WiFi starten
    setupWiFi();

    // mDNS starten
    #if ENABLE_MDNS
    if (MDNS.begin(HOSTNAME)) {
        Serial.println("✓ mDNS gestartet: http://" + String(HOSTNAME) + ".local");
        MDNS.addService("http", "tcp", WEB_SERVER_PORT);
    }
    #endif

    // NTP Zeit synchronisieren
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    Serial.println("✓ NTP Zeitsynchronisation gestartet");

    // Web Server & WebSocket starten
    setupWebServer();

    // OTA Updates konfigurieren
    #if ENABLE_OTA_UPDATE
    setupOTA();
    #endif

    // MQTT (optional)
    #if MQTT_ENABLED
    setupMQTT();
    #endif

    Serial.println("\n✓✓✓ SYSTEM BEREIT ✓✓✓\n");
    Serial.println("Web-Interface: http://" + WiFi.localIP().toString());
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        Serial.println("Access Point: http://" + WiFi.softAPIP().toString());
    }
    Serial.println();

    // Initiale Sensor-Lesung
    sensorManager.readAll();
}

// ========== MAIN LOOP ==========

void loop() {
    unsigned long currentMillis = millis();

    // Watchdog füttern
    powerManager.feedWatchdog();

    // Web Server & WebSocket
    server.handleClient();
    webSocket.loop();

    // OTA Updates
    #if ENABLE_OTA_UPDATE
    ArduinoOTA.handle();
    #endif

    // MQTT
    #if MQTT_ENABLED
    if (mqttClient.connected()) {
        mqttClient.loop();
    } else {
        reconnectMQTT();
    }
    #endif

    // Sensoren auslesen
    if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
        lastSensorRead = currentMillis;

        sensorManager.readAll();
        checkAlarms();

        // Batterie-Check (falls vorhanden)
        if (powerManager.isBatteryLow()) {
            powerManager.enablePowerSaveMode();
        }
    }

    // WebSocket Update
    if (currentMillis - lastWebSocketUpdate >= WEBSOCKET_UPDATE_INTERVAL) {
        lastWebSocketUpdate = currentMillis;
        sendWebSocketUpdate();
    }

    // SD-Karte Logging
    #if LOG_TO_SD
    if (currentMillis - lastLogWrite >= LOG_INTERVAL) {
        lastLogWrite = currentMillis;
        logDataToSD();
    }
    #endif

    // Statistiken täglich zurücksetzen
    if (currentMillis - lastStatsReset >= 86400000) { // 24h
        lastStatsReset = currentMillis;
        sensorManager.resetStats();
        Serial.println("✓ Statistiken zurückgesetzt (24h)");
    }

    // Kurze Pause für Stabilität
    delay(10);
}

// ========== WIFI SETUP ==========

void setupWiFi() {
    Serial.println("\n=== WiFi Konfiguration ===");

    WiFi.setHostname(HOSTNAME);

    // Station Mode versuchen (wenn Credentials vorhanden)
    if (strlen(STA_SSID) > 0) {
        Serial.print("Verbinde mit WLAN: ");
        Serial.println(STA_SSID);

        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(STA_SSID, STA_PASSWORD);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < WIFI_CONNECT_TIMEOUT) {
            delay(500);
            Serial.print(".");
            attempts++;
            powerManager.feedWatchdog();
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n✓ WLAN verbunden!");
            Serial.print("  IP-Adresse: ");
            Serial.println(WiFi.localIP());
            Serial.print("  Signal: ");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm");
        } else {
            Serial.println("\n⚠ WLAN Verbindung fehlgeschlagen, nutze nur AP-Modus");
            WiFi.mode(WIFI_AP);
        }
    } else {
        WiFi.mode(WIFI_AP);
    }

    // Access Point starten
    Serial.print("Starte Access Point: ");
    Serial.println(AP_SSID);

    WiFi.softAP(AP_SSID, AP_PASSWORD);
    IPAddress apIP = WiFi.softAPIP();

    Serial.print("✓ AP gestartet - IP: ");
    Serial.println(apIP);
    Serial.println("  SSID: " + String(AP_SSID));
    Serial.println("  Passwort: " + String(AP_PASSWORD));
    Serial.println();

    // WiFi Power Save
    #if WIFI_POWER_SAVE
    powerManager.setWiFiPowerSave(true);
    #endif
}

// ========== OTA UPDATES ==========

#if ENABLE_OTA_UPDATE
void setupOTA() {
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword(WEB_PASSWORD);

    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "Sketch" : "Filesystem";
        Serial.println("OTA Update gestartet: " + type);
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\n✓ OTA Update abgeschlossen");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        powerManager.feedWatchdog();
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("✗ OTA Fehler [%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
    Serial.println("✓ OTA Updates aktiviert");
}
#endif

// ========== MQTT ==========

#if MQTT_ENABLED
void setupMQTT() {
    if (strlen(MQTT_SERVER) > 0) {
        mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
        Serial.println("✓ MQTT konfiguriert: " + String(MQTT_SERVER));
        reconnectMQTT();
    }
}

void reconnectMQTT() {
    static unsigned long lastAttempt = 0;
    if (millis() - lastAttempt < 5000) return; // Nicht öfter als alle 5s

    lastAttempt = millis();

    if (mqttClient.connect(HOSTNAME, MQTT_USER, MQTT_PASSWORD)) {
        Serial.println("✓ MQTT verbunden");
        publishMQTTData();
    }
}

void publishMQTTData() {
    if (!mqttClient.connected()) return;

    SensorData data = sensorManager.getData();
    String topic = String(MQTT_TOPIC_PREFIX);

    mqttClient.publish((topic + "waterTemp").c_str(), String(data.waterTemp, 2).c_str());
    mqttClient.publish((topic + "airTemp").c_str(), String(data.airTemp, 2).c_str());
    mqttClient.publish((topic + "pH").c_str(), String(data.pH, 2).c_str());
    mqttClient.publish((topic + "tds").c_str(), String(data.tds, 0).c_str());
    mqttClient.publish((topic + "waterLevel").c_str(), data.waterLevel ? "OK" : "LOW");
    mqttClient.publish((topic + "aeration").c_str(), aerationActive ? "ON" : "OFF");
    mqttClient.publish((topic + "alarm").c_str(), alarmActive ? "ACTIVE" : "OK");
}
#endif

// ========== SD-KARTE ==========

#if LOG_TO_SD
void initSDCard() {
    Serial.print("Initialisiere SD-Karte... ");

    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    if (!SD.begin(SD_CS)) {
        Serial.println("✗ FEHLER");
        Serial.println("⚠ SD-Karte nicht verfügbar (nicht kritisch)");
        sdCardAvailable = false;
        return;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("✗ Keine SD-Karte");
        sdCardAvailable = false;
        return;
    }

    Serial.println("✓ OK");
    Serial.printf("  Typ: %s\n", cardType == CARD_MMC ? "MMC" : cardType == CARD_SD ? "SD" : "SDHC");
    Serial.printf("  Größe: %llu MB\n", SD.cardSize() / (1024 * 1024));

    sdCardAvailable = true;

    // Log-Verzeichnis erstellen
    if (!SD.exists(LOG_DIR)) {
        SD.mkdir(LOG_DIR);
    }

    createNewLogFile();
}

void createNewLogFile() {
    if (!sdCardAvailable) return;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        currentLogFile = String(LOG_DIR) + "/data.csv";
    } else {
        char filename[64];
        strftime(filename, sizeof(filename), "/logs/%Y-%m-%d.csv", &timeinfo);
        currentLogFile = String(filename);
    }

    if (!SD.exists(currentLogFile.c_str())) {
        File file = SD.open(currentLogFile.c_str(), FILE_WRITE);
        if (file) {
            file.println("Zeitstempel,Wasser_C,Luft_C,pH,TDS_ppm,Wasserstand,Alarm,Belueftung");
            file.close();
            Serial.println("✓ Log-Datei erstellt: " + currentLogFile);
        }
    }
}

void logDataToSD() {
    if (!sdCardAvailable) return;

    // Prüfen ob neuer Tag = neue Datei
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
        Serial.println("✗ SD Log Fehler");
        return;
    }

    SensorData data = sensorManager.getData();

    // Zeitstempel
    char timestamp[32];
    if (getLocalTime(&timeinfo)) {
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    } else {
        sprintf(timestamp, "%lu", millis() / 1000);
    }

    // CSV schreiben
    file.printf("%s,%.2f,%.2f,%.2f,%.0f,%s,%s,%s\n",
                timestamp,
                data.waterTemp,
                data.airTemp,
                data.pH,
                data.tds,
                data.waterLevel ? "OK" : "LOW",
                alarmActive ? "YES" : "NO",
                aerationActive ? "ON" : "OFF");

    file.close();

    #if ENABLE_SERIAL_DEBUG
    Serial.println("✓ Daten geloggt: " + String(timestamp));
    #endif
}
#endif

// ========== ALARM SYSTEM ==========

void checkAlarms() {
    SensorData data = sensorManager.getData();
    bool alarm = false;
    String reason = "";

    // Wassertemperatur
    if (data.waterTemp < TEMP_MIN) {
        alarm = true;
        reason = "Wassertemp zu niedrig: " + String(data.waterTemp, 1) + "°C";
    } else if (data.waterTemp > TEMP_MAX) {
        alarm = true;
        reason = "Wassertemp zu hoch: " + String(data.waterTemp, 1) + "°C";
    }

    // pH-Wert
    if (data.pH < PH_MIN) {
        alarm = true;
        reason = "pH zu niedrig: " + String(data.pH, 2);
    } else if (data.pH > PH_MAX) {
        alarm = true;
        reason = "pH zu hoch: " + String(data.pH, 2);
    }

    // TDS
    if (data.tds > TDS_MAX) {
        alarm = true;
        reason = "TDS zu hoch: " + String(data.tds, 0) + " ppm";
    }

    // Wasserstand
    if (!data.waterLevel) {
        alarm = true;
        reason = "NIEDRIGER WASSERSTAND!";
    }

    // Alarm-Status ändern
    if (alarm && !alarmActive) {
        alarmActive = true;
        Serial.println("⚠⚠⚠ ALARM: " + reason);

        #if ALARM_AUTO_AERATION
        setAeration(true);
        #endif

        #if MQTT_ENABLED
        if (mqttClient.connected()) {
            mqttClient.publish((String(MQTT_TOPIC_PREFIX) + "alarm/reason").c_str(), reason.c_str());
        }
        #endif
    } else if (!alarm && alarmActive) {
        alarmActive = false;
        Serial.println("✓ Alarm beendet");
    }
}

// ========== RELAIS STEUERUNG ==========

void setAeration(bool state) {
    aerationActive = state;
    digitalWrite(RELAY_AERATION, !state); // Invertiert für active-low
    Serial.println("Belüftung: " + String(state ? "AN" : "AUS"));

    #if MQTT_ENABLED
    if (mqttClient.connected()) {
        mqttClient.publish((String(MQTT_TOPIC_PREFIX) + "aeration").c_str(), state ? "ON" : "OFF");
    }
    #endif
}

// ========== WEBSOCKET ==========

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_TEXT) {
        String message = String((char*)payload);

        if (message == "aeration_on") {
            setAeration(true);
        } else if (message == "aeration_off") {
            setAeration(false);
        } else if (message == "reset_alarm") {
            alarmActive = false;
            Serial.println("✓ Alarm manuell zurückgesetzt");
        }
    }
}

void sendWebSocketUpdate() {
    SensorData data = sensorManager.getData();

    String json = "{";
    json += "\"waterTemp\":" + String(data.waterTemp, 2) + ",";
    json += "\"airTemp\":" + String(data.airTemp, 2) + ",";
    json += "\"pH\":" + String(data.pH, 2) + ",";
    json += "\"tds\":" + String(data.tds, 0) + ",";
    json += "\"waterLevel\":" + String(data.waterLevel ? "true" : "false") + ",";
    json += "\"alarm\":" + String(alarmActive ? "true" : "false") + ",";
    json += "\"aeration\":" + String(aerationActive ? "true" : "false");
    json += "}";

    webSocket.broadcastTXT(json);
}
