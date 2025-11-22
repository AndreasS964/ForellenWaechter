# 📁 Projektstruktur - ForellenWächter

## Übersicht

```
ForellenWaechter/
│
├── 📄 README.md                    # Hauptdokumentation
├── 📄 LICENSE                      # MIT Lizenz
├── 📄 .gitignore                   # Git Ignorierliste
├── 📄 CHANGELOG.md                 # Versionshistorie
├── 📄 QUICKSTART.md                # Schnellstart-Anleitung
├── 📄 WIRING.md                    # Verkabelungsanleitung
├── 📄 GITHUB_UPLOAD.md             # GitHub Upload Guide
│
├── 📂 ForellenWaechter/            # Arduino Projekt
│   ├── ForellenWaechter.ino       # Hauptcode (ESP32)
│   ├── config.h                   # Konfigurationsdatei
│   ├── sensors.h                  # Sensor-Funktionen
│   ├── webserver.h                # Web-Server Code
│   └── data_logger.h              # SD-Karten Logging
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

### 📂 ForellenWaechter/ (Arduino Projekt)

#### ForellenWaechter.ino
**Hauptcode** mit:
- setup() Funktion
- loop() Funktion
- WiFi Management
- Hauptlogik

#### config.h
**Konfiguration:**
```cpp
#define WIFI_SSID "..."
#define WATER_TEMP_MIN 8.0
#define PH_MIN 6.5
// ...
```

#### sensors.h
**Sensor-Funktionen:**
- readWaterTemp()
- readPH()
- readTDS()
- readWaterLevel()

#### webserver.h
**Web-Server:**
- Route-Handler
- API-Endpunkte
- WebSocket-Code

#### data_logger.h
**Datenlogging:**
- SD-Karten I/O
- CSV-Schreibfunktionen
- Datei-Management

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
│   └── (alle Dateien von oben)
│
├── hardware_tests/              # Temporäre Test-Sketches
│   └── quick_test.ino
│
├── backups/                     # SD-Karten Backups
│   ├── 2024-11-20/
│   └── 2024-11-22/
│
└── datasheets/                  # Sensor-Datenblätter
    ├── DS18B20.pdf
    ├── pH_sensor.pdf
    └── TDS_meter.pdf
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
