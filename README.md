# 🐟 ForellenWächter (Trout Guardian)

Ein ESP32-basiertes IoT-Monitoring-System für die Forellenaufzucht von Lucas.

![Version](https://img.shields.io/badge/version-1.2-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![ESP32](https://img.shields.io/badge/platform-ESP32-red)

## 📋 Übersicht

ForellenWächter ist ein umfassendes Überwachungssystem für Forellenzuchtanlagen, das kritische Wasserparameter und Umgebungsbedingungen in Echtzeit erfasst. Das System wurde speziell entwickelt und ermöglicht eine proaktive Bewirtschaftung ohne ständige physische Anwesenheit.

### Hauptfunktionen

- ⏱️ **Echtzeit-Monitoring** von Wasser- und Umgebungsparametern
- 💾 **SD-Karten Datenlogging** mit CSV-Export
- 📊 **Live-Visualisierung** mit Chart.js (24h Historie)
- 🌐 **Web-Dashboard** optimiert für Mobile & Desktop
- 🔄 **Automatische Belüftungssteuerung**
- 📡 **WiFi-Konnektivität** mit Access Point Fallback
  

## 🎯 Überwachte Parameter

### Wasserqualität
- **Wassertemperatur**: Optimal 8-14°C
- **pH-Wert**: Optimal 6.5-8.5
- **TDS (Total Dissolved Solids)**: bis 500 ppm
- **Wasserlevel**: Float Switch Überwachung

### Umgebung
- **Lufttemperatur**: Kontinuierliche Messung

### Steuerung
- **Automatische Belüftung**: Temperatur- und sauerstoffgesteuert

## 🛠️ Hardware-Anforderungen

### Hauptkomponenten
- ESP32 Development Board
- 4-Kanal Relay Modul
- SD-Karten Modul
- Netzteil (5V/3A empfohlen)

### Sensoren
- DS18B20 Wassertemperatur-Sensor (wasserdicht)
- DS18B20 Lufttemperatur-Sensor
- pH-Sensor (analog)
- TDS-Sensor
- 2x Float Switch für Wasserlevel

### Optional
- Wasserdichtes Gehäuse
- LED-Anzeigen
- Buzzer für Alarme

## 📦 Installation

### 1. Arduino IDE Setup

```bash
# Arduino IDE installieren (Version 2.x empfohlen)
# ESP32 Board Support hinzufügen:
# File -> Preferences -> Additional Board Manager URLs:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

### 2. Benötigte Bibliotheken

Über Arduino Library Manager installieren:
- `WiFi` (ESP32 Core)
- `WebServer` (ESP32 Core)
- `ESPmDNS` (ESP32 Core)
- `SD` (Standard)
- `SPI` (Standard)
- `EEPROM` (ESP32 Core)
- `OneWire`
- `DallasTemperature`
- `ArduinoJson`

### 3. Hardware-Verkabelung

```
ESP32 Pin-Belegung:
├── GPIO 4  → DS18B20 (Wassertemperatur) + 4.7kΩ Pull-up
├── GPIO 5  → DS18B20 (Lufttemperatur) + 4.7kΩ Pull-up
├── GPIO 34 → pH-Sensor (analog)
├── GPIO 35 → TDS-Sensor (analog)
├── GPIO 32 → Float Switch 1
├── GPIO 33 → Float Switch 2
├── GPIO 18 → SD Card SCK
├── GPIO 19 → SD Card MISO
├── GPIO 23 → SD Card MOSI
├── GPIO 25 → SD Card CS
├── GPIO 26 → Relay 1 (Belüftung)
├── GPIO 27 → Relay 2
├── GPIO 14 → Relay 3
└── GPIO 12 → Relay 4
```

**Detaillierte Verkabelungsdiagramme:** siehe [WIRING.md](WIRING.md)

### 4. Software hochladen

1. ESP32 Board auswählen (Tools -> Board -> ESP32 Dev Module)
2. COM-Port auswählen
3. Code kompilieren und hochladen
4. Serieller Monitor öffnen (115200 baud)

## 🚀 Erste Inbetriebnahme

### WiFi-Konfiguration

1. **Erster Start**: ESP32 erstellt Access Point "ForellenWaechter-Setup"
2. Mit dem AP verbinden (Passwort: `forelle123`)
3. Browser öffnen: `http://192.168.4.1`
4. WiFi-Zugangsdaten eingeben
5. System startet neu und verbindet sich mit dem Netzwerk

### Zugriff auf das Dashboard

Nach erfolgreicher WiFi-Verbindung:
- **URL**: `http://forellenwaechter.local` oder IP-Adresse
- Dashboard zeigt sofort Live-Daten an
- Historische Daten werden automatisch geladen

## 📊 Web-Interface Features

### Dashboard
- Live-Anzeige aller Sensorwerte
- Aktueller Status der Belüftung
- Farbcodierte Warnungen bei kritischen Werten
- Auto-Refresh alle 2 Sekunden (WebSocket)

### Datenvisualisierung
- 24-Stunden-Charts für alle Parameter
- Interaktive Chart.js Diagramme
- Exportfunktion für historische Daten

### Konfiguration
- Schwellenwerte anpassen
- WiFi-Einstellungen ändern
- Sensor-Kalibrierung
- System-Reset

## 💾 Datenlogging

### CSV-Format
```csv
timestamp,water_temp,air_temp,ph,tds,water_level,aeration
2024-01-15 14:30:00,10.5,12.3,7.2,245,OK,OFF
```

### Datenspeicherung
- Alle 5 Minuten wird ein Datensatz gespeichert
- Automatische Dateirotation bei 1MB
- SD-Karte Format: FAT32

### Export
- Download via Web-Interface
- CSV-Import in Excel/LibreOffice
- Kompatibel mit Datenanalyse-Tools

## 🔧 Konfiguration

### Optimale Werte für Forellen

```cpp
// Temperatur
#define WATER_TEMP_MIN 8.0
#define WATER_TEMP_MAX 14.0
#define WATER_TEMP_CRITICAL 16.0

// pH-Wert
#define PH_MIN 6.5
#define PH_MAX 8.5

// TDS
#define TDS_MAX 500

// Belüftung
#define AERATION_TEMP_THRESHOLD 13.0
```

### Sensor-Kalibrierung

#### pH-Sensor
1. Kalibrierlösungen pH 4.0 und 7.0 verwenden
2. Werte im Code anpassen (siehe Kommentare)

#### TDS-Sensor
1. Mit destilliertem Wasser (0 ppm) kalibrieren
2. Mit Kalibrierlösung (1413 µS/cm) prüfen

## 📱 API-Endpunkte

```
GET  /api/data          - Aktuelle Sensorwerte (JSON)
GET  /api/history       - Historische Daten (JSON)
GET  /api/export        - CSV-Download
POST /api/config        - Konfiguration ändern
GET  /api/status        - Systemstatus
POST /api/relay/{id}    - Relay manuell steuern
```

### Beispiel API-Aufruf

```bash
curl http://forellenwaechter.local/api/data
```

Response:
```json
{
  "water_temp": 10.5,
  "air_temp": 12.3,
  "ph": 7.2,
  "tds": 245,
  "water_level": "OK",
  "aeration": false,
  "timestamp": 1705329600
}
```

## 🎨 Design & Branding

Das Interface verwendet die Lucas Haug Farbpalette:
- **Primär**: Wasser-Blau (#0077BE)
- **Sekundär**: Forellen-Grün (#2D5A3D)
- **Akzent**: Frisches Türkis (#00B4D8)
- **Warnung**: Bernstein (#FFA500)
- **Alarm**: Rot (#DC3545)

## 🔒 Sicherheit

- WiFi-Passwörter werden verschlüsselt im EEPROM gespeichert
- Keine externen Cloud-Verbindungen
- Lokales Netzwerk erforderlich
- Optional: Basic Authentication für Web-Interface

## 🐛 Troubleshooting

### ESP32 startet nicht
- USB-Kabel prüfen
- Stromversorgung prüfen (min. 500mA)
- Boot-Button beim Upload gedrückt halten

### WiFi verbindet nicht
- SSID/Passwort prüfen
- WiFi-Reset durchführen (Button gedrückt halten)
- Seriellen Monitor auf Fehlermeldungen prüfen

### Sensoren zeigen falsche Werte
- Verkabelung prüfen
- Pull-up Widerstände kontrollieren (4.7kΩ)
- Sensoren kalibrieren

### SD-Karte wird nicht erkannt
- Format: FAT32
- Max. 32GB
- CS-Pin prüfen (GPIO 25)

## 📈 Geplante Features (Roadmap)

- [ ] SMS/Email Benachrichtigungen
- [ ] Historische Datenanalyse mit ML
- [ ] Multi-Teich Unterstützung
- [ ] Fütterungsautomatik
- [ ] Wetterstation-Integration
- [ ] Mobile App (Android/iOS)

## 🤝 Beitragen

Contributions sind willkommen! Bitte:
1. Fork das Repository
2. Feature Branch erstellen (`git checkout -b feature/AmazingFeature`)
3. Commit (`git commit -m 'Add AmazingFeature'`)
4. Push (`git push origin feature/AmazingFeature`)
5. Pull Request öffnen

## 📄 Lizenz

Dieses Projekt ist unter der MIT-Lizenz lizenziert - siehe [LICENSE](LICENSE) Datei.

## 👤 Autor

**Andreas Sika**
- Entwickelt für Lucas Forellenzucht




## 📞 Support

Bei Fragen oder Problemen:
- GitHub Issues öffnen
- Dokumentation lesen
- Community Forum

---

**Version**: 1.2  
**Letztes Update**: November 2024  
**Status**: Production Ready (Hardware-Integration läuft)

