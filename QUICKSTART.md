# 🚀 Quick Start Guide - ForellenWächter

## In 10 Minuten einsatzbereit!

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
  - `ArduinoJson`

---

### 3️⃣ Code hochladen (2 Minuten)

1. `ForellenWaechter.ino` öffnen
2. Board auswählen: **ESP32 Dev Module**
3. Port auswählen (z.B. COM3 oder /dev/ttyUSB0)
4. **Upload** klicken ⚡
5. **Serial Monitor** öffnen (115200 baud)

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
- ✅ Dashboard lädt
- ✅ Temperaturen werden angezeigt
- ✅ Charts erscheinen (nach 1 Minute)
- ✅ Belüftungs-Button funktioniert

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
2. ✅ Sensoren ins Wasser hängen
3. ✅ Temperaturen überwachen

### Für Fortgeschrittene:
1. 📁 SD-Karte für Logging hinzufügen
2. 🎨 Dashboard-Design anpassen
3. ⚙️ Schwellenwerte optimieren

### Für Profis:
1. 🔌 Alle Sensoren verkabeln (siehe WIRING.md)
2. 🏗️ Gehäuse bauen (wasserdicht!)
3. 🌐 Remote-Zugriff einrichten

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
