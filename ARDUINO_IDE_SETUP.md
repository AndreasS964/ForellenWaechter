# 🎯 Arduino IDE Setup - ForellenWächter v2.1

**Arduino IDE ist die empfohlene Methode** - einfacher, stabiler und schneller als PlatformIO!

## ⚡ Schnellstart (5 Minuten)

### 1️⃣ **Arduino IDE 2.x installieren**
Download: https://www.arduino.cc/en/software

### 2️⃣ **ESP32 Board Manager installieren**

1. Arduino IDE öffnen
2. **Datei** → **Einstellungen** (oder **File** → **Preferences**)
3. Bei "Zusätzliche Boardverwalter-URLs" einfügen:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Klicke **OK**
5. **Werkzeuge** → **Board** → **Boardverwalter...** (oder **Tools** → **Board** → **Boards Manager**)
6. Suche nach **"esp32"**
7. Wähle **"esp32" von Espressif Systems**
8. Installiere **Version 2.0.14 oder höher** (wichtig für WebServer!)
9. Warte bis Download abgeschlossen ist (~150MB)

### 3️⃣ **Bibliotheken installieren**

**Sketch** → **Bibliothek einbinden** → **Bibliotheken verwalten...** (oder **Tools** → **Manage Libraries**)

Installiere folgende Bibliotheken (suche nach Namen, klicke "Installieren"):

| Bibliothek | Version | Autor | Pflicht? |
|------------|---------|-------|----------|
| **OneWire** | 2.3.7 oder höher | Paul Stoffregen | ✅ Ja |
| **DallasTemperature** | 3.11.0 oder höher | Miles Burton | ✅ Ja |
| **WebSockets** | 2.4.1 oder höher | Markus Sattler (links2004) | ✅ Ja |
| **PubSubClient** | 2.8 (optional) | Nick O'Leary | ⚪ Optional (für MQTT) |

**Wichtig:** Klicke bei jeder Bibliothek auf "Install all" wenn nach Dependencies gefragt wird!

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

### 5️⃣ **Projekt öffnen**

1. **Datei** → **Öffnen...** (oder **File** → **Open**)
2. Navigiere zum Projektordner `ForellenWaechter`
3. Öffne die Datei **`ForellenWaechter_v2.1.ino`**

### 6️⃣ **WiFi konfigurieren**

Öffne den Tab **`config.h`** und ändere:

```cpp
// WiFi Credentials
#define WIFI_SSID "DeinWiFiName"        // ← HIER ÄNDERN!
#define WIFI_PASSWORD "DeinPassword"    // ← HIER ÄNDERN!
```

### 7️⃣ **Kompilieren & Upload**

1. Klicke auf **✓** (Verifizieren/Verify) - sollte OHNE Fehler kompilieren!
   - Erster Build kann 1-2 Minuten dauern
   - Bei Erfolg: "Kompilierung abgeschlossen" / "Compilation complete"
2. ESP32 per **USB verbinden**
3. Port auswählen: **Werkzeuge** → **Port** → Wähle den COM/USB Port
4. Klicke auf **→** (Hochladen/Upload)
5. Warte bis "Upload complete" erscheint

### 8️⃣ **Serial Monitor öffnen**

1. **Werkzeuge** → **Serieller Monitor** (oder **Strg+Shift+M**)
2. Baud Rate auf **115200** einstellen (unten rechts)
3. Du solltest die Boot-Meldungen sehen:
   ```
   ═══════════════════════════════════════
      ForellenWächter v2.1
      Lucas Haug's Forellenzucht
      Off-Grid Optimiert
   ═══════════════════════════════════════
   ```
4. Notiere die **IP-Adresse** und **generierten Passwörter**!

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

## ❌ Häufige Probleme & Lösungen

### ❌ "WebServer.h: No such file or directory"
**Ursache:** ESP32 Board Manager zu alt (Version < 2.0)

**Lösung:**
1. **Werkzeuge** → **Board** → **Boardverwalter**
2. Suche "esp32"
3. Deinstalliere alte Version
4. Installiere Version **2.0.14 oder höher**
5. Arduino IDE neu starten

---

### ❌ "OneWire.h: No such file or directory"
**Ursache:** Bibliotheken nicht installiert

**Lösung:**
1. **Sketch** → **Bibliothek einbinden** → **Bibliotheken verwalten**
2. Installiere alle 3 Pflicht-Bibliotheken (siehe Schritt 3)

---

### ❌ ESP32 Port nicht sichtbar / "Port nicht verfügbar"
**Ursache:** Fehlende USB-Treiber

**Lösung:**

**Windows:**
- CP2102 Treiber: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
- CH340 Treiber: http://www.wch.cn/downloads/CH341SER_ZIP.html

**Mac:**
- CP2102 Treiber automatisch installiert
- Evtl. Neustart nötig

**Linux:**
- Kein Treiber nötig
- User zu `dialout` Gruppe hinzufügen:
  ```bash
  sudo usermod -a -G dialout $USER
  ```
- Logout & Login

**Danach:** Arduino IDE neu starten, ESP32 neu einstecken

---

### ❌ "A fatal error occurred: Failed to connect"
**Ursache:** ESP32 nicht im Upload-Modus

**Lösung:**
1. Klicke Upload (→)
2. Sobald "Connecting..." erscheint:
3. Halte **BOOT** Button am ESP32 gedrückt
4. Lasse los wenn Upload startet

---

### ❌ Kompilierung dauert ewig (>5 Minuten)
**Ursache:** Langsame Festplatte oder Antivirus

**Lösung:**
1. Arduino-Sketch-Ordner von Antivirus ausschließen
2. SSD statt HDD nutzen
3. Beim 2. Build ist es schneller (Cache)

---

---

## 🎓 Tipps & Tricks

### Serial Plotter nutzen
**Werkzeuge** → **Serieller Plotter** zeigt Sensor-Daten als Graph!

### OTA Updates (Over-The-Air)
Nach dem ersten Upload kannst du wireless updaten:
- **Werkzeuge** → **Port** → Wähle `forellenwaechter at <IP>`
- Kein USB-Kabel mehr nötig!

### Debug-Ausgaben aktivieren
In `config.h`:
```cpp
#define ENABLE_SERIAL_DEBUG true
```

### Performance-Modus
In `config.h` anpassen:
- `SENSOR_INTERVAL` - Zeit zwischen Messungen
- `ENABLE_LIGHT_SLEEP` - Stromsparmodus
- `DS18B20_RESOLUTION` - Temperatur-Auflösung (9-12 bit)

---

## ✅ Warum Arduino IDE statt PlatformIO?

| Feature | Arduino IDE | PlatformIO |
|---------|-------------|------------|
| **Einsteigerfreundlich** | ✅ Sehr einfach | ⚠️ Komplex |
| **Setup-Zeit** | ✅ 5 Minuten | ⚠️ 15-30 Minuten |
| **Stabilität** | ✅ Zuverlässig | ⚠️ Dependency-Probleme |
| **Build-Zeit** | ✅ Schnell | ⚠️ Langsam |
| **USB-Treiber** | ✅ Automatisch | ⚠️ Manuell |
| **Serial Monitor** | ✅ Integriert | ⚠️ Extern |
| **Für ESP32** | ✅ Perfekt | ⚙️ Overkill |

**Fazit:** Für ESP32-Projekte ist Arduino IDE die bessere Wahl!

---

## 📚 Weitere Ressourcen

- [ESP32 Pinout](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [DS18B20 Tutorial](https://randomnerdtutorials.com/esp32-ds18b20-temperature-arduino-ide/)
- [Arduino IDE Docs](https://docs.arduino.cc/software/ide-v2/)
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/en/latest/)

---

**Viel Erfolg! 🐟**

Bei Problemen: Issues auf GitHub öffnen oder README.md lesen!
