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
// ZEITINTERVALLE (Millisekunden)
// ═══════════════════════════════════════════════════════════════════════════════════

#define SENSOR_INTERVAL 5000          // Sensor-Abfrage (5 Sekunden)
#define LOG_INTERVAL 300000           // SD-Logging (5 Minuten)
#define LTE_CHECK_INTERVAL 60000      // LTE Status Check (1 Minute)
#define HISTORY_INTERVAL 300000       // Chart-History Update (5 Minuten)

// ═══════════════════════════════════════════════════════════════════════════════════
// ERWEITERTE EINSTELLUNGEN
// ═══════════════════════════════════════════════════════════════════════════════════

// Watchdog Timeout (Sekunden)
#define WATCHDOG_TIMEOUT 120

// Deep Sleep (nur v1.3)
#define SLEEP_DURATION_SEC 600        // 10 Minuten

// Feature Toggles
#define ENABLE_LTE true               // LTE aktivieren (v1.4)
#define ENABLE_WIFI true              // WiFi aktivieren
#define ENABLE_EMAIL_ALERTS true      // E-Mail Alarme aktivieren
#define ENABLE_DO_SENSOR false        // Dissolved Oxygen Sensor
#define ENABLE_SD_LOGGING true        // SD-Karte aktivieren

// Test-Modus (Fake-Sensordaten)
#define TEST_MODE false               // true = Test-Werte, false = echte Sensoren

#endif // CONFIG_H
