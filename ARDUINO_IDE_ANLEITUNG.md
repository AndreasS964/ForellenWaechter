# 🐟 ForellenWächter - Arduino IDE Setup

**Viel einfacher als PlatformIO!** Hier ist die Schritt-für-Schritt Anleitung:

---

## 📋 Voraussetzungen

1. **Arduino IDE 2.x** (Download: https://www.arduino.cc/en/software)
2. **ESP32 Board** (ESP32 Dev Module empfohlen)

---

## 🔧 Schritt 1: ESP32 Board Support installieren

1. Öffne Arduino IDE
2. Gehe zu: **File → Preferences**
3. Füge unter "Additional Board Manager URLs" diese URL ein:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
4. Klicke auf **OK**
5. Gehe zu: **Tools → Board → Boards Manager**
6. Suche nach **"ESP32"**
7. Installiere **"esp32 by Espressif Systems"** Version **6.4.0 oder neuer**
8. Warte bis Installation abgeschlossen ist

---

## 📚 Schritt 2: Bibliotheken installieren

Gehe zu: **Tools → Manage Libraries** (oder Strg+Shift+I)

Installiere folgende Bibliotheken:

### Pflicht-Bibliotheken:
- ✅ **OneWire** (by Paul Stoffregen)
- ✅ **DallasTemperature** (by Miles Burton)
- ✅ **WebSockets** (by Markus Sattler) ← WICHTIG!

### Optional (nur wenn aktiviert):
- **PubSubClient** (by Nick O'Leary) - für MQTT
- **Adafruit INA219** - für Strommessung

---

## 📁 Schritt 3: Projekt öffnen

1. Öffne die Datei: **`ForellenWaechter.ino`** in Arduino IDE
2. Arduino IDE sollte automatisch alle `.h` Dateien im `src/` Ordner erkennen

**Dateistruktur:**
```
ForellenWaechter/
├── ForellenWaechter.ino          ← Hauptdatei (öffne diese!)
└── src/
    ├── config.h
    ├── sensors.h
    ├── webserver.h
    ├── power_management.h
    ├── chart_data.h
    ├── credentials_manager.h
    ├── rate_limiter.h
    ├── network_manager.h
    ├── do_sensor.h
    └── ina219_monitor.h
```

---

## ⚙️ Schritt 4: Board-Einstellungen konfigurieren

Gehe zu **Tools** und wähle:

| Einstellung | Wert |
|-------------|------|
| **Board** | ESP32 Dev Module |
| **Upload Speed** | 921600 |
| **CPU Frequency** | 240MHz (WiFi/BT) |
| **Flash Frequency** | 80MHz |
| **Flash Mode** | QIO |
| **Flash Size** | 4MB (32Mb) |
| **Partition Scheme** | Default 4MB with spiffs |
| **PSRAM** | Disabled |
| **Port** | Wähle deinen COM-Port |

---

## 🔌 Schritt 5: ESP32 anschließen

1. Verbinde ESP32 via USB mit dem Computer
2. Warte bis Treiber installiert sind (Windows)
3. Wähle den richtigen **Port** unter **Tools → Port**
   - Windows: `COM3`, `COM4`, etc.
   - Mac/Linux: `/dev/ttyUSB0`, `/dev/cu.SLAB_USBtoUART`, etc.

---

## 🚀 Schritt 6: Kompilieren und Hochladen

1. Klicke auf den **✓ Verify** Button (kompilieren)
2. Warte bis Kompilierung abgeschlossen ist
3. Klicke auf den **→ Upload** Button
4. **WICHTIG:** Halte ggf. die **BOOT** Taste am ESP32 gedrückt beim Upload-Start

**Erwartete Ausgabe:**
```
Sketch uses 1234567 bytes (59%) of program storage space.
Global variables use 45678 bytes (13%) of dynamic memory.
esptool.py v4.5.1
Connecting........_
Writing at 0x00010000... (100 %)
Wrote 1234567 bytes (789012 compressed) at 0x00010000 in 12.3 seconds
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
```

---

## 📡 Schritt 7: Serial Monitor öffnen

1. Klicke auf: **Tools → Serial Monitor**
2. Stelle **Baudrate** auf: **115200**
3. Du solltest die Startmeldung sehen:

```
═══════════════════════════════════════
   ForellenWächter v2.1
   Lucas Haug's Forellenzucht
   Off-Grid Optimiert
═══════════════════════════════════════
Build: Jan 24 2025

✓ Sensoren initialisiert
✓ mDNS gestartet: http://forellenwaechter.local
✓ Web Server gestartet auf Port 80
✓ WebSocket gestartet auf Port 81

✓✓✓ SYSTEM BEREIT ✓✓✓

Web-Interface: http://192.168.4.1
Access Point: http://192.168.4.1
```

---

## 🌐 Schritt 8: Web-Interface öffnen

1. Verbinde dein Smartphone/PC mit dem WiFi: **`ForellenWaechter`**
2. Passwort: Siehe Serial Monitor (wird beim ersten Start generiert)
3. Öffne Browser: **http://192.168.4.1**
4. Fertig! 🎉

---

## ⚠️ Häufige Probleme & Lösungen

### Problem: "Compilation error: webserver.h: No such file or directory"
**Lösung:** Stelle sicher, dass der `src/` Ordner im gleichen Verzeichnis wie `ForellenWaechter.ino` ist.

### Problem: "A fatal error occurred: Failed to connect to ESP32"
**Lösung:**
- Halte **BOOT** Taste gedrückt beim Upload-Start
- Versuche langsamere Upload-Speed: **115200**
- Drücke **RESET** Taste am ESP32

### Problem: "WebServer does not name a type"
**Lösung:**
- Board muss **ESP32 Dev Module** sein (nicht ESP8266!)
- ESP32 Core 6.4.0+ muss installiert sein

### Problem: "WebSocketsServer.h: No such file or directory"
**Lösung:**
- Installiere **WebSockets by Markus Sattler** Bibliothek
- **NICHT** "ArduinoWebsockets" (falsche Library!)

### Problem: Kompilierung dauert ewig
**Lösung:**
- Normal! Erster Build kann 2-5 Minuten dauern
- Nächste Builds sind schneller

---

## 🎯 Konfiguration anpassen

Bearbeite `src/config.h` um Einstellungen zu ändern:

```cpp
// WiFi AP Name ändern
const char* AP_SSID = "MeinForellenWaechter";

// Sensor-Intervalle ändern (in Millisekunden)
#define SENSOR_INTERVAL 30000  // 30 Sekunden

// Grenzwerte anpassen
#define TEMP_MIN 8.0   // Minimum Wassertemperatur
#define TEMP_MAX 18.0  // Maximum Wassertemperatur
#define PH_MIN 6.5     // Minimum pH
#define PH_MAX 8.5     // Maximum pH
```

**Nach Änderungen:** Neu kompilieren und hochladen!

---

## 🔄 Over-The-Air (OTA) Updates

Nach dem ersten Upload kannst du Updates drahtlos machen:

1. **Tools → Port** → Wähle **ForellenWaechter at 192.168.x.x**
2. Klicke auf **Upload**
3. Fertig! Kein USB-Kabel nötig

---

## 📊 Features testen

- ✅ Web-Interface unter http://192.168.4.1
- ✅ Sensor-Werte Live-Anzeige
- ✅ Chart.js Diagramme (4h Verlauf)
- ✅ Belüftung An/Aus schalten
- ✅ Alarm-System testen (Grenzwerte in config.h temporär anpassen)
- ✅ Dark/Light Mode umschalten
- ✅ PWA: "Zum Homescreen hinzufügen" auf Smartphone

---

## 🆘 Support

Bei Problemen:
1. Prüfe **Serial Monitor** für Fehler
2. Stelle sicher, dass alle Bibliotheken installiert sind
3. ESP32 Core 6.4.0+ ist erforderlich
4. Board muss **ESP32 Dev Module** sein

**Viel Erfolg! 🚀**

---

**Entwickelt von Andreas S. • ForellenWächter v2.1 • MIT Lizenz**
