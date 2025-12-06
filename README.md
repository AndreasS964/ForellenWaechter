# 🐟 ForellenWächter (Trout Guardian)

**ESP32-basiertes IoT-Monitoring-System für Forellenzucht**

![Version](https://img.shields.io/badge/Version-1.6.2-blue)
![License](https://img.shields.io/badge/License-MIT-green)
![Platform](https://img.shields.io/badge/Platform-ESP32-red)
![LTE](https://img.shields.io/badge/LTE-Remote-orange)
![Power](https://img.shields.io/badge/Power-Hydro-cyan)
![Telegram](https://img.shields.io/badge/Telegram-Bot-blue)
![Status](https://img.shields.io/badge/Status-Active-brightgreen)

---

## 📋 Übersicht

ForellenWächter ist ein umfassendes Monitoring-System für Forellenaufzuchtanlagen. Es wurde speziell für Standorte mit natürlicher Quellwasserversorgung entwickelt und ermöglicht zuverlässiges Monitoring - auch von entfernten Standorten aus.

**Anwendung:** Forellenzucht-Monitoring
**Stromversorgung:** Autark mit Wasserkraft-Turbine ⚡💧
**Fernzugriff:** Telegram Bot + DynDNS 📱🌐 **NEU in v1.6.1!**

> **⚠️ v1.6.2 Bugfix-Release:** Kritische Bugfixes für v1.6.1 - Telegram/DynDNS funktionieren jetzt, Watchdog-Timeouts gefixt, UI vervollständigt!

### ✨ Features v1.6.2 - Bugfix & UI Release

**🐛 GEFIXT in v1.6.2:**
| Problem | Status |
|---------|--------|
| 🔴 **Telegram/DynDNS fehlten in loop()** | ✅ Beide jetzt funktional |
| 🔴 **Watchdog Timeout** | ✅ WDT Resets hinzugefügt |
| 🔴 **Compiler Error** | ✅ Forward Declaration gefixt |
| 🟡 **O₂ Chart unsichtbar** | ✅ JSON-Parsing gefixt |
| 📱 **Dashboard UI** | ✅ Turbine/Batterie Cards hinzugefügt |
| ⚙️ **Settings UI** | ✅ "Remote" Tab hinzugefügt |
| 📉 **Memory** | ✅ char[] statt String für Alarme |

**🆕 NEU in v1.6.1:**
| Feature | Beschreibung |
|---------|--------------|
| 📱 **Telegram Bot** | Fernsteuerung & Push-Alarme (optional, kostenlos) |
| 🌐 **DynDNS Support** | Dashboard von überall via DuckDNS (optional) |
| 📚 **Setup-Guides** | Detaillierte Anleitungen für Telegram & DynDNS |

**Features aus v1.6:**
| Feature | Beschreibung |
|---------|--------------|
| ⚡ **Wasserturbine** | 10W Mini-Hydro-Generator für autarke Stromversorgung |
| 💧 **Durchfluss-Monitoring** | L/min via Hall-Sensor (Echtzeit) |
| 🔋 **Batterie-Monitoring** | Spannung, % und Low-Battery Warnung (Pb/Gel) |
| 📊 **Power-Monitoring** | Turbinen-Leistung in Watt |
| 🌦️ **Verbessertes Wetter** | 2x täglich Update mit Stundenvorhersage |
| ⚠️ **Flow-Alarm** | Warnung bei zu geringem Durchfluss |

**Features aus v1.5:**
| Feature | Beschreibung |
|---------|--------------|
| 📡 **LTE/4G Remote** | Zugriff von überall über Mobilfunk |
| 📧 **E-Mail Alarme** | Sofortige Benachrichtigung bei Problemen |
| 🌡️ **Dual Temperatur** | Wasser- und Lufttemperatur (DS18B20) |
| 🧪 **Wasserqualität** | pH, TDS & DO Monitoring mit TDS in Charts |
| 🫧 **Sauerstoff** | Dissolved Oxygen Sensor (optional) |
| 🌊 **Wasserlevel** | Float Switch mit Alarm |
| 📊 **Live Dashboard** | Modernes Glassmorphism-Design |
| 📈 **Charts** | 24h/7d Verlaufsdiagramme (Temp, pH, O₂, TDS) |
| 💾 **SD-Logging** | CSV-Export für Langzeitanalyse |
| 🎛️ **3-Modi Relays** | Auto/An/Aus für alle 4 Relais |
| 🔄 **REST API** | Für externe Integrationen |
| ⚙️ **Settings-Seite** | Kalibrierung, Fischarten, Wetter |
| 🐟 **Fischarten-Presets** | Forelle, Karpfen, Tilapia, Custom |
| 🌤️ **Wetter-Widget** | 24h Vorhersage nach PLZ |
| 🎯 **Kalibrierung** | 2-Punkt pH & TDS Kalibrierung |
| 🔄 **OTA Updates** | Drahtlose Firmware-Updates |

---

## 📦 Versionen

| Version | Anwendung | Features |
|---------|-----------|----------|
| **v1.6.2 Stable** | Wasserkraft + Fernzugriff | 🐛 Kritische Bugfixes, UI vervollständigt, Watchdog gefixt |
| **v1.6.1 Remote** | Wasserkraft + Fernzugriff | 📱 Telegram Bot + 🌐 DynDNS (teilweise nicht funktional) |
| **v1.6 Turbine** | Wasserkraft + Remote | ⚡ Turbine, Flow & Power Monitor, Batterie-Status |
| **v1.5 LTE** | Netzbetrieb + Remote | Settings-Seite, Fischarten-Presets, 3-Modi Relays, Wetter |
| **v1.4 LTE** | Netzbetrieb + Remote | LTE, E-Mail, DO-Sensor, OTA, Kalibrierung |
| **v1.2 Full** | Netzbetrieb | Web-Dashboard, Charts, WebSocket |
| **v1.3 Deep Sleep** | Batteriebetrieb | 8 Monate auf 30Ah |

### Welche Version?

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│  Hast du Wasser-Durchfluss?                                 │
│                                                             │
│  JA → v1.6 Turbine Edition (AUTARK!)     ⚡💧              │
│                                                             │
│  Hast du Strom vor Ort?                                     │
│                                                             │
│  JA + Remote-Zugriff gewünscht → v1.5 LTE Edition          │
│  JA + nur lokal               → v1.2 Full Edition          │
│  NEIN (Batterie)              → v1.3 Deep Sleep Edition    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 🔧 Hardware

### Kernkomponenten (alle Versionen)

| Komponente | Bezugsquelle |
|------------|--------------|
| ESP32 DevKit v1 | Amazon/AliExpress |
| 2x DS18B20 (wasserdicht) | Amazon |
| pH-Sensor Modul | Amazon |
| TDS-Sensor Modul | Amazon |
| Float Switch | Amazon |
| 4-Kanal Relais | Amazon |
| SD-Karten Modul | Amazon |
| Gehäuse IP65 | Amazon |
| Diverses (Kabel etc.) | Baumarkt/Elektronik |

**Hinweis:** Detaillierte Einkaufsliste siehe [docs/HARDWARE.md](docs/HARDWARE.md)

### Zusatz für v1.4 LTE

| Komponente | Bezugsquelle |
|------------|--------------|
| LILYGO T-SIM7600E ODER SIM7600E Modul | Amazon/AliExpress |
| SIM-Karte (Daten) | Mobilfunkanbieter |
| LTE Antenne | Amazon |
| DFRobot DO-Sensor (optional) | DFRobot Shop |

### Verkabelung (Kurzübersicht)

```
GPIO 4  → DS18B20 Data (mit 4.7kΩ Pull-up)
GPIO 34 → pH-Sensor
GPIO 35 → TDS-Sensor
GPIO 32 → DO-Sensor (optional)
GPIO 33 → Float Switch

GPIO 26 → Relais 1 (Belüftung)
GPIO 27 → Relais 2 (Alarm)
GPIO 14 → Relais 3 (Reserve)
GPIO 12 → Relais 4 (Reserve)

GPIO 5  → SD-Karte CS
GPIO 17 → LTE TX (v1.4)
GPIO 16 → LTE RX (v1.4)
```

Detailliert: [docs/HARDWARE.md](docs/HARDWARE.md)

---

## 🚀 Installation

### 1. Arduino IDE vorbereiten

```
Board Manager URL:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

Bibliotheken:
- OneWire
- DallasTemperature
- ArduinoJson
```

### 2. Code herunterladen

```bash
git clone https://github.com/AndreasS964/ForellenWaechter.git
```

### 3. Konfiguration anpassen

```cpp
// WiFi
const char* AP_SSID = "ForellenWaechter";
const char* AP_PASSWORD = "YourPassword123";  // Ändern!

// LTE (v1.4)
#define LTE_APN "internet"

// E-Mail (v1.4)
const char* EMAIL_WEBHOOK_URL = "https://maker.ifttt.com/...";
```

### 4. Upload

- Board: "ESP32 Dev Module"
- Upload Speed: 921600
- Port auswählen
- Upload!

Ausführlich: [docs/INSTALLATION.md](docs/INSTALLATION.md)

---

## 📱 Dashboard

Das neue v1.4 Dashboard mit Glassmorphism-Design:

```
┌──────────────────────────────────────────────────────────────┐
│                        🐟                                    │
│                  FORELLENWÄCHTER                             │
│        Powered by Wasserkraft ⚡                              │
├──────────────────────────────────────────────────────────────┤
│  ○ WiFi: -45dBm    ○ LTE: 18/31    ○ Uptime: 2d 5h          │
├──────────────────────────────────────────────────────────────┤
│ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐      │
│ │  💧  │ │  🌡️  │ │  🧪  │ │  📊  │ │  🫧  │ │  🌊  │      │
│ │ 11.5 │ │ 18.2 │ │ 7.24 │ │ 185  │ │ 8.5  │ │  OK  │      │
│ │  °C  │ │  °C  │ │      │ │ ppm  │ │ mg/L │ │      │      │
│ └──────┘ └──────┘ └──────┘ └──────┘ └──────┘ └──────┘      │
├──────────────────────────────────────────────────────────────┤
│ 📈 Temperatur    📡 Verbindung    ⚙️ System    🎛️ Steuerung  │
└──────────────────────────────────────────────────────────────┘
```

**Zugriff:**
- Lokal: http://192.168.4.1
- mDNS: http://forellenwaechter.local
- Remote: Über LTE (öffentliche IP)

---

## 📡 LTE & E-Mail

### E-Mail Alarme einrichten (IFTTT)

1. **IFTTT Account** erstellen: https://ifttt.com
2. **Webhook Applet** erstellen (Event: `forellen_alarm`)
3. **URL** im Code eintragen:

```cpp
const char* EMAIL_WEBHOOK_URL = "https://maker.ifttt.com/trigger/forellen_alarm/with/key/DEIN_KEY";
```

Ausführlich: [docs/LTE.md](docs/LTE.md)

---

## 🔌 API

| Endpunkt | Methode | Beschreibung |
|----------|---------|--------------|
| `/api/sensors` | GET | Alle Sensorwerte |
| `/api/status` | GET | System- und LTE-Status |
| `/api/history` | GET | Historische Daten |
| `/api/settings` | GET/POST | Grenzwerte lesen/setzen |
| `/api/relay` | POST | Relais steuern |
| `/api/test-email` | POST | Test-E-Mail senden |
| `/api/calibration` | GET | Kalibrierungsdaten abrufen ✨ |
| `/api/calibration/ph` | POST | pH-Sensor kalibrieren ✨ |
| `/api/calibration/tds` | POST | TDS-Sensor kalibrieren ✨ |
| `/api/calibration/reset` | POST | Kalibrierung zurücksetzen ✨ |

**Beispiel:**
```bash
curl http://192.168.4.1/api/sensors
```

```json
{
  "waterTemp": 11.5,
  "airTemp": 18.2,
  "ph": 7.24,
  "tds": 185,
  "dissolvedOxygen": 8.5,
  "waterLevel": true,
  "alarm": false
}
```

Vollständig: [docs/API.md](docs/API.md)

---

## 📊 Grenzwerte (Regenbogenforelle)

| Parameter | Optimal | Warnung | Alarm |
|-----------|---------|---------|-------|
| Temperatur | 8-14°C | <8 / >14°C | >16°C |
| pH | 6.5-8.5 | außerhalb | - |
| TDS | <400 ppm | >400 | >500 |
| Sauerstoff | >9 mg/L | <9 mg/L | <6 mg/L |

---

## 📁 Projektstruktur

```
ForellenWaechter/
├── README.md
├── LICENSE
├── CHANGELOG.md
├── src/
│   ├── ForellenWaechter_v1.4_LTE.ino      # NEU: LTE + Remote
│   ├── ForellenWaechter_v1.2_Full.ino     # Netzbetrieb
│   └── ForellenWaechter_v1.3_DeepSleep.ino # Batterie
├── docs/
│   ├── INSTALLATION.md
│   ├── HARDWARE.md
│   ├── API.md
│   ├── LTE.md                              # LTE Setup Guide
│   ├── TELEGRAM_SETUP.md                   # Telegram Bot Setup
│   ├── DYNDNS_SETUP.md                     # DynDNS Setup
│   ├── ROADMAP.md
│   └── TROUBLESHOOTING.md
└── examples/
    └── sensor_test.ino
```

---

## 🔮 Roadmap

| Version | Features | Status |
|---------|----------|--------|
| **v1.4** | LTE, E-Mail, DO-Sensor | ✅ Aktuell |
| v1.5 | Multi-Becken (ESP-NOW) | 🚧 Q1 2025 |
| v1.6 | Fischdatenbank | 📅 Q2 2025 |
| v1.7 | Automatisierung | 📅 Q3 2025 |
| v2.0 | Mobile App, Cloud | 📅 Q4 2025 |

Details: [docs/ROADMAP.md](docs/ROADMAP.md)

---

## 🐛 Troubleshooting

| Problem | Lösung |
|---------|--------|
| LTE verbindet nicht | SIM-PIN deaktivieren, APN prüfen |
| E-Mail kommt nicht | Webhook-URL prüfen, IFTTT aktiv? |
| Sensoren zeigen 0 | TEST_MODE aktivieren, Verkabelung |
| Dashboard lädt nicht | Cache leeren, 192.168.4.1 nutzen |

Ausführlich: [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md)

---

## 📜 Lizenz

MIT License - siehe [LICENSE](LICENSE)

---

## 🙏 Credits

Entwickelt für professionelle Forellenzucht-Monitoring

---

## 📬 Beitragen

**Repository:** https://github.com/AndreasS964/ForellenWaechter

Issues und Pull Requests sind willkommen!

---

*Made with ❤️ für die Forellen* 🐟
