# 📜 Changelog

Alle Änderungen am ForellenWächter Projekt.

---

## [1.6.0] - 2024-12-03

### ⚡ Turbine & Power Edition - Autarke Stromversorgung!

**Großes Energie-Update: Wasserkraft-Turbine & Batterie-Monitoring**

#### Hinzugefügt

- **Wasserturbinen-Integration**
  - Mini Hydro-Generator Support (12V, 10W)
  - Flow-Sensor: Hall-Sensor Impulsmessung via GPIO 2
  - Interrupt-basierte Pulse-Zählung (IRAM_ATTR ISR)
  - Durchfluss-Berechnung in L/min (Echtzeit)
  - Kalibrierbar via `TURBINE_PULSES_PER_LITER`

- **Power-Monitoring**
  - Turbinen-Leistung in Watt
  - Berechnung basierend auf Durchfluss
  - API: `/api/sensors` erweitert um `turbinePower`, `flowRate`
  - Serial Monitor Ausgabe: Flow + Power

- **Batterie-Monitoring**
  - Echtzeit Spannungsmessung (Pb/Gel-Batterien)
  - Spannungsteiler Support (10kΩ + 3.3kΩ) auf GPIO 36
  - Prozent-Anzeige (10.5V-13.8V Bereich)
  - Low-Battery Warnung bei < 11.5V
  - API: `batteryVoltage`, `batteryPercent`, `batteryLow`

- **Alarm-Erweiterungen**
  - Flow-Alarm: Warnung bei Durchfluss < 5.0 L/min
  - Batterie-Alarm: Warnung bei niedriger Batterie
  - Beide Alarme in `checkAlarms()` integriert

- **Konfiguration**
  - `ENABLE_TURBINE` - Feature-Toggle
  - `ENABLE_BATTERY_MONITOR` - Feature-Toggle
  - Turbinen-Parameter: Spannung, Max-Power, Min-Flow
  - Batterie-Parameter: R1, R2, FULL, EMPTY, WARNING
  - `WEATHER_UPDATE_INTERVAL` - 12h (2x täglich)

- **Dokumentation**
  - HARDWARE.md: Kompletter Abschnitt "NEU in v1.6"
  - Schaltplan Stromversorgung v1.6
  - Verdrahtungsanleitungen für Turbine + Laderegler
  - Spannungsteiler-Berechnung
  - Kalibrierungsanleitungen (Flow + Batterie)
  - README.md: v1.6 Features und Badge
  - config.example.h: Turbine & Batterie Konfiguration

#### Geändert

- **Pin-Belegung**
  - GPIO 2: Flow-Sensor (Turbinen Hall-Sensor)
  - GPIO 36: Batterie-Spannung (Spannungsteiler)
  - GPIO 27: DO-Sensor (verschoben von GPIO 36)

- **API**
  - `handleAPISensors()`: JSON-Dokument auf 768 Bytes erweitert
  - Turbinen- und Batterie-Daten in API integriert

- **Test-Mode**
  - `generateTestData()`: Turbine & Batterie Fake-Daten
  - Sinuswellen für Flow (0.9-1.5 L/min) und Power (5-10W)
  - Batterie-Simulation (12.0-13.0V, 60-90%)

- **Serial Output**
  - `printSensorValues()`: Flow, Power, Batterie angezeigt
  - Formatierung mit Emojis (⚡🔋💧)

#### Technische Details

- **Stromversorgung v1.6:**
  ```
  Turbine (12V, 10W)
    → LM2596 Laderegler (13.8V)
    → Pb/Gel-Batterie (12V, 7-20Ah)
    → Step-Down (5V)
    → ESP32
  ```

- **Hardware-Kosten:**
  - Turbine: ~15€
  - Laderegler: ~8€
  - Batterie: 20-50€
  - **Gesamt: ca. 50€ für autarke Stromversorgung!**

- **Energiebilanz:**
  - Turbinen-Output: 10W
  - ESP32 + Sensoren: ~1.5W
  - Überschuss: 8.5W → lädt Batterie
  - **Vollständig autark bei kontinuierlichem Durchfluss!**

---

## [1.5.1] - 2024-12-01

### 🔧 GPIO-Korrektur

#### Behoben
- **GPIO-Pins nach Hersteller-Spezifikation korrigiert:**
  - STATUS_LED: GPIO 2 → GPIO 23
  - RELAY_1: GPIO 32 (Alarm) ✓
  - RELAY_2: GPIO 33 (Reserve 1) ✓
  - RELAY_3: GPIO 23 → GPIO 25 (Reserve 2)
  - RELAY_4: GPIO 26 (Belüftung) ✓
  - DO_PIN: GPIO 32 → GPIO 36 (Konflikt mit Relay 1 behoben)
  - WATER_LEVEL_PIN: GPIO 33 → GPIO 39 (Konflikt mit Relay 2 behoben)

---

## [1.5.0] - 2024-12-01

### 🆕 Settings-Seite & erweiterte Features

**Großes UI-Update mit Settings-Seite!**

#### Hinzugefügt
- **Settings-Seite (/settings)**
  - Neuer Settings-Button (⚙️) im Dashboard-Header
  - Modern design mit Tab-Navigation
  - 3 Hauptbereiche:
    1. **Kalibrierung:** pH 2-Punkt & TDS 1-Punkt UI
    2. **Fischarten:** Preset-Auswahl mit automatischen Grenzwerten
    3. **Wetter:** PLZ-Konfiguration für Wettervorhersage

- **Fischarten-Voreinstellungen**
  - 4 vordefinierte Fischarten:
    - Forelle (8-16°C, pH 6.5-8.5)
    - Regenbogenforelle (10-20°C, pH 6.5-8.0)
    - Karpfen (15-28°C, pH 6.5-9.0)
    - Tilapia (20-32°C, pH 6.5-9.0)
  - Benutzerdefinierte Einstellungen möglich
  - Automatisches Laden der Grenzwerte

- **Wetter-Widget**
  - Neue Wetter-Karte im Dashboard
  - 24h Vorhersage über wttr.in
  - PLZ-basierte Wetterinfo (Deutschland)
  - localStorage für PLZ-Speicherung

- **3-Modi Relay-Steuerung**
  - Alle 4 Relays steuerbar: Auto/An/Aus
  - Toggle-Funktion durch Klick
  - Visuelles Feedback:
    - Auto: Blauer Hintergrund
    - An: Grüner Hintergrund
    - Aus: Ausgegraut
  - Mode-Labels unter jedem Button

- **TDS im Wasserqualitäts-Chart**
  - 3 Y-Achsen: pH, O₂, TDS
  - Bessere Darstellung der Wasserqualität

#### Geändert
- Dashboard-Header: Flexbox-Layout mit Settings-Button
- Relay-Buttons mit Mode-Anzeige
- Footer: "© 2024 Andreas Sika"
- Version auf v1.5.0 aktualisiert
- API: POST /api/relay ohne state-Parameter (toggle)
- Charts: TDS-Linie hinzugefügt

#### Behoben
- Chart-Overflow Bug: Charts mit fester Höhe (300px)
- Watchdog-Resets in setup() und loop()
- getLocalTime() mit Timeouts versehen
- Browser-Crashes durch überlaufende Charts behoben

---

## [1.4.1] - 2024-12-01

### 🔧 OTA & Kalibrierung

#### Hinzugefügt
- **OTA (Over-The-Air) Updates**
  - ArduinoOTA Support
  - Passwort-geschützt
  - Netzwerk-Port in Arduino IDE
  - Vollständige Dokumentation in docs/OTA_GUIDE.md

- **Sensor-Kalibrierung**
  - pH: 2-Punkt Kalibrierung (4.0 & 7.0)
  - TDS: 1-Punkt Kalibrierung (707 ppm Standard)
  - EEPROM-Speicherung mit Checksumme
  - API-Endpoints:
    - GET /api/calibration
    - POST /api/calibration/ph
    - POST /api/calibration/tds
    - POST /api/calibration/reset
  - Dokumentation in docs/CALIBRATION.md

#### Behoben
- Compilation Error: Forward Declarations hinzugefügt

---

## [1.4.0] - 2024-12-01

### 🆕 LTE Remote Edition

**Vollständiger Remote-Zugriff über Mobilfunk!**

#### Hinzugefügt
- **LTE/4G Mobilfunk (SIM7600)**
  - Remote-Dashboard von überall
  - Automatische Verbindungsprüfung
  - Signal-Stärke Anzeige
  - Operator-Erkennung
  
- **E-Mail Alarme**
  - Webhook-basiert (IFTTT kompatibel)
  - Konfigurierbare Cooldown-Zeit
  - Detaillierte Alarmnachricht mit allen Werten
  
- **Dissolved Oxygen Sensor (optional)**
  - DFRobot Gravity DO-Sensor Support
  - Temperaturkompensation
  - Kalibrierungsanleitung

- **Neues Dashboard**
  - Glassmorphism-Design
  - Animierter Fisch-Logo
  - Status-Pills mit Live-Anzeige
  - LTE/WiFi Signal-Anzeige
  - Alarm-Banner
  
- **Erweiterte API**
  - POST /api/settings für Grenzwerte
  - POST /api/test-email
  - Mehr Status-Informationen

- **NTP Zeitsynchronisation**
  - Echtzeit-Uhr
  - Timestamps in Logs

#### Geändert
- Dashboard komplett überarbeitet
- Bessere Code-Struktur
- Erweiterte Datenstrukturen

#### Hardware
- SIM7600E LTE-Modul
- DFRobot Gravity DO (optional)
- Alarm-LED separat (GPIO 13)
- Buzzer (GPIO 15)

---

## [1.3.0] - 2024-12-01

### 🆕 Neu: Deep Sleep Edition

**Für Batteriebetrieb optimiert!**

#### Hinzugefügt
- **Deep Sleep Mode**
  - ESP32 schläft zwischen Messungen
  - Nur ~10µA im Sleep
  - ~0.4mA Durchschnitt bei 10 Min Intervall
  
- **Battery Monitoring**
  - Spannungsmessung per Spannungsteiler
  - Prozent-Anzeige (0-100%)
  - Low Battery Warning
  - Emergency Sleep bei kritischem Level

- **RTC Memory**
  - Boot-Counter überlebt Deep Sleep
  - Alarm-Counter persistent
  - Letzte Messwerte gespeichert

- **Buzzer/Summer Support**
  - Akustischer Alarm
  - 3x Piep bei Alarmen

- **Event-Logging**
  - Separate events.log Datei
  - Alarme werden protokolliert

#### Geändert
- Kein WiFi im Deep Sleep Mode (zu stromhungrig)
- Vereinfachtes Logging (CSV ohne WebSocket)
- Angepasste Watchdog-Konfiguration

#### Entfernt
- WebSocket (nur in v1.2)
- Web-Dashboard (nur in v1.2)
- Chart-Historie (nur in v1.2)

#### Stromverbrauch
```
v1.2 (24/7 WiFi): ~320mA → 4 Tage auf 30Ah
v1.3 (Deep Sleep): ~0.4mA → 8 MONATE auf 30Ah! 🎉
```

---

## [1.2.0] - 2024-11-21

### 🆕 Full Edition mit Charts & Logging

**Komplettes Web-Dashboard mit Visualisierung!**

#### Hinzugefügt
- **Chart.js Visualisierung**
  - Temperatur-Verlauf
  - pH & TDS Verlauf
  - 24h / 7d / 30d Ansichten

- **SD-Karten Datenlogging**
  - CSV-Format für Excel
  - Tägliche Log-Dateien
  - Automatische Rotation

- **WebSocket Live-Updates**
  - Echtzeit alle 2 Sekunden
  - Kein Seiten-Reload nötig

- **Ring-Buffer Historie**
  - 288 Datenpunkte (24h)
  - 5-Minuten Intervall

- **REST API erweitert**
  - /api/history
  - /api/settings
  - /api/relay

- **Professionelles Branding**
  - Blau/Grün Farbschema
  - Professionelles Design

#### Verbessert
- Responsive Dashboard (Mobile optimiert)
- Status-Karten mit Farbcodierung
- Alarm-Animation

---

## [1.1.0] - 2024-11-15

### 🆕 Web-Dashboard

#### Hinzugefügt
- **Web-Dashboard**
  - Live-Anzeige aller Sensoren
  - Mobil-optimiert

- **mDNS Support**
  - Zugriff via http://forellenwaechter.local

- **Automatische Belüftung**
  - Aktiviert bei Grenzwertüberschreitung

- **Status LED**
  - Blinkt schnell bei Alarm

#### Verbessert
- WiFi AP-Fallback
- Stabilere Sensor-Abfrage

---

## [1.0.0] - 2024-11-10

### 🎉 Erste Version

#### Features
- **Temperatur-Monitoring**
  - 2x DS18B20 (Wasser + Luft)
  - 12-Bit Auflösung

- **Wasserqualität**
  - pH-Sensor (analog)
  - TDS-Sensor (analog)

- **Wasserlevel**
  - Float Switch

- **Relais-Steuerung**
  - 4-Kanal Modul
  - Belüftungspumpe

- **Software Watchdog**
  - Auto-Reset bei Hänger
  - 60 Sekunden Timeout

- **Serial Debug Output**
  - Alle Messwerte
  - Systemstatus

#### Hardware
- ESP32 DevKit v1
- Stromversorgung: USB oder 5V

---

## Geplante Features

### v1.4 (geplant)
- [ ] GSM/SMS Alarme (SIM7600)
- [ ] Solar-Laderegler Integration
- [ ] Multi-Becken Support

### v1.5 (geplant)
- [ ] Wetter-API Integration
- [ ] Fütterungszeit-Tracker
- [ ] Cloud-Backup (optional)

### v2.0 (Vision)
- [ ] Multi-ESP32 Mesh-Netzwerk
- [ ] MQTT Support
- [ ] Grafana Dashboard

---

## Migration

### Von v1.1 auf v1.2

1. Neue Bibliotheken installieren (keine neuen nötig)
2. SD-Karten Modul anschließen:
   ```
   CS   → GPIO5
   MOSI → GPIO23
   MISO → GPIO19
   SCK  → GPIO18
   ```
3. Code neu hochladen
4. SD-Karte mit FAT32 formatieren

### Von v1.2 auf v1.3

⚠️ **v1.3 ist für Batteriebetrieb!**

Für Netzbetrieb weiterhin v1.2 nutzen.

Bei Wechsel auf v1.3:
1. Buzzer anschließen (GPIO27)
2. Batterie-Spannungsteiler (GPIO36)
3. Code anpassen:
   ```cpp
   #define DEEP_SLEEP_ENABLED true
   ```

---

## Bekannte Bugs

### v1.2
- Charts können bei >24h Uptime verzögern
- mDNS funktioniert nicht auf allen Android-Geräten

### v1.3
- Keine bekannten Bugs

---

## Kompatibilität

| Version | ESP32 Core | Arduino IDE |
|---------|------------|-------------|
| 1.3.x | 3.x | 2.x |
| 1.2.x | 2.x, 3.x | 1.8.x, 2.x |
| 1.1.x | 2.x | 1.8.x |
| 1.0.x | 2.x | 1.8.x |

---

*Datum-Format: YYYY-MM-DD*
