# 🚀 Quick Start Guide - ForellenWächter v2.0

## In 10 Minuten einsatzbereit!

**Neu in v2.0:** Modulare Struktur mit 5 Dateien + Off-Grid Optimierung

### 1️⃣ Hardware (5 Minuten)

**Minimale Verkabelung für ersten Test:**

```
ESP32 ──────► USB-Kabel ──────► Computer
   │
   ├─ GPIO 4  ──────► DS18B20 (Wassertemp) + 4.7kΩ zu 3.3V
   ├─ GPIO 5  ──────► DS18B20 (Lufttemp) + 4.7kΩ zu 3.3V
   ├─ GPIO 34 ──────► pH-Sensor (optional)
   ├─ GPIO 35 ──────► TDS-Sensor (optional)
   └─ GPIO 26 ──────► Relay IN1 (Belüftung)
```

**Stromversorgung:** 3.3V und GND von ESP32 zu Sensoren

---

### 2️⃣ Software (3 Minuten)

**Arduino IDE:**
1. Arduino IDE 2.x installieren
2. ESP32 Board hinzufügen:
   - File → Preferences → Additional Board Manager URLs:
   - `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Tools → Board → ESP32 → ESP32 Dev Module

**Bibliotheken:**
- Tools → Manage Libraries → Installieren:
  - `OneWire`
  - `DallasTemperature`
  - `WebSocketsServer` (by Markus Sattler)
  - `PubSubClient` (optional, für MQTT)

**Projekt-Dateien (NEU in v2.0):**
```
Alle 5 Dateien in einen Ordner:
├── ForellenWaechter_v2.0.ino
├── config.h
├── power_management.h
├── sensors.h
└── webserver.h
```

---

### 3️⃣ Konfiguration & Upload (2 Minuten)

**Konfiguration anpassen:**
1. `config.h` öffnen
2. WiFi-Daten eintragen (optional):
   ```cpp
   const char* STA_SSID = "DeinWLAN";
   const char* STA_PASSWORD = "DeinPasswort";
   ```
3. Passwort ändern (empfohlen):
   ```cpp
   const char* WEB_PASSWORD = "deinPasswort";
   ```

**Upload:**
1. `ForellenWaechter_v2.0.ino` öffnen
2. Board: **ESP32 Dev Module**
3. Upload Speed: **921600**
4. Flash Frequency: **80MHz** (wichtig für Energiesparen!)
5. **Upload** klicken ⚡
6. **Serial Monitor** öffnen (115200 baud)

---

### 4️⃣ WiFi Setup (1 Minute)

**Beim ersten Start:**

1. ESP32 erstellt Access Point: `ForellenWaechter-Setup`
2. Mit dem WiFi verbinden (Passwort: `forelle123`)
3. Browser: `http://192.168.4.1`
4. Deine WiFi-Daten eingeben
5. **Speichern** → ESP32 startet neu

**Zugriff:**
- URL: `http://forellenwaechter.local`
- Oder IP-Adresse (steht im Serial Monitor)

---

## ✅ Erfolgskontrolle

**Du solltest jetzt sehen:**
- ✅ Dashboard lädt (Dark Mode)
- ✅ Temperaturen werden angezeigt
- ✅ Dark/Light Mode Toggle funktioniert (☀️ Button)
- ✅ Live-Updates alle 2 Sekunden
- ✅ 24h Statistiken (Min/Max/Avg)
- ✅ Belüftungs-Button funktioniert
- ✅ Connection Status (unten rechts) zeigt "Verbunden"

---

## 🔍 Schnelltest

### Temperatursensoren
```
Serial Monitor sollte zeigen:
"Water: 12.5°C | Air: 15.2°C"
```

**Problem?**
- Verkabelung prüfen
- Pull-up Widerstände (4.7kΩ) vorhanden?

### Web-Interface
```
Browser: http://forellenwaechter.local
```

**Problem?**
- mDNS nicht verfügbar? → IP-Adresse nutzen
- Firewall blockiert? → Temporär deaktivieren

### Relay-Test
```
Dashboard → Belüftung ON/OFF klicken
Relay-LED sollte leuchten
```

---

## 📊 Was als Nächstes?

### Für Anfänger:
1. ✅ Dashboard beobachten
2. ✅ Dark/Light Mode ausprobieren
3. ✅ Sensoren ins Wasser hängen
4. ✅ 24h Statistiken beobachten

### Für Fortgeschrittene:
1. 📁 SD-Karte für Logging hinzufügen
2. 🏠 MQTT für Home Assistant konfigurieren
3. ⚙️ Schwellenwerte in `config.h` optimieren
4. 📱 Als PWA auf Smartphone installieren

### Für Profis:
1. 🔌 Alle Sensoren verkabeln (siehe WIRING.md)
2. 🏗️ Gehäuse bauen (wasserdicht!)
3. ☀️ Solar-Panel anschließen (Off-Grid)
4. 🔄 OTA Updates über WiFi nutzen
5. 🌐 Remote-Zugriff einrichten

### Off-Grid Setup:
1. 🔋 12V Batterie anschließen
2. ☀️ Solar-Panel (50-100W) mit Laderegler
3. 📊 Batteriespannung monitoren (optional GPIO36)
4. ⚡ Power-Save-Modus testen

---

## 🆘 Hilfe!

### "Kompilierung fehlgeschlagen"
→ Alle Bibliotheken installiert?
→ Richtiges Board gewählt?

### "Upload failed"
→ Boot-Button beim Upload gedrückt halten
→ Anderes USB-Kabel probieren

### "WiFi verbindet nicht"
→ SSID/Passwort korrekt?
→ 2.4 GHz WiFi (kein 5 GHz!)

### "Dashboard lädt nicht"
→ Gleiche WiFi wie ESP32?
→ IP-Adresse im Serial Monitor checken

---

## 📖 Vollständige Dokumentation

Für Details siehe:
- [README.md](README.md) - Vollständige Übersicht
- [WIRING.md](WIRING.md) - Detaillierte Verkabelung
- [GITHUB_UPLOAD.md](GITHUB_UPLOAD.md) - GitHub Anleitung

---

## 💡 Tipps

**Testen ohne Sensoren:**
- Code sendet Demo-Werte wenn keine Sensoren erkannt

**Erste Inbetriebnahme:**
- Erst ohne Wasser testen!
- Sensoren trocken verkabeln
- Dann vorsichtig ins Wasser

**Entwicklung:**
- Serial Monitor immer offen lassen
- Fehler werden dort angezeigt

---

**Viel Erfolg! 🐟**

*Weitere Fragen? → GitHub Issues öffnen*
