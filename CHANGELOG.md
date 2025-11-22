# Changelog - ForellenWächter

Alle nennenswerten Änderungen an diesem Projekt werden in dieser Datei dokumentiert.

Das Format basiert auf [Keep a Changelog](https://keepachangelog.com/de/1.0.0/),
und dieses Projekt folgt [Semantic Versioning](https://semver.org/lang/de/).

## [1.2.0] - 2024-11-22

### ✨ Hinzugefügt
- **SD-Karten Datenlogging** mit CSV-Export
- **24-Stunden Historie** für alle Sensoren
- **Chart.js Visualisierung** mit interaktiven Diagrammen
- **WebSocket Live-Updates** alle 2 Sekunden
- **Lucas Haug Branding** mit professionellem Design
- **REST API** für externe Zugriffe
- **Export-Funktion** für historische Daten
- **mDNS Support** (`forellenwaechter.local`)
- **EEPROM Konfiguration** für persistente Settings

### 🔧 Geändert
- **Interface Redesign** mit Wasser-Blau/Grün Farbschema
- **Optimierte Sensor-Abfrage** für bessere Performance
- **Verbesserte WiFi-Stabilität** mit Auto-Reconnect
- **Responsive Design** für alle Bildschirmgrößen

### 🐛 Behoben
- Float Switch Debouncing
- SD-Karten Schreibfehler bei voller Karte
- WebSocket Verbindungsabbrüche
- pH-Sensor Drift bei langen Laufzeiten

### 📚 Dokumentation
- Vollständige README.md
- Detaillierte WIRING.md
- GitHub Upload Anleitung
- API-Dokumentation

---

## [1.1.0] - 2024-11-15

### ✨ Hinzugefügt
- **Web-Dashboard** mit Live-Anzeige
- **WiFi-Konfiguration** über Web-Interface
- **Access Point Modus** für erste Einrichtung
- **Automatische Belüftungssteuerung**
- **Temperatur-basierte Alerts**

### 🔧 Geändert
- Von Serial Monitor zu Web-Interface migriert
- Sensor-Kalibrierung verbessert
- Code-Struktur aufgeräumt

### 🐛 Behoben
- Temperatur-Sensor Ausfälle
- WiFi Verbindungsprobleme
- Relay-Schaltlogik

---

## [1.0.0] - 2024-11-01

### ✨ Hinzugefügt
- **Grundlegende Sensor-Überwachung**:
  - Wassertemperatur (DS18B20)
  - Lufttemperatur (DS18B20)
  - pH-Wert (analog)
  - TDS/Wasserqualität (analog)
  - Wasserlevel (Float Switches)
- **4-Kanal Relay-Steuerung**
- **Serial Monitor Ausgabe**
- **ESP32 WiFi-Verbindung**

### 🎯 Projektziele
- Monitoring-System für Lucas Haug Forellenzucht
- Optimale Parameter-Überwachung für Forellenaufzucht
- Automatisierung der Belüftung

---

## [Unreleased]

### 🚀 Geplant für v2.0
- [ ] SMS/Email Benachrichtigungen
- [ ] Historische Datenanalyse mit Machine Learning
- [ ] Multi-Teich Support
- [ ] Fütterungsautomatik
- [ ] Wetterstation-Integration
- [ ] Mobile App (Android/iOS)
- [ ] OTA (Over-The-Air) Updates
- [ ] Backup auf Cloud-Speicher

### 💡 Ideen für spätere Versionen
- [ ] Sauerstoff-Sensor (DO)
- [ ] Trübungsmessung
- [ ] Ammoniak/Nitrit/Nitrat Messung
- [ ] Automatische Fütterung basierend auf Wasserqualität
- [ ] Video-Überwachung
- [ ] Fischzählung mit Computer Vision
- [ ] Integration mit Wetterdiensten
- [ ] Langzeit-Statistiken und Trends
- [ ] Vergleich mit anderen Anlagen
- [ ] KI-basierte Vorhersagen

---

## Version History Overview

| Version | Datum       | Highlights                          |
|---------|-------------|-------------------------------------|
| 1.2.0   | 2024-11-22  | SD-Logging, Charts, Branding        |
| 1.1.0   | 2024-11-15  | Web-Dashboard, Auto-Belüftung       |
| 1.0.0   | 2024-11-01  | Erste funktionierende Version       |

---

## Upgrade-Anleitung

### Von v1.1 auf v1.2

1. **Hardware**: SD-Karten Modul hinzufügen (siehe WIRING.md)
2. **Bibliotheken**: Keine neuen erforderlich
3. **Code**: Neue Version hochladen
4. **Konfiguration**: Bleibt erhalten (EEPROM)
5. **SD-Karte**: FAT32 formatiert einlegen

### Von v1.0 auf v1.1

1. **Keine Hardware-Änderungen** erforderlich
2. **WiFi neu konfigurieren** über Web-Interface
3. **Relay-Verkabelung prüfen** (LOW-aktiv!)

---

## Breaking Changes

### v1.2.0
- Keine Breaking Changes

### v1.1.0
- **WiFi-Konfiguration** nun über Web-Interface (nicht mehr hardcoded)
- **Relay-Logik** ist LOW-aktiv (invertiert)

### v1.0.0
- Erste Version, keine vorherigen Versionen

---

## Lizenz

Alle Versionen sind unter der MIT-Lizenz veröffentlicht.

---

## Mitwirkende

- **Andreas** - Hauptentwickler
- **Lucas Haug** - Anforderungen, Testing, Feedback

---

## Feedback & Bug Reports

Gefunden einen Bug oder hast einen Feature-Wunsch?
- 🐛 [Issue erstellen](https://github.com/DEIN-USERNAME/ForellenWaechter/issues)
- 💬 [Diskussion starten](https://github.com/DEIN-USERNAME/ForellenWaechter/discussions)

---

**Letzte Aktualisierung:** 22. November 2024  
**Aktuelle Version:** 1.2.0  
**Status:** Production Ready ✅
