# 📁 Projektstruktur - ForellenWächter v2.0

## Übersicht

```
ForellenWaechter/
│
├── 📄 README.md                    # Hauptdokumentation (aktualisiert für v2.0)
├── 📄 README_V2.md                 # Detaillierte v2.0 Dokumentation
├── 📄 LICENSE                      # MIT Lizenz
├── 📄 .gitignore                   # Git Ignorierliste
├── 📄 CHANGELOG.md                 # Versionshistorie (inkl. v2.0)
├── 📄 CHANGELOG_V2.md              # Detailliertes v2.0 Changelog
├── 📄 QUICKSTART.md                # Schnellstart-Anleitung (v2.0)
├── 📄 WIRING.md                    # Verkabelungsanleitung
├── 📄 GITHUB_UPLOAD.md             # GitHub Upload Guide
├── 📄 PROJECT_STRUCTURE.md         # Diese Datei
│
├── 📂 Arduino Projekt (v2.0 - Modular)
│   ├── ForellenWaechter_v2.0.ino  # Hauptprogramm
│   ├── config.h                    # Zentrale Konfiguration
│   ├── power_management.h          # Energie-Management Klasse
│   ├── sensors.h                   # Sensor-Manager Klasse
│   └── webserver.h                 # Web-Server & UI
│
├── 📂 Legacy (v1.2)
│   └── ForellenWaechter_v1.2.ino  # Alte monolithische Version
│
├── 📂 web/                         # Web-Interface Assets
│   ├── index.html                 # Dashboard HTML
│   ├── style.css                  # Styling
│   ├── script.js                  # Frontend JavaScript
│   └── chart-config.js            # Chart.js Konfiguration
│
├── 📂 images/                      # Screenshots & Diagramme
│   ├── dashboard_screenshot.png   # Dashboard Ansicht
│   ├── hardware_setup.jpg         # Hardware Foto
│   ├── wiring_diagram.png         # Verkabelungsdiagram
│   ├── logo.png                   # Lucas Haug Logo
│   └── sensor_placement.jpg       # Sensor-Positionierung
│
├── 📂 docs/                        # Zusätzliche Dokumentation
│   ├── API.md                     # REST API Dokumentation
│   ├── CALIBRATION.md             # Sensor-Kalibrierung
│   ├── TROUBLESHOOTING.md         # Problemlösungen
│   └── HARDWARE_LIST.md           # Teileliste mit Links
│
├── 📂 examples/                    # Beispiel-Sketches
│   ├── sensor_test/               # Sensor einzeln testen
│   │   └── sensor_test.ino
│   ├── wifi_setup/                # WiFi-Konfiguration
│   │   └── wifi_setup.ino
│   ├── relay_test/                # Relay-Test
│   │   └── relay_test.ino
│   └── sd_card_test/              # SD-Karten Test
│       └── sd_card_test.ino
│
├── 📂 data/                        # Beispiel-Daten
│   ├── sample_log.csv             # Beispiel CSV-Log
│   └── calibration_data.json      # Kalibrier-Daten
│
├── 📂 schematics/                  # Hardware-Schaltpläne
│   ├── main_circuit.fzz           # Fritzing Projekt
│   ├── main_circuit.pdf           # PDF Schaltplan
│   └── pcb_layout.png             # PCB Layout (optional)
│
├── 📂 3d_models/                   # 3D-Druck Dateien (optional)
│   ├── enclosure.stl              # Gehäuse
│   └── sensor_mount.stl           # Sensor-Halterung
│
└── 📂 tools/                       # Hilfs-Scripts
    ├── csv_analyzer.py            # Python Daten-Analyse
    ├── firmware_updater.sh        # Update-Script
    └── backup_sd.sh               # SD-Karten Backup
```

---

## Dateibeschreibungen

### Root-Ebene

#### 📄 README.md
**Hauptdokumentation** mit:
- Projekt-Übersicht
- Feature-Liste
- Hardware-Anforderungen
- Installation
- API-Dokumentation
- Roadmap

#### 📄 LICENSE
**MIT Lizenz** - Open Source, freie Verwendung

#### 📄 .gitignore
**Git Ignorierliste** für:
- Build-Dateien
- Sensible Daten
- Temporäre Dateien
- IDE-Konfigurationen

#### 📄 CHANGELOG.md
**Versionshistorie** aller Änderungen:
- v1.2.0: SD-Logging, Charts, Branding
- v1.1.0: Web-Dashboard, Auto-Belüftung
- v1.0.0: Basic Monitoring

#### 📄 QUICKSTART.md
**10-Minuten Schnellstart** für Einsteiger

#### 📄 WIRING.md
**Detaillierte Verkabelung** mit Diagrammen

#### 📄 GITHUB_UPLOAD.md
**Schritt-für-Schritt Anleitung** für GitHub Upload

---

### 📂 Arduino Projekt v2.0 (Modular)

#### ForellenWaechter_v2.0.ino
**Hauptprogramm** mit:
- setup() Funktion - Initialisierung aller Module
- loop() Funktion - Koordination
- WiFi Setup
- OTA Updates Setup
- MQTT Setup (optional)
- Alarm-System
- WebSocket Event Handler

#### config.h
**Zentrale Konfiguration:**
```cpp
// Version & Build
#define FW_VERSION "2.0.0"

// WiFi
const char* AP_SSID = "ForellenWaechter";
const char* STA_SSID = "DeinWLAN";

// Web-Server
const char* WEB_USERNAME = "admin";
const char* WEB_PASSWORD = "...";

// Pins
#define ONE_WIRE_BUS 4
#define PH_PIN 34
#define TDS_PIN 35
#define RELAY_AERATION 26

// Grenzwerte
#define TEMP_MIN 8.0
#define TEMP_MAX 18.0
#define PH_MIN 6.5
#define PH_MAX 8.5

// Energie-Management
#define CPU_FREQUENCY 80
#define WIFI_POWER_SAVE true

// MQTT (optional)
#define MQTT_ENABLED false
const char* MQTT_SERVER = "";

// Features
#define ENABLE_OTA_UPDATE true
#define ENABLE_WEB_AUTH true
```

#### power_management.h
**PowerManager Klasse:**
- CPU-Frequenz-Steuerung
- WiFi Power Save
- Bluetooth-Deaktivierung
- Watchdog Timer
- Batteriespannungs-Monitoring
- Power-Save-Modus
- Energie-Statistiken
```cpp
class PowerManager {
  void begin();
  void feedWatchdog();
  void setWiFiPowerSave(bool);
  float getBatteryVoltage();
  bool isBatteryLow();
  void enablePowerSaveMode();
  void printPowerStats();
};
```

#### sensors.h
**SensorManager Klasse:**
- Sensor-Initialisierung
- Sensor-Auslesungen mit Fehlerbehandlung
- Median-Filter für pH & TDS
- Temperaturkompensation
- 24h Statistiken (Min/Max/Avg)
- Fehler-Tracking
- Kalibrierung
```cpp
struct SensorData {
  float waterTemp, airTemp, pH, tds;
  bool waterLevel;
  unsigned long timestamp;
  bool valid;
};

class SensorManager {
  bool begin();
  bool readAll();
  SensorData getData();
  SensorStats getWaterTempStats();
  void calibratePH(float, float);
  void calibrateTDS(float);
  void printSensorData();
  void printErrorStats();
};
```

#### webserver.h
**Web-Server & UI:**
- Modernes HTML/CSS/JavaScript (eingebettet)
- Dark/Light Mode Support
- Glassmorphism Design
- PWA Manifest
- API-Endpunkte:
  - GET /api/data - Aktuelle Werte
  - GET /api/stats - 24h Statistiken
  - GET /api/download - CSV Download
- WebSocket für Live-Updates
- Login-Authentifizierung (optional)

**Funktionen:**
- handleRoot() - Dashboard HTML
- handleManifest() - PWA Manifest
- handleAPIData() - JSON Sensor-Daten
- handleAPIStats() - JSON Statistiken
- setupWebServer() - Initialisierung

---

### 📂 web/ (Web-Interface)

#### index.html
**Dashboard HTML-Struktur**

#### style.css
**Lucas Haug Branding:**
- Blau/Grün Farbschema
- Responsive Design
- Mobile Optimierung

#### script.js
**Frontend-Logik:**
- WebSocket-Verbindung
- Live-Updates
- API-Calls

#### chart-config.js
**Chart.js Konfiguration:**
- 24h Historien-Charts
- Temperatur-Diagramme
- pH/TDS-Visualisierung

---

### 📂 images/

#### dashboard_screenshot.png
![Dashboard](Beispiel-Screenshot des Dashboards)

#### hardware_setup.jpg
![Hardware](Foto der verkabelten Hardware)

#### wiring_diagram.png
![Wiring](Verkabelungsdiagramm)

#### logo.png
**Lucas Haug Logo** für Branding

#### sensor_placement.jpg
**Foto** der Sensor-Positionierung am Teich

---

### 📂 docs/ (Erweiterte Dokumentation)

#### API.md
**REST API Dokumentation:**
```
GET  /api/data
GET  /api/history
POST /api/config
```

#### CALIBRATION.md
**Sensor-Kalibrierung:**
- pH-Sensor Kalibrierung (pH 4.0 & 7.0)
- TDS-Sensor Kalibrierung (1413 µS/cm)
- Temperatur-Sensor Offset

#### TROUBLESHOOTING.md
**Problemlösungen:**
- WiFi verbindet nicht
- Sensoren zeigen falsche Werte
- SD-Karte nicht erkannt

#### HARDWARE_LIST.md
**Detaillierte Teileliste:**
- Amazon-Links
- Alternative Produkte
- Preis-Übersicht

---

### 📂 examples/ (Beispiel-Sketches)

#### sensor_test.ino
Jeder Sensor einzeln testen

#### wifi_setup.ino
WiFi-Verbindung testen

#### relay_test.ino
Relays durchschalten

#### sd_card_test.ino
SD-Karte lesen/schreiben

---

### 📂 data/

#### sample_log.csv
```csv
timestamp,water_temp,air_temp,ph,tds
2024-01-15 14:30:00,10.5,12.3,7.2,245
```

#### calibration_data.json
```json
{
  "ph": {
    "offset": 0.15,
    "slope": 1.02
  }
}
```

---

### 📂 schematics/

#### main_circuit.fzz
**Fritzing Projekt-Datei** zum Bearbeiten

#### main_circuit.pdf
**PDF-Export** für Druck

#### pcb_layout.png
**PCB Design** (falls eigene Platine erstellt wird)

---

### 📂 3d_models/ (Optional)

#### enclosure.stl
**3D-Druck Gehäuse** für Elektronik

#### sensor_mount.stl
**Sensor-Halterung** für Teich-Montage

---

### 📂 tools/

#### csv_analyzer.py
**Python Script** für Datenanalyse:
```python
import pandas as pd
df = pd.read_csv('data.csv')
# Analyse...
```

#### firmware_updater.sh
**Bash Script** für OTA-Updates:
```bash
#!/bin/bash
esptool.py --port /dev/ttyUSB0 write_flash 0x0 firmware.bin
```

#### backup_sd.sh
**SD-Karten Backup:**
```bash
#!/bin/bash
rsync -av /media/sd_card/ ./backups/
```

---

## Empfohlene Ordnerstruktur für Entwicklung

```
Arbeitsbereich/
│
├── ForellenWaechter/           # Git Repository
│   ├── ForellenWaechter_v2.0.ino
│   ├── config.h
│   ├── power_management.h
│   ├── sensors.h
│   ├── webserver.h
│   ├── ForellenWaechter_v1.2.ino (Legacy)
│   ├── README.md
│   ├── README_V2.md
│   ├── CHANGELOG.md
│   ├── CHANGELOG_V2.md
│   ├── QUICKSTART.md
│   ├── PROJECT_STRUCTURE.md
│   ├── WIRING.md
│   ├── GITHUB_UPLOAD.md
│   └── LICENSE
│
├── hardware_tests/              # Temporäre Test-Sketches
│   ├── sensor_test.ino
│   └── power_test.ino
│
├── backups/                     # SD-Karten Backups
│   ├── 2024-11-20/
│   ├── 2024-11-22/
│   └── 2024-11-23/
│
└── datasheets/                  # Sensor-Datenblätter
    ├── DS18B20.pdf
    ├── pH_sensor.pdf
    ├── TDS_meter.pdf
    └── ESP32_datasheet.pdf
```

## v2.0 Architektur-Übersicht

```
┌─────────────────────────────────────────────────────────┐
│         ForellenWaechter_v2.0.ino (Main)                │
│   - setup(): Initialisiert alle Module                  │
│   - loop(): Koordiniert Sensoren, WebSocket, MQTT       │
└─────────────────────────────────────────────────────────┘
           │          │          │          │
           ▼          ▼          ▼          ▼
    ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐
    │ config.h │ │  power_  │ │ sensors  │ │webserver │
    │          │ │management│ │    .h    │ │   .h     │
    └──────────┘ └──────────┘ └──────────┘ └──────────┘
         ▲            ▲            ▲            ▲
         │            │            │            │
         │            │            │            │
    ┌────┴────────────┴────────────┴────────────┴────┐
    │        Zentrale Konfiguration & Module         │
    │  - WiFi, MQTT, OTA Settings                    │
    │  - PowerManager: CPU, WiFi, Watchdog           │
    │  - SensorManager: Alle Sensoren + Stats        │
    │  - WebServer: HTML UI, API, WebSocket          │
    └────────────────────────────────────────────────┘
```

## Code-Abhängigkeiten

```
ForellenWaechter_v2.0.ino
├── Verwendet: config.h (Konfiguration)
├── Verwendet: power_management.h (PowerManager)
├── Verwendet: sensors.h (SensorManager)
├── Verwendet: webserver.h (Web-Server Setup)
└── Bibliotheken:
    ├── WiFi.h
    ├── ESPmDNS.h
    ├── ArduinoOTA.h
    ├── SD.h
    ├── time.h
    └── PubSubClient.h (optional)

power_management.h
├── Verwendet: config.h
└── Bibliotheken:
    ├── esp_wifi.h
    ├── esp_bt.h
    └── esp_sleep.h

sensors.h
├── Verwendet: config.h
└── Bibliotheken:
    ├── OneWire.h
    └── DallasTemperature.h

webserver.h
├── Verwendet: config.h
├── Verwendet: sensors.h (SensorManager Referenz)
└── Bibliotheken:
    ├── WebServer.h
    └── WebSocketsServer.h
```

---

## Git Repository Größe

**Empfohlene Limits:**
- Einzelne Datei: < 10 MB
- Repository gesamt: < 100 MB
- Binärdateien vermeiden (PDFs, große Bilder)

**Große Dateien:**
- GitHub LFS für >50 MB
- Oder externe Links verwenden

---

## Dateien NICHT in Git aufnehmen

❌ `.gitignore` schließt aus:
- Build-Dateien (`*.bin`, `*.elf`)
- IDE-Konfigurationen (`.vscode/`)
- Sensible Daten (`secrets.h`, `passwords.txt`)
- Temporäre Dateien (`*.tmp`, `*.bak`)
- SD-Karten Logs (`data/*.csv`)

---

## Nächste Schritte

1. ✅ Grundstruktur erstellen
2. ✅ Code organisieren
3. ✅ Dokumentation schreiben
4. ✅ Screenshots hinzufügen
5. ✅ Auf GitHub pushen

---

**Gut organisiert = Gut entwickelt! 📂✨**
