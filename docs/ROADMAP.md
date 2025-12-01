# 🗺️ ForellenWächter Roadmap

Geplante Features und Entwicklungsrichtung für zukünftige Versionen.

---

## ✅ Abgeschlossene Versionen

### v1.0 - Basis-Monitoring
- [x] Temperatursensoren (DS18B20)
- [x] pH-Sensor, TDS-Sensor
- [x] Float Switch
- [x] Relais-Steuerung
- [x] Software Watchdog

### v1.1 - Web-Dashboard
- [x] Echtzeit-Dashboard
- [x] mDNS Support
- [x] Automatische Belüftung

### v1.2 - Charts & Logging
- [x] Chart.js Visualisierung
- [x] SD-Karten Datenlogging
- [x] WebSocket Live-Updates
- [x] REST API

### v1.3 - Deep Sleep
- [x] Ultra-Stromsparmodus
- [x] Battery Monitoring
- [x] RTC Memory

### v1.4 - LTE Remote (Aktuell)
- [x] SIM7600 LTE-Modul
- [x] E-Mail Alarme
- [x] Dissolved Oxygen Sensor (optional)
- [x] Verbessertes Dashboard
- [x] NTP Zeitsynchronisation

---

## 🚧 In Entwicklung

### v1.5 - Cloud & Multi-Becken (Q1 2025)

#### Multi-Becken Support

**Konzept:** Ein Master-Gerät koordiniert mehrere Slave-Geräte.

```
┌─────────────────────────────────────────────────────────────┐
│                      MASTER (v1.5)                          │
│                    ┌─────────────┐                          │
│                    │   ESP32     │                          │
│                    │  + LTE      │                          │
│                    │  + WiFi AP  │                          │
│                    └──────┬──────┘                          │
│                           │                                 │
│              ┌────────────┼────────────┐                    │
│              │            │            │                    │
│        ┌─────┴─────┐┌─────┴─────┐┌─────┴─────┐             │
│        │  SLAVE 1  ││  SLAVE 2  ││  SLAVE 3  │             │
│        │  Becken A ││  Becken B ││  Becken C │             │
│        │  (ESP32)  ││  (ESP32)  ││  (ESP32)  │             │
│        └───────────┘└───────────┘└───────────┘             │
│                                                             │
│              Kommunikation: ESP-NOW (2.4GHz)               │
└─────────────────────────────────────────────────────────────┘
```

**Features:**
- ESP-NOW Protokoll (kein Router nötig)
- Reichweite: ~200m im Freien
- Automatische Slave-Erkennung
- Zentrales Dashboard für alle Becken
- Individuelle Grenzwerte pro Becken

**Slave-Hardware (günstig):**
- ESP32-C3 Mini (kostengünstig)
- Nur benötigte Sensoren
- Batterie oder Solar möglich

#### Cloud-Dashboard

**Optionen:**
1. **ThingSpeak** (kostenlos bis 3M Datenpunkte/Jahr)
2. **InfluxDB Cloud** (kostenlos bis 30 Tage Retention)
3. **Eigener Server** (Raspberry Pi + Grafana)

**Datenfluss:**
```
ESP32 → LTE → MQTT/HTTP → Cloud → Grafana Dashboard
                              ↓
                         Mobile App
```

#### Geplante API-Erweiterungen

```
/api/pools                    # Alle Becken
/api/pools/{id}               # Einzelnes Becken
/api/pools/{id}/sensors       # Sensoren eines Beckens
/api/pools/{id}/settings      # Einstellungen pro Becken
/api/slaves                   # Verbundene Slave-Geräte
/api/slaves/{mac}/reboot      # Slave neustarten
```

---

### v1.6 - Fischdatenbank (Q2 2025)

#### Artspezifische Parameter

**Konzept:** Vordefinierte Parameter für verschiedene Fischarten.

```cpp
struct FishSpecies {
  String name;
  String latinName;
  float tempMin;
  float tempMax;
  float tempOptimal;
  float phMin;
  float phMax;
  float doMin;           // Sauerstoff mg/L
  float tdsMax;
  float ammoniaMax;      // NH3 (zukünftig)
  float nitriteMax;      // NO2 (zukünftig)
  String feedingNotes;
  String breedingNotes;
};
```

**Enthaltene Arten:**

| Art | Temp (°C) | pH | DO (mg/L) |
|-----|-----------|-----|-----------|
| Regenbogenforelle | 8-14 | 6.5-8.5 | >6 |
| Bachforelle | 6-16 | 6.5-8.0 | >7 |
| Saibling | 4-12 | 6.5-8.0 | >7 |
| Karpfen | 15-25 | 6.5-9.0 | >5 |
| Stör | 10-20 | 7.0-8.5 | >6 |
| Aal | 12-22 | 6.5-8.5 | >5 |

**Dashboard-Integration:**
- Art pro Becken auswählbar
- Grenzwerte automatisch gesetzt
- Warnungen angepasst an Art
- Saisonale Anpassungen

#### Fütterungs-Tracker

```cpp
struct FeedingLog {
  unsigned long timestamp;
  int poolId;
  float amount;          // Gramm
  String feedType;
  float waterTemp;       // Bei Fütterung
};
```

**Features:**
- Fütterungszeiten protokollieren
- Temperaturabhängige Empfehlungen
- Statistiken über Futterverbrauch

---

### v1.7 - Automatisierung (Q3 2025)

#### Automatische Fütterung

**Hardware:**
- Futterspender mit Servo/Motor
- Zeitgesteuert oder auf Knopfdruck
- Anpassung an Temperatur

#### Wasserwechsel-Automation

- Magnetventile für Zulauf/Ablauf
- Zeitgesteuerte Teilwasserwechsel
- Level-Sensor gesteuert

#### Wetter-Integration

```cpp
// OpenWeatherMap API
struct WeatherData {
  float airTemp;
  float humidity;
  float pressure;
  String condition;
  float rainProbability;
  float uvIndex;
};
```

**Nutzen:**
- Vorhersage von Temperaturänderungen
- Warnung vor Unwetter
- Anpassung der Belüftung bei Hitze

---

### v2.0 - Komplett-System (Q4 2025)

#### Mesh-Netzwerk

- Mehrere Master können sich verbinden
- Gemeinsame Cloud-Anbindung
- Backup-Routing bei Ausfall

#### KI-Vorhersagen

- Temperatur-Trend Vorhersage
- Anomalie-Erkennung
- Optimierungsvorschläge

#### Mobile App

- Native iOS/Android App
- Push-Benachrichtigungen
- Kamera-Integration
- Offline-Fähig

#### Hardware v2

- Custom PCB Design
- Integriertes Gehäuse
- Plug & Play Sensoren
- PoE Stromversorgung

---

## 💡 Feature-Requests

Ideen aus der Community:

| Feature | Priorität | Status |
|---------|-----------|--------|
| OTA Updates | Hoch | Geplant v1.5 |
| Backup auf USB-Stick | Mittel | Evaluierung |
| SMS statt E-Mail | Mittel | Möglich über SIM7600 |
| Sprachsteuerung | Niedrig | Nice-to-have |
| Solar-Integration | Mittel | Hardware-abhängig |
| pH-Dosierung | Hoch | Hardware nötig |
| UV-C Sterilisation | Mittel | Relais vorhanden |
| Kamera-Überwachung | Mittel | ESP32-CAM Add-on |

---

## 🔧 Technische Schulden

Dinge, die verbessert werden sollten:

- [ ] Kalibrierungsdaten in EEPROM speichern
- [ ] Bessere Fehlerbehandlung bei LTE-Ausfall
- [ ] Komprimierte History-Daten (weniger RAM)
- [ ] Firmware-Signierung für OTA
- [ ] Unit Tests für kritische Funktionen
- [ ] Code-Dokumentation (Doxygen)

---

## 🤝 Beitragen

Du hast Ideen oder willst mitentwickeln?

1. **Issues erstellen** für Feature-Requests
2. **Pull Requests** für Verbesserungen
3. **Wiki** für Dokumentation
4. **Discussions** für Fragen

---

## 📅 Release-Plan

| Version | Features | Geplant |
|---------|----------|---------|
| v1.4.1 | Bugfixes, Stabilität | Dezember 2024 |
| v1.5.0 | Multi-Becken, Cloud | März 2025 |
| v1.6.0 | Fischdatenbank | Juni 2025 |
| v1.7.0 | Automatisierung | September 2025 |
| v2.0.0 | Komplett-System | Dezember 2025 |

---

*Die Roadmap ist ein lebendiges Dokument und wird regelmäßig aktualisiert.*
