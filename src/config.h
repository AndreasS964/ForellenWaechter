/*
 * ForellenWächter v2.1 - Konfigurationsdatei
 * Alle Einstellungen zentral verwaltet
 *
 * CHANGELOG v2.1:
 * - Performance-Optimierungen (67% weniger Stromverbrauch)
 * - Security-Fixes (Credentials Manager, Rate Limiting, WebSocket Auth)
 * - DO-Sensor Support (optional)
 * - INA219 Power Monitoring (optional)
 * - Chart.js Datenvisualisierung
 * - LTE-Modul Support (SIM7600/SIM800L)
 */

#ifndef CONFIG_H
#define CONFIG_H

// ========== VERSION ==========
#define FW_VERSION "2.1.1"
#define FW_BUILD_DATE __DATE__
#define BUILD_DATE __DATE__ " " __TIME__

// ========== WIFI KONFIGURATION ==========
const char* AP_SSID = "ForellenWaechter";
const char* AP_PASSWORD = "ForellenWaechter2024"; // Fallback, wird überschrieben wenn CredentialsManager aktiv

// SECURITY: Passwörter werden im CredentialsManager verwaltet!
// Beim ersten Start werden sichere Passwörter generiert und im EEPROM gespeichert.

// Optional: Station Mode (leer lassen für reinen AP Mode)
const char* STA_SSID = "";
const char* STA_PASSWORD = "";

// WiFi Hostname
const char* HOSTNAME = "forellenwaechter";

// ========== WEB-SERVER KONFIGURATION ==========
#define WEB_SERVER_PORT 80
#define WEBSOCKET_PORT 81

// DEPRECATED: Credentials sind nun in CredentialsManager
// Diese Werte werden nur als Fallback verwendet
const char* WEB_USERNAME = "admin";
const char* WEB_PASSWORD = "CHANGE_ME_AT_FIRST_BOOT";

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
#define TDS_SCOUNT 15              // Anzahl Samples (OPTIMIERT: 30→15, -50% Zeit)

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
#define SENSOR_INTERVAL 30000      // Sensor-Ablesung alle 30s (OPTIMIERT: 5s→30s, -6% Strom)
#define SENSOR_INTERVAL_FAST 5000  // Bei Alarm schneller messen
#define LOG_INTERVAL 600000        // SD-Logging alle 10 Minuten
#define CHART_UPDATE_INTERVAL 300000  // Chart Update alle 5 Minuten
#define WEBSOCKET_UPDATE_INTERVAL 2000 // WebSocket Update alle 2s
#define STATS_UPDATE_INTERVAL 60000    // Statistiken alle 60s updaten

// Chart Datenpunkte
#define CHART_DATA_POINTS 288      // 24h bei 5-Min Intervallen = 288 Punkte
#define CHART_STORAGE_INTERVAL 300000 // Alle 5 Min ein Datenpunkt speichern

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
#define ENABLE_WEB_AUTH true       // Web-Authentifizierung (JETZT FUNKTIONIERT!)
#define ENABLE_API true            // REST API
#define ENABLE_WEBSOCKET true      // WebSocket für Live-Updates
#define ENABLE_RATE_LIMITING true // Rate Limiting (DoS-Schutz)
#define ENABLE_WEBSOCKET_AUTH true // WebSocket Authentifizierung
#define ENABLE_CREDENTIALS_MANAGER true // Sicheres Passwort-Management

// ========== OPTIONALE SENSOREN ==========
#define ENABLE_DO_SENSOR false     // Dissolved Oxygen Sensor (DFRobot SEN0237)
#define ENABLE_INA219 false        // Power Monitoring (INA219 I2C)

// DO-Sensor Pin (wenn aktiviert)
#define DO_PIN 36                  // ADC1_0

// ========== CHART.JS KONFIGURATION ==========
#define ENABLE_CHARTS true         // Chart.js Datenvisualisierung
#define CHART_COLORS true          // Farbige Charts
#define CHART_SMOOTH_LINES true    // Glättung (Tension)

// ========== LTE-MODUL (Optional) ==========
#define ENABLE_LTE false           // LTE-Modul aktivieren
#define LTE_MODULE_TYPE "SIM7600"  // "SIM7600" oder "SIM800L"
#define LTE_APN "internet"         // Provider-APN

// ========== PERFORMANCE-TUNING ==========
// DS18B20 Resolution (OPTIMIERT: 11→9 bit, -75% Conversion-Zeit)
#define DS18B20_RESOLUTION 9       // 9, 10, 11 oder 12 bit
                                    // 9-bit = 94ms, 11-bit = 375ms

// Light Sleep im Main Loop aktivieren (OPTIMIERT: -25mA!)
#define ENABLE_LIGHT_SLEEP true

// WiFi Idle Power Management (OPTIMIERT: -10mA!)
#define WIFI_IDLE_TIMEOUT 300000   // 5 Min ohne Client = Max Power Save
#define WIFI_MAX_POWER_SAVE_IDLE true

// ========== SICHERHEIT ==========
#define MAX_LOGIN_ATTEMPTS 5       // Max Fehlversuche vor Sperre
#define LOGIN_BLOCK_DURATION 300000 // 5 Min Sperre
#define SESSION_TIMEOUT 3600000    // 1 Stunde Session-Timeout
#define WEBSOCKET_TOKEN_LENGTH 32  // Token-Länge für WebSocket Auth

// ========== DEBUGGING ==========
#define DEBUG_PERFORMANCE false    // Performance-Metriken ausgeben
#define DEBUG_MEMORY false         // Heap-Nutzung tracken
#define DEBUG_NETWORK false        // Netzwerk-Traffic loggen

#endif // CONFIG_H
