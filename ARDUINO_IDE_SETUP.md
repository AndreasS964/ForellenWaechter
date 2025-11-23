# 🎯 Arduino IDE Setup - ForellenWächter v2.1

## ⚡ Schnellstart (5 Minuten)

### 1️⃣ **Arduino IDE 2.x installieren**
Download: https://www.arduino.cc/en/software

### 2️⃣ **ESP32 Board Manager installieren**

1. Arduino IDE öffnen
2. **Datei** → **Einstellungen**
3. Bei "Zusätzliche Boardverwalter-URLs" einfügen:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. **Werkzeuge** → **Board** → **Boardverwalter**
5. Suche nach "esp32" von Espressif Systems
6. Installiere **Version 2.0.14 oder höher** (wichtig für WebServer!)

### 3️⃣ **Bibliotheken installieren**

**Sketch** → **Bibliothek einbinden** → **Bibliotheken verwalten**, dann installieren:

| Bibliothek | Version | Autor |
|------------|---------|-------|
| OneWire | ^2.3.7 | Paul Stoffregen |
| DallasTemperature | ^3.11.0 | Miles Burton |
| WebSockets | ^2.4.1 | Markus Sattler |
| PubSubClient | ^2.8 (optional) | Nick O'Leary |

### 4️⃣ **Board-Einstellungen**

**Werkzeuge** → Folgende Einstellungen:

```
Board: "ESP32 Dev Module"
Upload Speed: 921600
CPU Frequency: 80MHz (Power Save!)
Flash Frequency: 80MHz
Flash Size: 4MB (32Mb)
Partition Scheme: Default 4MB with spiffs
Core Debug Level: None
PSRAM: Disabled
```

### 5️⃣ **Kompilieren & Upload**

1. Öffne `ForellenWaechter_v2.1.ino`
2. Klicke auf ✓ (Verifizieren) - sollte OHNE Fehler kompilieren!
3. ESP32 per USB verbinden
4. Port auswählen: **Werkzeuge** → **Port**
5. Klicke auf → (Hochladen)

---

## 🔧 Konfiguration

Vor dem Upload `src/config.h` bearbeiten:

```cpp
// WiFi Credentials
#define WIFI_SSID "DeinWiFiName"
#define WIFI_PASSWORD "DeinPassword"

// Sensoren konfigurieren
#define ENABLE_DO_SENSOR false     // DO-Sensor (optional)
#define ENABLE_INA219 false         // Power Monitor (optional)
#define ENABLE_LTE false            // LTE-Modul (optional)

// Security
#define ENABLE_CREDENTIALS_MANAGER true   // Empfohlen!
#define ENABLE_RATE_LIMITING true         // DoS-Schutz
```

---

## 📊 Web-Interface Zugriff

Nach Upload:
- URL: `http://forellenwächter.local` oder `http://<ESP32-IP>`
- Standard-Credentials werden beim ersten Start generiert (siehe Serial Monitor!)

---

## ❌ Häufige Probleme

### "WebServer.h not found"
→ **Lösung**: ESP32 Board Manager Version 2.0.x oder höher installieren!

### "Compilation error: ..."
→ **Lösung**: Alle Bibliotheken installiert? OneWire, DallasTemperature, WebSockets

### ESP32 wird nicht erkannt
→ **Lösung**:
1. CP2102/CH340 USB-Treiber installieren
2. Anderes USB-Kabel versuchen
3. Boot-Button beim Upload gedrückt halten

---

## ✅ Vorteile Arduino IDE vs. PlatformIO

✅ **Einfacher** - keine komplexe Konfiguration
✅ **Schneller** - direkter Upload ohne Build-System
✅ **Stabiler** - weniger Abhängigkeits-Konflikte
✅ **Visueller** - Serial Plotter, besseres Debugging

---

**Viel Erfolg! 🐟**
