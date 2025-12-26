/*
 * ═══════════════════════════════════════════════════════════════════════════════════
 * config.h - ForellenWächter v1.6 Konfiguration
 * ═══════════════════════════════════════════════════════════════════════════════════
 * 
 * Alle Einstellungen zentral an einem Ort.
 * WICHTIG: Vor dem Upload anpassen!
 */

#ifndef CONFIG_H
#define CONFIG_H

// ═══════════════════════════════════════════════════════════════════════════════════
// BETRIEBSMODUS
// ═══════════════════════════════════════════════════════════════════════════════════

#define TEST_MODE false              // true = Fake-Werte für Tests
#define DEBUG_MODE true              // true = Serial-Ausgabe aktiv
#define WATCHDOG_TIMEOUT 120         // Sekunden bis Watchdog-Reset

// ═══════════════════════════════════════════════════════════════════════════════════
// FEATURE TOGGLES
// ═══════════════════════════════════════════════════════════════════════════════════

#define ENABLE_LTE true              // LTE Mobilfunk
#define ENABLE_WIFI true             // WiFi (parallel zu LTE)
#define ENABLE_EMAIL_ALERTS true     // E-Mail Benachrichtigungen
#define ENABLE_DO_SENSOR false       // Dissolved Oxygen Sensor
#define ENABLE_SD_LOGGING true       // SD-Karten Logging
#define ENABLE_OTA true              // Over-The-Air Updates

// ═══════════════════════════════════════════════════════════════════════════════════
// WIFI KONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════════════

#define AP_SSID "ForellenWaechter"
#define AP_PASSWORD "YourPassword123"     // ⚠️ ÄNDERN!
#define STA_SSID ""                       // Optional: Heimnetz SSID
#define STA_PASSWORD ""                   // Optional: Heimnetz Passwort
#define MDNS_NAME "forellenwaechter"
#define OTA_PASSWORD "forellenadmin123"   // ⚠️ ÄNDERN!

// ═══════════════════════════════════════════════════════════════════════════════════
// LTE KONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════════════

#define LTE_APN "internet"           // APN deines Providers
#define LTE_USER ""                  // APN Username (meist leer)
#define LTE_PASS ""                  // APN Passwort (meist leer)

// ═══════════════════════════════════════════════════════════════════════════════════
// E-MAIL / WEBHOOK KONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════════════

#define EMAIL_WEBHOOK_URL "https://maker.ifttt.com/trigger/forellen_alarm/with/key/YOUR_IFTTT_KEY"
#define EMAIL_RECIPIENT "your-email@example.com"  // ⚠️ ÄNDERN!
#define EMAIL_COOLDOWN_MIN 30        // Min. Minuten zwischen E-Mails

// ═══════════════════════════════════════════════════════════════════════════════════
// PIN DEFINITIONEN
// ═══════════════════════════════════════════════════════════════════════════════════

// Temperatursensoren (OneWire Bus)
// HINWEIS: Interner Pull-up wird aktiviert (~45kΩ)
// Für Kabellängen >3m externen 4.7kΩ Pull-up verwenden!
#define ONE_WIRE_BUS 4               // DS18B20 Data Pin

// Analog Sensoren (keine Pull-ups nötig)
#define PH_PIN 34
#define TDS_PIN 35
#define DO_PIN 36                    // Dissolved Oxygen

// Digital I/O
// HINWEIS: Interner Pull-up wird aktiviert - KEIN externer Widerstand nötig!
#define WATER_LEVEL_PIN 39           // Float Switch (intern pulled up)
#define STATUS_LED 23                // Status LED
#define ALARM_LED 13                 // Externe Alarm-LED (rot)
#define BUZZER_PIN 15

// Relais (Active HIGH nach Korrektur)
#define RELAY_1 32                   // GPIO32 - Alarm
#define RELAY_2 33                   // GPIO33 - Reserve 1
#define RELAY_3 25                   // GPIO25 - Reserve 2
#define RELAY_4 26                   // GPIO26 - Belüftung

// SD-Karte (SPI)
#define SD_CS 5

// LTE Modul (SIM7600)
#define LTE_TX 17
#define LTE_RX 16
#define LTE_PWR 25                   // Power Key (optional)

// ═══════════════════════════════════════════════════════════════════════════════════
// ZEITINTERVALLE (Millisekunden)
// ═══════════════════════════════════════════════════════════════════════════════════

#define SENSOR_INTERVAL 5000         // Sensor-Abfrage: 5 Sekunden
#define LOG_INTERVAL 300000          // SD-Logging: 5 Minuten
#define LTE_CHECK_INTERVAL 60000     // LTE Status: 1 Minute
#define HISTORY_INTERVAL 300000      // Chart-History: 5 Minuten
#define NTP_SYNC_INTERVAL 3600000    // Zeit-Sync: 1 Stunde

// ═══════════════════════════════════════════════════════════════════════════════════
// SENSOR GRENZWERTE (Regenbogenforelle Standard)
// ═══════════════════════════════════════════════════════════════════════════════════

#define DEFAULT_TEMP_MIN 8.0
#define DEFAULT_TEMP_MAX 14.0
#define DEFAULT_TEMP_CRITICAL 16.0
#define DEFAULT_PH_MIN 6.5
#define DEFAULT_PH_MAX 8.5
#define DEFAULT_TDS_MAX 500
#define DEFAULT_DO_MIN 6.0
#define DEFAULT_DO_OPTIMAL 9.0

// ═══════════════════════════════════════════════════════════════════════════════════
// EEPROM
// ═══════════════════════════════════════════════════════════════════════════════════

#define EEPROM_SIZE 256
#define EEPROM_MAGIC 0xF156          // Magic Number für v1.6

// ═══════════════════════════════════════════════════════════════════════════════════
// HISTORIE
// ═══════════════════════════════════════════════════════════════════════════════════

#define HISTORY_SIZE 288             // 24h bei 5min Intervall

// ═══════════════════════════════════════════════════════════════════════════════════
// VERSION
// ═══════════════════════════════════════════════════════════════════════════════════

#define FIRMWARE_VERSION "1.6.1"

#endif // CONFIG_H
