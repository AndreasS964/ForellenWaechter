# Changelog - ForellenWächter

## [1.6.1] - 2024-12-26

### 🔧 Verbesserungen
- **Interne Pull-ups aktiviert** - Keine externen Widerstände mehr nötig!
  - OneWire Bus (DS18B20): Interner ~45kΩ Pull-up
  - Float Switch: Interner Pull-up
- Dokumentation in config.h verbessert
- Hinweise zu Kabellängen hinzugefügt

### ⚠️ Hinweis
Für DS18B20 Kabellängen >3m weiterhin externen 4.7kΩ Pull-up verwenden!

---

## [1.6.0] - 2024-12-26 - KRITISCHES UPDATE

### 🔴 Kritische Bugfixes
- **HTML ausgelagert** - Behebt RAM-Overflow und Watchdog-Resets
- **Streaming-Ausgabe** - HTML wird in Chunks gesendet, kein großer String mehr
- **PROGMEM für HTML** - CSS/JS/HTML liegt im Flash, nicht RAM
- **Watchdog-Handling** - yield() und esp_task_wdt_reset() an kritischen Stellen

### ✨ Neue Struktur
- `config.h` - Alle Konfiguration zentral
- `html_dashboard.h` - Dashboard HTML/CSS
- `html_dashboard_js.h` - Dashboard JavaScript
- `html_settings.h` - Einstellungsseite
- `ForellenWaechter_v1.6_LTE.ino` - Hauptlogik (nur noch 1224 Zeilen!)

### 🔧 Verbesserungen
- RAM-Verbrauch um ~30% reduziert
- Stabilere WiFi/LTE-Verbindung
- Besseres Error-Handling
- Übersichtlicherer Code

---

## [1.5.0] - 2024-12-01

### ✨ Features
- ⚙️ Settings-Seite mit 3 Tabs (Kalibrierung, Fischarten, Wetter)
- 🐟 Fischarten-Presets (10 Arten)
- 🎛️ 3-Modi Relay-Steuerung (Auto/An/Aus)
- 🌡️ Wetter-Widget mit Live-Daten
- 📡 LTE/4G Mobilfunk
- 📧 E-Mail Alarme
- 🫧 Dissolved Oxygen Sensor Support

### ⚠️ Bekannte Probleme (in v1.6 behoben)
- RAM-Overflow bei großem HTML
- Gelegentliche Watchdog-Resets
- Kompilierungsprobleme mit manchen IDEs

---

## [1.4.1] - 2024-11-30

### ✨ Features
- Telegram Bot Integration
- Boot-Nachricht
- Täglicher Report um 8:00

---

## [1.4.0] - 2024-11-28

### ✨ Features
- LILYGO T-SIM7600E LTE Support
- Remote Dashboard über Mobilfunk
- Verbesserte Charts

---

## [1.3.0] - 2024-11-25

### ✨ Features
- Deep Sleep Modus (später entfernt)
- Batterie-Monitoring (nicht mehr nötig wegen Turbine)

---

## [1.2.0] - 2024-11-22

### ✨ Features
- SD-Karten Datenlogging
- Chart.js Visualisierung
- WebSocket Live-Updates
- Lucas Haug Branding
- REST API
- EEPROM Konfiguration

---

## [1.1.0] - 2024-11-15

### ✨ Features
- Grundlegendes Monitoring
- Alarm-System
- Web-Interface

---

## [1.0.0] - 2024-11-10

### 🎉 Initial Release
- Temperatur, pH, TDS Monitoring
- Float Switch Wasserlevel
- 4-Relay Steuerung
- WiFi Access Point
