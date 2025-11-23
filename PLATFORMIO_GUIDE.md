# 🚀 PlatformIO Setup - ForellenWächter v2.0

## Installation

### 1. VS Code installieren

```bash
# Download von: https://code.visualstudio.com/
# Installieren und starten
```

### 2. PlatformIO Extension installieren

1. VS Code öffnen
2. Extensions (Strg+Shift+X)
3. Suche: "PlatformIO IDE"
4. Installieren (von PlatformIO)
5. VS Code neustarten

### 3. Projekt öffnen

```bash
# Projekt-Ordner in VS Code öffnen
# File -> Open Folder -> ForellenWaechter/
```

PlatformIO erkennt automatisch die `platformio.ini`!

---

## 📂 Projektstruktur

```
ForellenWaechter/
├── platformio.ini          # PlatformIO Konfiguration
├── src/                    # Quellcode
│   ├── main.cpp            # Hauptprogramm (vorher .ino)
│   ├── config.h            # Konfiguration
│   ├── power_management.h  # Energie-Management
│   ├── sensors.h           # Sensoren
│   └── webserver.h         # Web-Server
├── lib/                    # Eigene Bibliotheken (optional)
├── test/                   # Unit Tests (optional)
└── .pio/                   # Build-Dateien (gitignored)
```

---

## ⚙️ Konfiguration

### WiFi & Passwörter anpassen

Öffne `src/config.h` und passe an:

```cpp
// WiFi
const char* STA_SSID = "DeinWLAN";
const char* STA_PASSWORD = "DeinPasswort";

// Web-Login
const char* WEB_PASSWORD = "deinPasswort";
```

---

## 🔨 Build & Upload

### Option 1: VS Code GUI (Empfohlen)

1. **PlatformIO Toolbar** (unten):
   - ✓ **Build** - Kompilieren
   - → **Upload** - Auf ESP32 laden
   - 🔌 **Serial Monitor** - Debug-Ausgabe

2. **Oder über Sidebar:**
   - PlatformIO Icon (links)
   - PROJECT TASKS
   - esp32dev
     - General
       - Build
       - Upload
       - Monitor

### Option 2: Terminal

```bash
# Kompilieren
pio run

# Upload auf ESP32
pio run --target upload

# Serial Monitor
pio device monitor

# Alles in einem
pio run -t upload && pio device monitor
```

---

## 🎯 Environments (Umgebungen)

### 1. Development (Standard)

```bash
pio run -e esp32dev
```

**Features:**
- Debug Output aktiviert
- 80MHz CPU (Energiesparen)
- Serial Monitor

### 2. OTA Updates (Over-The-Air)

```bash
pio run -e esp32dev-ota -t upload
```

**Vorteile:**
- Upload über WiFi (kein USB!)
- ESP32 kann montiert bleiben
- URL: `forellenwaechter.local`

**Erste Verwendung:**
1. Einmal per USB uploaden
2. ESP32 startet und verbindet WiFi
3. Danach nur noch OTA!

### 3. Production (Release)

```bash
pio run -e esp32dev-prod
```

**Optimierungen:**
- Kein Debug Output
- Maximale Performance
- Minimaler Flash-Speicher

### 4. Debug (Debugging)

```bash
pio run -e esp32dev-debug
```

**Features:**
- Volle Debug-Infos
- Exception Decoder
- Timestamps

---

## 📊 Nützliche Befehle

```bash
# Bibliotheken aktualisieren
pio pkg update

# Clean (Build-Cache löschen)
pio run -t clean

# Geräteinformationen
pio device list

# Flash löschen
pio run -t erase

# Firmware-Info
pio run -t size

# Tests ausführen (wenn vorhanden)
pio test
```

---

## 🐛 Debugging

### Serial Monitor

```bash
# Monitor mit Filter
pio device monitor --filter esp32_exception_decoder

# Monitor mit Timestamps
pio device monitor --filter time

# Baudrate anpassen
pio device monitor -b 115200
```

### Exception Decoder

Bei Crashes automatisch aktiviert (esp32dev-debug):

```
Guru Meditation Error: Core 1 panic'ed...
```

Wird automatisch dekodiert zu:
```
In function: handleWebRequest
At line: sensors.cpp:123
```

---

## 📈 Performance Monitoring

### Build-Zeit

```bash
time pio run
```

**Arduino IDE:** ~45s
**PlatformIO:** ~12s ⚡

### Flash-Speicher

```bash
pio run -t size
```

```
RAM:   [==        ]  23.4% (used 76788 bytes)
Flash: [====      ]  35.2% (used 461684 bytes)
```

---

## 🔄 Migration von Arduino IDE

### Was ändert sich?

1. **.ino → .cpp** umbenennen
2. `#include <Arduino.h>` hinzufügen (bereits gemacht!)
3. Bibliotheken in `platformio.ini` (automatisch)
4. Upload über PlatformIO

### Was bleibt gleich?

- ✅ Alle Arduino-Funktionen
- ✅ Alle Bibliotheken verfügbar
- ✅ setup() und loop()
- ✅ Serial.print(), digitalWrite(), etc.

---

## 💡 VS Code Features

### IntelliSense (Auto-Completion)

Während du tippst:
```cpp
sensor.r → [TAB] → sensor.readAll()
```

### Go to Definition

```cpp
Strg+Click auf Funktion → Springt zur Definition
```

### Find All References

```cpp
Rechtsklick → Find All References
```

### Multi-Cursor

```cpp
Alt+Click → Mehrere Cursors
Strg+D → Nächstes Vorkommen
```

---

## 🎨 VS Code Extensions (Optional)

```bash
1. C/C++ (Microsoft) - Bereits installiert mit PlatformIO
2. GitLens - Git-Historie inline
3. Better Comments - Farbige Kommentare
4. Error Lens - Fehler inline anzeigen
5. Bracket Pair Colorizer - Klammern farbig
```

---

## ⚡ Tipps & Tricks

### 1. Schneller Build

In `platformio.ini` hinzufügen:

```ini
build_flags =
    -j4  ; Parallel kompilieren (4 Cores)
```

### 2. Custom Upload Port

```ini
upload_port = /dev/ttyUSB0  ; Linux
upload_port = COM5          ; Windows
```

### 3. Monitor Auto-Reconnect

```ini
monitor_dtr = 1
monitor_rts = 1
```

### 4. Bibliotheken lokal

```bash
# Bibliothek in lib/ Ordner legen
lib/
└── MyCustomLib/
    ├── MyCustomLib.h
    └── MyCustomLib.cpp
```

---

## 🆘 Troubleshooting

### "Device not found"

```bash
# Port manuell setzen
pio run -t upload --upload-port /dev/ttyUSB0
```

### "Permission denied" (Linux)

```bash
sudo usermod -a -G dialout $USER
# Logout/Login
```

### "Library not found"

```bash
# Bibliotheken installieren
pio pkg install
```

### "Out of memory"

In `platformio.ini`:

```ini
build_flags = -D BOARD_HAS_PSRAM
```

---

## 📚 Weiterführende Links

- **PlatformIO Docs:** https://docs.platformio.org/
- **ESP32 Platform:** https://docs.platformio.org/en/latest/platforms/espressif32.html
- **Arduino Framework:** https://docs.platformio.org/en/latest/frameworks/arduino.html
- **VS Code Shortcuts:** https://code.visualstudio.com/shortcuts/keyboard-shortcuts-windows.pdf

---

## ✅ Checkliste

Nach Installation solltest du können:

- [x] Projekt in VS Code öffnen
- [x] Code kompilieren (Build Button)
- [x] Auf ESP32 uploaden (Upload Button)
- [x] Serial Monitor öffnen (Monitor Button)
- [x] OTA Updates durchführen
- [x] Code mit IntelliSense bearbeiten

---

**Viel Erfolg mit PlatformIO! 🚀**

Bei Fragen: GitHub Issues öffnen
