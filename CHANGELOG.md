# 📜 Changelog

Alle Änderungen am ForellenWächter Projekt.

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
