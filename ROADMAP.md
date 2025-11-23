# 🗺️ ForellenWächter Roadmap

## 📊 Aktueller Stand: v2.0.0 (November 2024)

### ✅ Erreichte Meilensteine
- ✅ Modulare Architektur (5 Dateien)
- ✅ Off-Grid Optimierung (56% weniger Energie)
- ✅ Modernes Dark/Light Mode UI
- ✅ PlatformIO Support
- ✅ 24h Statistiken
- ✅ MQTT Integration
- ✅ OTA Updates
- ✅ PWA Manifest

### 🔍 Code-Analyse: Was fehlt noch?

**Basierend auf ~2130 Zeilen Code:**

#### 🎨 UI/UX
- ❌ **Chart.js Visualisierung** - Historische Datenvisualisierung fehlt
- ❌ **Service Worker** - Für echte Offline-PWA
- ❌ **Kalibrierungs-UI** - Momentan nur hardcoded
- ❌ **Multi-Language** - Nur Deutsch
- ⚠️ **Login-System** - Sehr basic, Session-Timeout fehlt teilweise

#### ⚡ Performance & Energie
- ❌ **Deep Sleep Mode** - Batterie könnte noch länger halten
- ❌ **Live Power Monitoring** - Stromverbrauch nicht sichtbar im UI
- ❌ **Dynamisches CPU Scaling** - CPU-Frequenz könnte adaptiv sein
- ⚠️ **WiFi Auto-Reconnect** - Könnte stabiler sein

#### 📊 Daten & Logging
- ❌ **CSV Export-Funktion** - API-Endpoint fehlt
- ⚠️ **SD-Karte Logging** - Teilweise implementiert, nicht vollständig getestet
- ❌ **Datenkompression** - Logs werden groß
- ❌ **Automatisches Backup** - SD-Karte könnte ausfallen

#### 🔔 Benachrichtigungen & Alarme
- ❌ **Email Notifications** - SMTP fehlt komplett
- ❌ **Telegram Bot** - Bewusst ausgelassen in v2.0, aber wäre nützlich
- ❌ **Push Notifications** - Für PWA
- ❌ **Alarm-Historie** - Welche Alarme wann?

#### 🔧 Wartung & Debugging
- ❌ **Remote Debugging** - Nur Serial Monitor
- ❌ **Unit Tests** - Keine Tests vorhanden
- ❌ **System Health Dashboard** - CPU, RAM, WiFi Qualität
- ❌ **Error Log Viewer** - Fehler-Tracking im UI

#### 🌐 Integration & Erweiterungen
- ⚠️ **MQTT** - Implementiert aber minimal
- ❌ **REST API Dokumentation** - Keine Swagger/OpenAPI
- ❌ **Webhook Support** - Für externe Services
- ❌ **Home Assistant Discovery** - Auto-Discovery fehlt

---

## 🎯 Version 2.1 (Q1 2025)
**Fokus:** UI-Verbesserungen & Datenvisualisierung

### 🔥 Must-Have Features

#### 📈 1. Chart.js Integration (Prio 1)
**Warum:** Visualisierung ist essentiell für Trend-Analyse
```
Features:
- 24h Temperatur-Graph (Wasser & Luft)
- pH/TDS Trend-Linien
- Zoom & Pan Funktionen
- Export als PNG
- Responsive Charts für Mobile

Aufwand: 2-3 Tage
Impact: ⭐⭐⭐⭐⭐
```

#### 💾 2. Vollständiges SD-Karten Logging (Prio 1)
**Warum:** Langzeit-Daten sind wichtig für Optimierung
```
Features:
- Robuste CSV-Generierung
- Automatische Datei-Rotation (täglich)
- Download via Web-UI (/api/download)
- Daten-Kompression (optional)
- Status-Anzeige (Speicherplatz)

Aufwand: 2 Tage
Impact: ⭐⭐⭐⭐⭐
```

#### 🔋 3. Live Power Dashboard (Prio 2)
**Warum:** Energie-Optimierung braucht Transparenz
```
Features:
- Echtzeit Stromverbrauch (mA)
- Batteriespannungs-Graph
- Geschätzte Restlaufzeit
- Power-Save-Modus Status
- Energie-Statistiken (kWh/Tag)

Aufwand: 1-2 Tage
Impact: ⭐⭐⭐⭐
```

#### ⚙️ 4. Web-basierte Kalibrierung (Prio 2)
**Warum:** Aktuell muss man Code ändern
```
Features:
- pH Kalibrierung (2-Punkt: pH 4 & pH 7)
- TDS Kalibrierung (1413 µS/cm)
- Temperatur Offset
- Speichern in EEPROM/Preferences
- Kalibrierungs-Assistent

Aufwand: 2 Tage
Impact: ⭐⭐⭐⭐
```

#### 🌙 5. Deep Sleep Mode (Prio 3)
**Warum:** Off-Grid braucht maximale Effizienz
```
Features:
- Intervall-Messung (z.B. alle 15 Min)
- Wake-up via Timer
- RTC Speicherung
- Configurable Sleep Duration
- Ultra-Low-Power: ~5mA statt 80mA

Aufwand: 3-4 Tage (komplex!)
Impact: ⭐⭐⭐⭐⭐

Bedenken:
- WiFi Reconnect nach Wake-up dauert ~3s
- WebSocket-Verbindung wird unterbrochen
- Nicht für Live-Monitoring geeignet
```

### 🎁 Nice-to-Have Features

#### 📱 Service Worker (PWA Offline)
```
- Offline-Caching von UI
- Background Sync
- Install-Prompt
Aufwand: 1 Tag
Impact: ⭐⭐⭐
```

#### 🔔 Alarm-Historie
```
- Liste aller Alarme (Timestamp)
- Alarm-Statistiken
- Export als CSV
Aufwand: 1 Tag
Impact: ⭐⭐⭐
```

#### 🌡️ Erweiterte Sensoren
```
- Sauerstoff-Sensor (DO)
- Redox-Potential (ORP)
- Trübung (Turbidity)
Aufwand: 2-3 Tage pro Sensor
Impact: ⭐⭐⭐⭐
```

### 📦 v2.1 Release-Ziel
```
ETA: März 2025
Features: 5 Must-Have + 2 Nice-to-Have
Breaking Changes: Keine
Migration: Drop-in Upgrade
```

---

## 🚀 Version 2.2 (Q2 2025)
**Fokus:** Benachrichtigungen & Cloud-Integration

### 🔥 Hauptfeatures

#### 📧 1. Email Notifications
```
Features:
- SMTP Client (Gmail, Outlook, eigener Server)
- Alarm-Emails mit Sensor-Daten
- Täglicher Report (optional)
- HTML Email Templates
- Attachments (CSV-Logs)

Libraries: ESP Mail Client
Aufwand: 3 Tage
Impact: ⭐⭐⭐⭐
```

#### 🤖 2. Telegram Bot (Optional, On-Demand)
```
Features:
- /status - Aktuelle Werte
- /stats - 24h Statistiken
- /alarm on/off - Benachrichtigungen
- /photo - Kamera-Bild (wenn vorhanden)
- Interaktive Buttons

Aufwand: 2-3 Tage
Impact: ⭐⭐⭐⭐
```

#### 🔔 3. Push Notifications (PWA)
```
Features:
- Browser Push API
- Subscription Management
- Custom Notification Sounds
- Silent Notifications

Aufwand: 2 Tage
Impact: ⭐⭐⭐
```

#### 🏠 4. Home Assistant Integration
```
Features:
- MQTT Auto-Discovery
- Sensor Entities
- Binary Sensors (Alarme)
- Device Triggers
- Lovelace UI Card

Aufwand: 2-3 Tage
Impact: ⭐⭐⭐⭐⭐ (für Smart Home Fans)
```

#### 🌐 5. REST API Erweiterung
```
Features:
- OpenAPI/Swagger Dokumentation
- Authentication (API Keys)
- Rate Limiting
- Webhook Support
- JSON Schema Validation

Aufwand: 3 Tage
Impact: ⭐⭐⭐⭐
```

#### 📊 6. Erweiterte Analytik
```
Features:
- Trend-Erkennung (steigend/fallend)
- Anomalie-Detektion (einfache Heuristik)
- Vorhersagen (linear regression)
- Wochenvergleich

Aufwand: 4-5 Tage
Impact: ⭐⭐⭐⭐
```

### 📦 v2.2 Release-Ziel
```
ETA: Juni 2025
Features: 6 Hauptfeatures
Fokus: Kommunikation & Smart Home
```

---

## 🌟 Version 3.0 (Q4 2025)
**Fokus:** Cloud, KI & Automation

### 🔮 Vision: Intelligent Aquaculture System

#### ☁️ 1. Cloud-Integration (Optional)
```
Features:
- Cloud Dashboard (React/Vue.js)
- Multi-Device Synchronisation
- Remote Access (sicher via VPN/Cloudflare Tunnel)
- Mobile App (Flutter oder React Native)
- Cloud-Backup der Logs

Technologien:
- Backend: Firebase / Supabase / eigener Server
- Frontend: React + TailwindCSS
- Auth: OAuth2

Aufwand: 4-6 Wochen
Impact: ⭐⭐⭐⭐⭐
Kosten: Cloud-Hosting erforderlich
```

#### 🤖 2. Machine Learning
```
Features:
- Anomalie-Erkennung (TensorFlow Lite)
- Vorhersage von Problemen
- Optimale Fütterungszeiten
- Wachstumsrate-Prognosen

Modelle:
- LSTM für Zeitreihen
- Isolation Forest für Anomalien
- Edge ML auf ESP32 (quantisiert)

Aufwand: 6-8 Wochen
Impact: ⭐⭐⭐⭐⭐
Herausforderung: ESP32 RAM limitiert
```

#### 🎥 3. Kamera & Computer Vision
```
Features:
- ESP32-CAM Integration
- Fisch-Erkennung (YOLOv8 Nano)
- Fisch-Zählung
- Verhaltensanalyse
- Timelapse Videos

Hardware: ESP32-CAM (~10€)
Aufwand: 4-5 Wochen
Impact: ⭐⭐⭐⭐
```

#### 🍽️ 4. Automatische Fütterung
```
Features:
- Zeitgesteuerte Fütterung
- Menge basierend auf Wasserqualität
- Servomotor-Steuerung
- Fütterungs-Statistiken
- KI-optimierte Fütterungszeiten

Hardware: Servo + Futterautomat
Aufwand: 2 Wochen
Impact: ⭐⭐⭐⭐⭐
```

#### 🌍 5. Multi-Teich Management
```
Features:
- Mehrere ESP32 Nodes
- Zentrales Dashboard
- Teich-Vergleiche
- Mesh-Netzwerk (ESP-NOW)
- Redundanz & Failover

Aufwand: 3-4 Wochen
Impact: ⭐⭐⭐⭐⭐ (für kommerzielle Zucht)
```

#### 📱 6. Native Mobile App
```
Features:
- Android & iOS (Flutter)
- Push Notifications
- Offline Mode
- Kamera-Zugriff
- Widgets

Aufwand: 6-8 Wochen
Impact: ⭐⭐⭐⭐
```

### 📦 v3.0 Release-Ziel
```
ETA: Dezember 2025
Features: 6 Premium Features
Fokus: KI, Automation, Skalierung
Zielgruppe: Kommerzielle Züchter
```

---

## 🛠️ Technologie-Upgrade-Pfad

### Hardware-Erweiterungen

#### Phase 1 (v2.1-2.2)
```
✓ Aktuell ausreichend: ESP32, DS18B20, pH, TDS
+ Optional: Sauerstoff-Sensor (DO)
+ Optional: Batterievoltmeter (einfacher Spannungsteiler)
```

#### Phase 2 (v3.0)
```
+ ESP32-CAM für Vision
+ Servo für Fütterung
+ Größere SD-Karte (32GB+)
+ Externe RTC (DS3231) für präzise Zeit
+ Multi-Node Setup (mehrere ESP32)
```

#### Phase 3 (v3.0+)
```
+ LTE Modem (SIM7600) für Remote-Gebiete
+ LoRaWAN für große Distanzen
+ Wasserpumpen-Steuerung
+ Automatische Wasserwechsel-Steuerung
```

### Software-Stack Evolution

#### v2.x Stack
```
- Platform: PlatformIO + ESP-IDF
- Language: C++17
- Web: Vanilla JS + HTML/CSS
- Communication: WebSocket + MQTT
- Storage: SD Card (CSV)
```

#### v3.0 Stack
```
- Platform: ESP-IDF + Arduino Framework
- Language: C++20
- Web Frontend: React + TypeScript
- Mobile: Flutter (Dart)
- Backend: Node.js / Python FastAPI
- Database: InfluxDB (Zeitreihen) / PostgreSQL
- ML: TensorFlow Lite Micro
- Storage: Cloud + SD Card
```

---

## 📊 Entwicklungs-Priorisierung

### 🎯 Impact vs. Effort Matrix

```
                  Hoher Impact
                       │
        Chart.js ●     │     ● Deep Sleep
        CSV Export ●   │     ● Cloud
                       │     ● ML
        Kalibrierung ● │     ● Multi-Pond
    ───────────────────┼───────────────────
         Low Effort    │    High Effort
    ───────────────────┼───────────────────
        PWA SW ●       │     ● Native App
        Telegram ●     │     ● Computer Vision
                       │
                  Niedriger Impact
```

### 🚦 Entwicklungs-Phasen

#### ⚡ Quick Wins (1-2 Wochen)
1. Chart.js Integration ⭐⭐⭐⭐⭐
2. CSV Export & Download ⭐⭐⭐⭐⭐
3. Live Power Dashboard ⭐⭐⭐⭐
4. Service Worker (PWA) ⭐⭐⭐

#### 🏗️ Foundation (1-2 Monate)
1. Web-Kalibrierung ⭐⭐⭐⭐
2. Email Notifications ⭐⭐⭐⭐
3. Home Assistant Integration ⭐⭐⭐⭐⭐
4. REST API + Swagger ⭐⭐⭐⭐

#### 🚀 Advanced (3-6 Monate)
1. Deep Sleep Optimization ⭐⭐⭐⭐⭐
2. Telegram Bot ⭐⭐⭐⭐
3. Extended Analytics ⭐⭐⭐⭐
4. Multi-Language Support ⭐⭐⭐

#### 🌟 Premium (6-12 Monate)
1. Cloud Dashboard ⭐⭐⭐⭐⭐
2. Machine Learning ⭐⭐⭐⭐⭐
3. Computer Vision ⭐⭐⭐⭐
4. Native Mobile App ⭐⭐⭐⭐
5. Multi-Pond Management ⭐⭐⭐⭐⭐

---

## 💡 Innovative Ideen für v4.0+ (2026)

### 🔬 Forschung & Entwicklung

#### 1. Biologisches Monitoring
- **DNA-Analyse** - Wasserqualität via eDNA
- **Mikroskop-Integration** - Plankton-Erkennung
- **Parasiten-Detektion** - Computer Vision

#### 2. Autonome Systeme
- **Selbst-Reinigende Sensoren** - Servos für Reinigung
- **Automatischer Wasserwechsel** - Pumpen-Steuerung
- **Selbst-Diagnose** - System prüft eigene Sensoren

#### 3. Energie-Harvesting
- **Solar Panel Tracking** - Bewegliche Panels
- **Mikro-Wasserkraft** - Energie aus Durchfluss
- **Thermoelektrischer Generator** - Aus Temperaturdifferenz

#### 4. Fortgeschrittene KI
- **Reinforcement Learning** - Optimale Steuerung lernen
- **Federated Learning** - Von anderen Anlagen lernen
- **Voice Control** - "Alexa, wie geht's den Forellen?"

#### 5. Blockchain & IoT
- **Transparente Lieferkette** - Farm-to-Fork Tracking
- **Smart Contracts** - Automatische Qualitätszertifikate
- **NFT-Fische** - Einzigartige Fisch-Identifikation (fun!)

---

## 🎓 Community & Open Source

### 🌍 Projekt-Wachstum

#### Kurzfristig (2025)
- [ ] GitHub Stars: 100+
- [ ] Contributors: 5-10
- [ ] Forks: 20+
- [ ] Dokumentation: Wiki erstellen
- [ ] YouTube Tutorial-Serie

#### Mittelfristig (2026)
- [ ] NPM Package für Web-UI
- [ ] PlatformIO Library
- [ ] Academic Papers (Aquaculture Journals)
- [ ] Konferenz-Präsentationen
- [ ] Kooperation mit Fischzucht-Verbänden

#### Langfristig (2027+)
- [ ] Kommerzielle Version
- [ ] Hardware-Kit zum Verkauf
- [ ] Zertifizierung (CE, FCC)
- [ ] Startup-Gründung?

---

## 💰 Kosten-Kalkulation

### v2.0 (Aktuell)
```
Hardware:
- ESP32: 8€
- Sensoren: 40€
- SD-Karte: 10€
- Relays: 5€
- Gehäuse: 15€
- Solar/Batterie: 80€
─────────────────
Total: ~160€
```

### v3.0 (Vision)
```
Hardware (Pro Node):
- ESP32 + Cam: 15€
- Sensoren (erweitert): 100€
- Servo (Fütterung): 10€
- Größere SD: 15€
- LTE Modem: 30€
- Solar/Batterie: 150€
─────────────────
Total: ~320€ pro Teich

Software/Cloud:
- Hosting: 5€/Monat
- Domain: 12€/Jahr
- SSL Cert: Free (Let's Encrypt)
─────────────────
Laufende Kosten: ~60€/Jahr
```

---

## 🤔 Strategische Entscheidungen

### Was sollen wir NICHT bauen?

#### ❌ Blockchain-Integration
**Warum:** Unnötige Komplexität, hohe Kosten, zweifelhafter Nutzen

#### ❌ VR/AR Dashboard
**Warum:** Overkill für Aquakultur, keine echten Use Cases

#### ❌ Gaming-Features
**Warum:** Ablenkung vom Kern-Produkt

#### ⚠️ Full Cloud-Dependency
**Warum:** Off-Grid-Fokus widerspricht Cloud-Only
**Besser:** Hybrid (lokal + optional cloud)

---

## 🗳️ Community-Voting

### Was soll als Nächstes kommen?

**Stimme ab:** (GitHub Discussions / Issues)

1. **Chart.js Visualisierung** 📊
2. **Deep Sleep Mode** 🌙
3. **Email Notifications** 📧
4. **Home Assistant** 🏠
5. **Machine Learning** 🤖
6. **Mobile App** 📱
7. **Multi-Pond** 🌊
8. **Computer Vision** 🎥

---

## 📅 Release-Zeitplan (Tentativ)

```
┌─────────────────────────────────────────────────────────┐
│ 2024                                                    │
├─────────────────────────────────────────────────────────┤
│ Nov  │ ✅ v2.0 - Modular + Off-Grid                     │
│ Dez  │ 🔧 Bug Fixes, Community Feedback                 │
├─────────────────────────────────────────────────────────┤
│ 2025                                                    │
├─────────────────────────────────────────────────────────┤
│ Jan  │ 🚧 v2.1 Development Start                        │
│ Feb  │ 🚧 Chart.js + CSV Export                         │
│ Mar  │ 🎉 v2.1 Release                                  │
│ Apr  │ 🚧 v2.2 Development                              │
│ Mai  │ 🚧 Email + Telegram                              │
│ Jun  │ 🎉 v2.2 Release                                  │
│ Jul  │ 🏖️ Summer Break / Community                      │
│ Aug  │ 🚧 v3.0 Planning                                 │
│ Sep  │ 🚧 Cloud Infrastructure                          │
│ Okt  │ 🚧 ML Integration                                │
│ Nov  │ 🚧 Mobile App                                    │
│ Dez  │ 🎉 v3.0 Release                                  │
└─────────────────────────────────────────────────────────┘
```

---

## 🎯 Erfolgs-Metriken

### Technische Metriken
- Uptime: >99%
- Batterie-Laufzeit: >30 Tage
- API Response Time: <100ms
- Sensor-Genauigkeit: ±0.5%
- Code Coverage: >80%

### Business Metriken
- GitHub Stars: 500+
- Active Installations: 100+
- Community Contributors: 20+
- Documentation Quality: 9/10

### User Satisfaction
- Setup Time: <30min
- User Errors: <5%
- Support Tickets: <10/Monat
- NPS Score: >50

---

## 🤝 Mitmachen

### Wie du beitragen kannst:

#### 🐛 Bug Reports
- Issues auf GitHub erstellen
- Detaillierte Beschreibung
- Logs & Screenshots

#### 💻 Code Contributions
- Pull Requests willkommen
- Code Style Guide beachten
- Tests hinzufügen

#### 📚 Dokumentation
- Tutorials schreiben
- Übersetzungen
- Video-Anleitungen

#### 💡 Feature Requests
- GitHub Discussions nutzen
- Use Cases beschreiben
- Prioritäten abstimmen

#### 🎨 Design
- UI/UX Verbesserungen
- Icons & Grafiken
- 3D-Modelle für Gehäuse

---

## 📖 Inspiration & Referenzen

### Ähnliche Projekte
- **ESPHome** - YAML-basierte ESP32 Konfiguration
- **Tasmota** - Alternative Firmware
- **OpenMQTTGateway** - IoT Gateway

### Aquakultur-Systeme
- **FarmBot** - Automatisierte Landwirtschaft
- **Reef-Pi** - Aquarium-Controller
- **GrowBox** - Hydroponik-System

### Technologie
- **TinyML** - Machine Learning on Edge
- **Edge Impulse** - ML Platform
- **ThingsBoard** - IoT Platform

---

## 🏆 Ziel-Vision 2027

**ForellenWächter soll werden:**

✨ **Das führende Open-Source Aquakultur-Monitoring-System**

- 🌍 Weltweit >1000 Installationen
- 🏆 Genutzt in kommerziellen Zuchtanlagen
- 🎓 Referenziert in wissenschaftlichen Arbeiten
- 💼 Basis für Startup/Produkt
- 🌱 Beitrag zur nachhaltigen Fischzucht

---

**Erstellt:** November 2024
**Nächstes Update:** Nach v2.1 Release
**Status:** Living Document 📝

**Feedback willkommen!** 💬
