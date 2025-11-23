/*
 * ForellenWächter v2.0 - Konfigurationsdatei
 * Alle Einstellungen zentral verwaltet
 */

#ifndef CONFIG_H
#define CONFIG_H

// ========== VERSION ==========
#define FW_VERSION "2.0.0"
#define FW_BUILD_DATE __DATE__

// ========== WIFI KONFIGURATION ==========
const char* AP_SSID = "ForellenWaechter";
const char* AP_PASSWORD = "Lucas2024";

// Optional: Station Mode (leer lassen für reinen AP Mode)
const char* STA_SSID = "";
const char* STA_PASSWORD = "";

// WiFi Hostname
const char* HOSTNAME = "forellenwaechter";

// ========== WEB-SERVER KONFIGURATION ==========
#define WEB_SERVER_PORT 80
#define WEBSOCKET_PORT 81

// Login Credentials (leer = kein Login erforderlich)
const char* WEB_USERNAME = "admin";
const char* WEB_PASSWORD = "forelle2024";

// ========== PIN DEFINITIONEN ==========
// Temperatursensoren (OneWire)
#define ONE_WIRE_BUS 4

// Analoge Sensoren
#define PH_PIN 34              // pH Sensor
#define TDS_PIN 35             // TDS Sensor

// Digitale Eingänge
#define WATER_LEVEL_PIN 25     // Float Switch

// Relais Ausgänge (Active LOW)
#define RELAY_AERATION 26      // Belüftung
#define RELAY_2 27             // Reserve
#define RELAY_3 14             // Reserve
#define RELAY_4 12             // Reserve

// SD-Karte (SPI)
#define SD_CS 5
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18

// Optional: Status LED
#define STATUS_LED 2           // Built-in LED

// ========== SENSOR KALIBRIERUNG ==========
// pH Sensor
#define PH_NEUTRAL_VOLTAGE 1500.0  // mV bei pH 7.0
#define PH_ACID_VOLTAGE 2032.0     // mV bei pH 4.0

// TDS Sensor
#define TDS_VREF 3.3               // ESP32 ADC Referenzspannung
#define TDS_SCOUNT 30              // Anzahl Samples

// ========== GRENZWERTE ==========
// Forellen-optimale Werte
#define TEMP_MIN 8.0           // Minimum Wassertemperatur (°C)
#define TEMP_OPTIMAL_MIN 10.0  // Optimal min
#define TEMP_OPTIMAL_MAX 14.0  // Optimal max
#define TEMP_MAX 18.0          // Maximum Wassertemperatur (°C)
#define TEMP_CRITICAL 20.0     // Kritisch

#define PH_MIN 6.5             // Minimum pH
#define PH_OPTIMAL_MIN 7.0     // Optimal min
#define PH_OPTIMAL_MAX 8.0     // Optimal max
#define PH_MAX 8.5             // Maximum pH

#define TDS_MIN 50             // Minimum TDS (ppm)
#define TDS_OPTIMAL_MAX 200    // Optimal max
#define TDS_MAX 300            // Maximum TDS (ppm)

// ========== TIMING KONFIGURATION ==========
#define SENSOR_INTERVAL 5000       // Sensor-Ablesung alle 5 Sekunden
#define LOG_INTERVAL 600000        // SD-Logging alle 10 Minuten
#define CHART_UPDATE_INTERVAL 300000  // Chart Update alle 5 Minuten
#define WEBSOCKET_UPDATE_INTERVAL 2000 // WebSocket Update alle 2s

// Chart Datenpunkte (24h bei 5-Min Intervallen = 288 Punkte)
#define CHART_DATA_POINTS 288

// ========== ENERGIE-MANAGEMENT ==========
#define ENABLE_POWER_SAVE true     // Power Save aktivieren
#define DEEP_SLEEP_ENABLED false   // Deep Sleep (nur wenn keine WiFi-Verbindung nötig)
#define SLEEP_DURATION_SEC 60      // Deep Sleep Dauer in Sekunden

// WiFi Power Save
#define WIFI_POWER_SAVE true       // WiFi Modem Sleep aktivieren
#define WIFI_CONNECT_TIMEOUT 20    // WiFi Verbindungs-Timeout (Sekunden)

// CPU Frequenz (80MHz für Energiesparen, 240MHz für Performance)
#define CPU_FREQUENCY 80           // 80, 160 oder 240 MHz

// Sensor Power Save
#define SENSOR_WARMUP_TIME 100     // Sensor Aufwärmzeit (ms)
#define REDUCE_SENSOR_FREQUENCY false  // Bei Batterie: seltener messen

// Display Power Save
#define DIM_LED_BRIGHTNESS 20      // LED Helligkeit (0-255)

// ========== MQTT KONFIGURATION (OPTIONAL) ==========
#define MQTT_ENABLED false         // MQTT aktivieren
const char* MQTT_SERVER = "";      // MQTT Broker IP
#define MQTT_PORT 1883
const char* MQTT_USER = "";
const char* MQTT_PASSWORD = "";
const char* MQTT_TOPIC_PREFIX = "forellen/";

// ========== NTP KONFIGURATION ==========
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 3600;      // GMT+1 (Deutschland)
const int DAYLIGHT_OFFSET_SEC = 3600;  // Sommerzeit

// ========== LOGGING KONFIGURATION ==========
#define ENABLE_SERIAL_DEBUG true   // Serial Debug Output
#define SERIAL_BAUD_RATE 115200
#define LOG_TO_SD true             // Daten auf SD-Karte loggen
#define LOG_DIR "/logs"
#define MAX_LOG_FILE_SIZE 5242880  // 5MB

// ========== ALARMIERUNG ==========
#define ENABLE_ALARM_SOUND false   // Piezo-Summer
#define ALARM_AUTO_AERATION true   // Bei Alarm Belüftung aktivieren
#define ALARM_COOLDOWN 300000      // Alarm Cooldown 5 Min

// ========== WATCHDOG ==========
#define WATCHDOG_TIMEOUT 30        // Watchdog Timeout in Sekunden

// ========== FEATURES ==========
#define ENABLE_OTA_UPDATE true     // Over-The-Air Updates
#define ENABLE_MDNS true           // mDNS (http://forellenwaechter.local)
#define ENABLE_WEB_AUTH true       // Web-Authentifizierung
#define ENABLE_API true            // REST API
#define ENABLE_WEBSOCKET true      // WebSocket für Live-Updates

#endif // CONFIG_H
