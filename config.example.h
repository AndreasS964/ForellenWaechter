/*
 * ═══════════════════════════════════════════════════════════════════════════════════
 * ForellenWächter - Konfigurationsvorlage
 * ═══════════════════════════════════════════════════════════════════════════════════
 *
 * ANLEITUNG:
 * 1. Diese Datei kopieren und als "config.h" speichern
 * 2. Alle Werte mit IHREN Daten ausfüllen
 * 3. config.h wird von .gitignore ignoriert (bleibt privat!)
 *
 * ═══════════════════════════════════════════════════════════════════════════════════
 */

#ifndef CONFIG_H
#define CONFIG_H

// ═══════════════════════════════════════════════════════════════════════════════════
// WiFi KONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════════════

// Access Point (ESP32 erstellt eigenes WLAN)
#define WIFI_AP_SSID "ForellenWaechter"
#define WIFI_AP_PASSWORD "IhrSicheresPasswort123!"  // Min. 8 Zeichen!

// Station Mode (optional - zum Verbinden mit bestehendem WLAN)
#define WIFI_STA_SSID ""                  // Leer lassen für reinen AP-Modus
#define WIFI_STA_PASSWORD ""

// mDNS Hostname
#define MDNS_HOSTNAME "forellenwaechter"  // Zugriff via http://forellenwaechter.local

// ═══════════════════════════════════════════════════════════════════════════════════
// LTE KONFIGURATION (nur v1.4)
// ═══════════════════════════════════════════════════════════════════════════════════

// APN Ihres Mobilfunkanbieters
#define LTE_APN "internet"                // z.B. "internet", "web.vodafone.de", etc.
#define LTE_USER ""                       // Meist leer
#define LTE_PASS ""                       // Meist leer

// ═══════════════════════════════════════════════════════════════════════════════════
// E-MAIL ALARME (nur v1.4)
// ═══════════════════════════════════════════════════════════════════════════════════

// Webhook-URL für E-Mail-Dienst (IFTTT, EmailJS, etc.)
#define EMAIL_WEBHOOK_URL "https://maker.ifttt.com/trigger/forellen_alarm/with/key/IHR_IFTTT_KEY"

// Empfänger-E-Mail
#define EMAIL_RECIPIENT "ihre-email@example.com"

// Cooldown zwischen E-Mails (Minuten)
#define EMAIL_COOLDOWN_MIN 30

// ═══════════════════════════════════════════════════════════════════════════════════
// SENSOR GRENZWERTE (an Ihre Fischart anpassen!)
// ═══════════════════════════════════════════════════════════════════════════════════

// Temperatur (°C)
#define TEMP_MIN 8.0
#define TEMP_MAX 14.0
#define TEMP_CRITICAL 16.0

// pH-Wert
#define PH_MIN 6.5
#define PH_MAX 8.5

// TDS (Total Dissolved Solids) in ppm
#define TDS_MAX 500

// Gelöster Sauerstoff (mg/L) - nur wenn DO-Sensor vorhanden
#define DO_MIN 6.0
#define DO_OPTIMAL 9.0

// ═══════════════════════════════════════════════════════════════════════════════════
// TURBINEN-KONFIGURATION (v1.6)
// ═══════════════════════════════════════════════════════════════════════════════════

// Hall-Sensor Kalibrierung
#define TURBINE_PULSES_PER_LITER 450  // Impulse pro Liter (experimentell bestimmen!)
#define TURBINE_VOLTAGE 12.0          // Nennspannung (V)
#define TURBINE_MAX_POWER 10.0        // Maximale Leistung (W)

// Durchfluss-Alarm
#define FLOW_MIN_ALARM 5.0            // Min. Durchfluss L/min (Alarm wenn unterschritten)

// ═══════════════════════════════════════════════════════════════════════════════════
// BATTERIE-KONFIGURATION (v1.6)
// ═══════════════════════════════════════════════════════════════════════════════════

// Spannungsteiler (R1 = oberer Widerstand, R2 = unterer Widerstand)
#define BATTERY_R1 10000.0            // 10kΩ
#define BATTERY_R2 3300.0             // 3.3kΩ

// Spannungsgrenzen für Blei/Gel-Batterien
#define BATTERY_FULL 13.8             // 100% Ladespannung (V)
#define BATTERY_EMPTY 10.5            // 0% Tiefentladung (V)
#define BATTERY_WARNING 11.5          // Warnschwelle (V)

// ═══════════════════════════════════════════════════════════════════════════════════
// ZEITINTERVALLE (Millisekunden)
// ═══════════════════════════════════════════════════════════════════════════════════

#define SENSOR_INTERVAL 5000          // Sensor-Abfrage (5 Sekunden)
#define LOG_INTERVAL 300000           // SD-Logging (5 Minuten)
#define LTE_CHECK_INTERVAL 60000      // LTE Status Check (1 Minute)
#define HISTORY_INTERVAL 300000       // Chart-History Update (5 Minuten)
#define WEATHER_UPDATE_INTERVAL 43200000  // Wetter-Update (12h = 2x täglich)

// ═══════════════════════════════════════════════════════════════════════════════════
// ERWEITERTE EINSTELLUNGEN
// ═══════════════════════════════════════════════════════════════════════════════════

// Watchdog Timeout (Sekunden)
#define WATCHDOG_TIMEOUT 120

// Deep Sleep (nur v1.3)
#define SLEEP_DURATION_SEC 600        // 10 Minuten

// ═══════════════════════════════════════════════════════════════════════════════════
// TELEGRAM BOT (v1.6.1) - Optional
// ═══════════════════════════════════════════════════════════════════════════════════

// Siehe: docs/TELEGRAM_SETUP.md für Anleitung
const char* TELEGRAM_BOT_TOKEN = "YOUR_BOT_TOKEN";  // Von @BotFather
const char* TELEGRAM_CHAT_ID = "YOUR_CHAT_ID";      // Von @userinfobot
#define TELEGRAM_CHECK_INTERVAL 2000  // Nachrichten-Check (ms)

// ═══════════════════════════════════════════════════════════════════════════════════
// DYNDNS (v1.6.1) - Optional
// ═══════════════════════════════════════════════════════════════════════════════════

// Siehe: docs/DYNDNS_SETUP.md für Anleitung
const char* DYNDNS_DOMAIN = "forellenwaechter.duckdns.org";  // Deine DuckDNS Domain
const char* DYNDNS_TOKEN = "YOUR_DUCKDNS_TOKEN";             // Token von duckdns.org
#define DYNDNS_UPDATE_INTERVAL 300000  // Update alle 5 Minuten

// ═══════════════════════════════════════════════════════════════════════════════════
// FEATURE TOGGLES
// ═══════════════════════════════════════════════════════════════════════════════════

#define ENABLE_LTE true               // LTE aktivieren (v1.4+)
#define ENABLE_WIFI true              // WiFi aktivieren
#define ENABLE_EMAIL_ALERTS true      // E-Mail Alarme aktivieren
#define ENABLE_DO_SENSOR false        // Dissolved Oxygen Sensor
#define ENABLE_SD_LOGGING true        // SD-Karte aktivieren
#define ENABLE_TURBINE true           // Wasserturbine Flow & Power Monitoring (v1.6)
#define ENABLE_BATTERY_MONITOR true   // Batterie-Überwachung (v1.6)
#define ENABLE_TELEGRAM false         // Telegram Bot (v1.6.1)
#define ENABLE_DYNDNS false           // DynDNS Auto-Update (v1.6.1)

// Test-Modus (Fake-Sensordaten)
#define TEST_MODE false               // true = Test-Werte, false = echte Sensoren

#endif // CONFIG_H
