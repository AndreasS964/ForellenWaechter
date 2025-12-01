# 📦 Installation

Schritt-für-Schritt Anleitung zur Installation des ForellenWächters.

---

## Inhaltsverzeichnis

1. [Voraussetzungen](#voraussetzungen)
2. [Arduino IDE Setup](#arduino-ide-setup)
3. [Bibliotheken installieren](#bibliotheken-installieren)
4. [Code konfigurieren](#code-konfigurieren)
5. [Hochladen](#hochladen)
6. [Erster Test](#erster-test)
7. [Produktiv-Betrieb](#produktiv-betrieb)

---

## Voraussetzungen

### Software

- **Arduino IDE** 1.8.x oder 2.x ([Download](https://www.arduino.cc/en/software))
- USB-Treiber für ESP32 (meist CP2102 oder CH340)

### Hardware (Minimum für Test)

- ESP32 DevKit V1 (oder kompatibel)
- USB-Kabel (Micro-USB oder USB-C je nach Board)
- Computer mit Windows/Mac/Linux

---

## Arduino IDE Setup

### 1. ESP32 Board Support installieren

1. Arduino IDE öffnen
2. **Datei** → **Voreinstellungen**
3. Bei "Zusätzliche Boardverwalter-URLs" einfügen:

```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

4. **OK** klicken
5. **Werkzeuge** → **Board** → **Boardverwalter...**
6. Suchen nach "ESP32"
7. "esp32 by Espressif Systems" installieren
8. Warten bis Installation abgeschlossen

### 2. Board auswählen

**Werkzeuge** → **Board** → **ESP32 Arduino** → **ESP32 Dev Module**

### 3. Einstellungen prüfen

```
Board:           ESP32 Dev Module
Upload Speed:    921600
CPU Frequency:   240MHz (WiFi/BT)
Flash Frequency: 80MHz
Flash Mode:      QIO
Flash Size:      4MB (32Mb)
Partition Scheme: Default 4MB with spiffs
Core Debug Level: None
PSRAM:           Disabled
```

---

## Bibliotheken installieren

### Erforderliche Bibliotheken

1. **Sketch** → **Bibliothek einbinden** → **Bibliotheken verwalten...**

2. Suchen und installieren:

| Bibliothek | Version | Zweck |
|------------|---------|-------|
| **OneWire** | 2.3.7+ | DS18B20 Kommunikation |
| **DallasTemperature** | 3.9.0+ | Temperatur-Auslesen |

---

## Code konfigurieren

### 1. Code öffnen

`src/ForellenWaechter_v1.3.ino` in Arduino IDE öffnen

### 2. Für ersten Test: Test-Modus aktivieren

```cpp
// Zeile ~60-62 ändern:
#define DEEP_SLEEP_ENABLED    false   // Deep Sleep erstmal AUS
#define TEST_MODE             true    // Fake-Werte für Test
#define DEBUG_OUTPUT          true    // Ausführliche Ausgaben
```

---

## Hochladen

1. ESP32 per USB verbinden
2. **Werkzeuge** → **Port** → COM-Port auswählen
3. **Hochladen** Button klicken
4. Bei "Connecting..." evtl. **BOOT** Button am ESP32 drücken

---

## Erster Test

1. **Werkzeuge** → **Serieller Monitor** (115200 Baud)
2. Messwerte sollten erscheinen
3. Mit WiFi "ForellenWaechter" verbinden (PW: YourPassword123)
4. Browser: `http://192.168.4.1`

---

## Produktiv-Betrieb

```cpp
#define TEST_MODE             false   // Echte Sensoren
#define DEEP_SLEEP_ENABLED    true    // Stromsparen
#define DEBUG_OUTPUT          false   // Weniger Ausgaben
```

Mehr Details: [HARDWARE.md](HARDWARE.md) | [KONFIGURATION.md](KONFIGURATION.md)
