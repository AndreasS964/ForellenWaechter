# 📋 Empfehlungen zur Weiterentwicklung

Dieses Dokument enthält Vorschläge für die weitere Entwicklung des ForellenWächter-Systems.

---

## 🎯 Priorität: HOCH

### 1. Konfigurationsdatei-System implementieren

**Status:** ✅ Teilweise umgesetzt (config.example.h erstellt)

**Nächste Schritte:**
- Code umschreiben, um `config.h` statt Hardcoded-Werte zu verwenden
- `#include "config.h"` in allen .ino Dateien einfügen
- Prüfung einbauen: `#ifndef CONFIG_H` → Fehlermeldung

**Vorteil:**
- Keine sensiblen Daten mehr im Code
- Einfache Anpassung ohne Code-Änderung
- Team-Entwicklung ohne Konflikte

### 2. OTA (Over-The-Air) Updates

**Beschreibung:** Firmware-Updates ohne physischen Zugriff

**Implementierung:**
```cpp
#include <ArduinoOTA.h>

void setupOTA() {
  ArduinoOTA.setHostname("forellenwaechter");
  ArduinoOTA.setPassword("admin");  // Aus config.h!

  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH)
      ? "sketch" : "filesystem";
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  // ... Rest
}
```

**Vorteil:**
- Fernwartung möglich
- Schnelle Bugfixes
- Keine Leiter nötig 😅

### 3. Sensor-Kalibrierung via Dashboard

**Aktuell:** Kalibrierungswerte im Code hardcoded

**Vorschlag:**
- Webinterface mit Kalibrierungs-Assistent
- Werte in EEPROM speichern
- 2-Punkt-Kalibrierung für pH
- TDS mit bekannter Referenzlösung

**API-Endpunkte:**
```
POST /api/calibration/ph
  Body: { "step": 1, "buffer": 7.0, "reading": 1823 }

POST /api/calibration/tds
  Body: { "ppm": 1413, "reading": 2456 }

GET /api/calibration
  Response: { "ph": {...}, "tds": {...} }
```

### 4. Alarm-Eskalation

**Aktuell:** Nur eine E-Mail bei Alarm

**Vorschlag:**
```
Alarm erkannt
  └─> Warte 5 Min
       └─> Immer noch Alarm?
            ├─> JA: 2. E-Mail + SMS (falls SIM7600)
            └─> NEIN: "Resolved"-E-Mail
```

**Vorteile:**
- Weniger Fehlalarme
- Eskalation bei echten Problemen
- Protokoll wann was passiert ist

### 5. Multi-Becken Support (ESP-NOW)

**Ziel:** Mehrere Becken mit einem Master überwachen

**Architektur:**
```
Master (LTE + Dashboard)
  ├─ Slave 1 (Becken A)
  ├─ Slave 2 (Becken B)
  └─ Slave 3 (Becken C)
```

**ESP-NOW Vorteile:**
- Kein WiFi-Router nötig
- Bis 200m Reichweite
- Sehr energieeffizient
- Automatische Pairing

**Implementierung:** Siehe `docs/ROADMAP.md` v1.5

---

## 🎯 Priorität: MITTEL

### 6. Datenbank-Integration

**Aktuell:** SD-Karte mit CSV

**Vorschlag:** InfluxDB oder SQLite

**InfluxDB (Cloud):**
```cpp
#include <InfluxDbClient.h>

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG,
                      INFLUXDB_BUCKET, INFLUXDB_TOKEN);

Point sensor("water_quality");
sensor.addTag("location", "main_tank");
sensor.addField("temperature", waterTemp);
sensor.addField("ph", phValue);
sensor.addField("tds", tdsValue);

client.writePoint(sensor);
```

**Vorteile:**
- Professionelle Zeitreihen-DB
- Grafana-Integration möglich
- Unbegrenzte Historie (Cloud)
- Aggregationen (Durchschnitt, Min, Max)

### 7. Grafana Dashboard

**Beschreibung:** Professionelles Monitoring-Dashboard

**Features:**
- Multi-Becken Übersicht
- Historische Daten-Analyse
- Alarme konfigurierbar
- Export als PDF-Report

**Alternative:** Selbst gehostet auf Raspberry Pi

### 8. Mobile App (React Native)

**Features:**
- Push-Benachrichtigungen (echter Push, nicht E-Mail)
- Kamera-Integration (ESP32-CAM)
- Offline-Fähig (cached Daten)
- Widgets für iOS/Android

**Aufwand:** Hoch (3-6 Monate Entwicklung)

### 9. Wetter-Integration

**API:** OpenWeatherMap (kostenlos bis 1000 calls/Tag)

**Nutzen:**
```cpp
struct Weather {
  float airTemp;        // Vorhersage
  float humidity;
  float rainProb;       // Regen → Wasserlevel?
  float uvIndex;        // UV → Algen?
};
```

**Smart Features:**
- "Morgen wird es heiß → Belüftung verstärken?"
- "Starkregen erwartet → Überlauf-Alarm vorbereiten"

### 10. Fischdatenbank

**Beschreibung:** Profile für verschiedene Fischarten

**Enthält:**
- Optimale Parameter (Temp, pH, O2)
- Fütterungshinweise
- Krankheits-Symptome
- Züchtungsinfos

**Implementierung:** JSON-Datei oder struct im Code

---

## 🎯 Priorität: NIEDRIG (Nice-to-have)

### 11. KI-Vorhersagen

**ML-Modell:** TensorFlow Lite für Mikrocontroller

**Anwendungen:**
- Temperatur-Trend Vorhersage (24h)
- Anomalie-Erkennung (plötzliche Änderungen)
- Optimale Belüftungs-Zeiten

**Aufwand:** Sehr hoch, braucht viele Trainingsdaten

### 12. Sprachsteuerung (Alexa/Google Home)

**Use Cases:**
- "Alexa, wie ist die Wassertemperatur?"
- "Alexa, schalte Belüftung an"

**Implementierung:** AWS Lambda + Alexa Skill

### 13. Automatische Fütterung

**Hardware:**
- Servo-Motor mit Futterspender
- Zeitgesteuert + temperaturabhängig

**Smart:**
- Bei Temperatur < 8°C → weniger Futter
- Bei Alarm → keine Fütterung

### 14. pH-Dosierung (automatisch)

**Hardware:**
- Peristaltik-Pumpen für pH+ / pH-
- Magnetventile

**Sicherheit:**
- Max. Dosis pro Tag
- Langsame Anpassung (nicht schockieren!)
- Doppel-Sensor Verifizierung

### 15. Kamera-Überwachung

**Hardware:** ESP32-CAM (5€)

**Features:**
- Foto bei Alarm
- Zeitraffer-Video
- KI: Fisch-Zählung? 🐟 (sehr experimentell)

---

## 🔧 Code-Qualität Verbesserungen

### 1. Unit Tests

**Framework:** AUnit (Arduino)

```cpp
#include <AUnit.h>

test(waterTempInRange) {
  float temp = 11.5;
  assertTrue(temp >= TEMP_MIN && temp <= TEMP_MAX);
}

test(phValueValid) {
  float ph = 7.2;
  assertTrue(ph >= PH_MIN && ph <= PH_MAX);
}
```

### 2. Code-Dokumentation (Doxygen)

```cpp
/**
 * @brief Liest Wassertemperatur von DS18B20 Sensor
 * @return Temperatur in °C oder -127 bei Fehler
 */
float readWaterTemperature() {
  // ...
}
```

**Generiert:** HTML-Dokumentation automatisch

### 3. Fehlerbehandlung verbessern

**Aktuell:** Teilweise fehlende Error-Checks

**Vorschlag:**
```cpp
enum ErrorCode {
  OK = 0,
  ERR_SENSOR_DISCONNECTED = 1,
  ERR_SD_CARD_FAILED = 2,
  ERR_LTE_NO_SIGNAL = 3,
  ERR_WATCHDOG_RESET = 4
};

ErrorCode initSensors() {
  if (!sensors.begin()) {
    return ERR_SENSOR_DISCONNECTED;
  }
  return OK;
}
```

### 4. Logging-System

**Levels:**
- DEBUG: Detaillierte Infos (nur bei Development)
- INFO: Normale Betriebsmeldungen
- WARN: Warnungen (noch kein Alarm)
- ERROR: Fehler (erfordert Eingriff)

**Implementierung:**
```cpp
#define LOG_DEBUG(msg) if(DEBUG_MODE) Serial.println("[DEBUG] " + String(msg))
#define LOG_INFO(msg) Serial.println("[INFO] " + String(msg))
#define LOG_WARN(msg) Serial.println("[WARN] " + String(msg))
#define LOG_ERROR(msg) Serial.println("[ERROR] " + String(msg))
```

### 5. State Machine

**Aktuell:** Viele if-else Checks

**Vorschlag:** Zustandsmaschine

```cpp
enum SystemState {
  STATE_INITIALIZING,
  STATE_NORMAL,
  STATE_WARNING,
  STATE_ALARM,
  STATE_MAINTENANCE
};

void handleState() {
  switch(currentState) {
    case STATE_NORMAL:
      // Normaler Betrieb
      if (alarmDetected()) currentState = STATE_ALARM;
      break;

    case STATE_ALARM:
      // Alarm-Aktionen
      if (alarmResolved()) currentState = STATE_NORMAL;
      break;

    // ...
  }
}
```

---

## 📊 Monitoring & Analytics

### Dashboard-Erweiterungen

**Neue Widgets:**
- [ ] Jahresübersicht (12 Monate)
- [ ] Vergleich: Heute vs. gestern
- [ ] Alarm-Historie (letzte 10)
- [ ] Sensoren Health Check
- [ ] Netzwerk-Status (Ping-Test)

**Export-Funktionen:**
- [ ] CSV-Download direkt im Browser
- [ ] PDF-Report generieren
- [ ] Excel-Template (.xlsx)

---

## 🌐 Community & Open Source

### GitHub Verbesserungen

**Issues Templates:**
- Bug Report Template
- Feature Request Template
- Hardware-Frage Template

**GitHub Actions:**
- Automatische Code-Checks
- Arduino Compile-Test
- Dokumentations-Build

**Discussions:**
- Community-Forum aktivieren
- Wiki für Tutorials

### Contribution Guidelines

**CONTRIBUTING.md erstellen:**
- Code-Style Guide
- Pull Request Template
- Testing Requirements

---

## 💡 Weitere Ideen (Brainstorming)

- [ ] Solar-Laderegler Integration
- [ ] Backup-Batterie mit automatischem Umschalten
- [ ] Wasserwechsel-Zähler (Durchflussmesser)
- [ ] Futter-Level Sensor (Ultraschall)
- [ ] Netzwerk-Mesh (mehrere Standorte)
- [ ] Blockchain-basierte Datenintegrität (😅 übertrieben?)
- [ ] Drohnen-Inspektion (sehr futuristisch)

---

## 📅 Vorgeschlagene Roadmap

| Phase | Zeitraum | Features |
|-------|----------|----------|
| **Phase 1** | Q1 2025 | Config-System, OTA, Kalibrierung-UI |
| **Phase 2** | Q2 2025 | Multi-Becken (ESP-NOW), InfluxDB |
| **Phase 3** | Q3 2025 | Mobile App, Alarm-Eskalation |
| **Phase 4** | Q4 2025 | KI, Automatisierung, Grafana |

---

## 🤝 Fazit

Das ForellenWächter-Projekt hat eine solide Basis. Die vorgeschlagenen Erweiterungen können schrittweise umgesetzt werden, je nach Bedarf und Priorität.

**Empfehlung:** Erst Phase 1 abschließen, dann Nutzer-Feedback einholen!

---

**Erstellt:** 2024-12-01
**Autor:** Code-Optimierung & Analyse
