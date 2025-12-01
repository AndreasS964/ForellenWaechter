# 🔧 Troubleshooting

Lösungen für häufige Probleme mit dem ForellenWächter System.

---

## Inhaltsverzeichnis

1. [Upload-Probleme](#upload-probleme)
2. [WiFi-Probleme](#wifi-probleme)
3. [Sensor-Probleme](#sensor-probleme)
4. [SD-Karten Probleme](#sd-karten-probleme)
5. [Dashboard-Probleme](#dashboard-probleme)
6. [Allgemeine Probleme](#allgemeine-probleme)

---

## Upload-Probleme

### ESP32 wird nicht erkannt

**Symptome:**
- Kein COM-Port in Arduino IDE
- "Port" ist ausgegraut

**Lösungen:**

1. **USB-Treiber installieren**
   - CP2102: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
   - CH340: http://www.wch.cn/download/CH341SER_EXE.html

2. **Anderes USB-Kabel probieren**
   - Manche Kabel sind nur zum Laden (ohne Daten)
   - Original-Kabel bevorzugen

3. **USB-Hub vermeiden**
   - Direkt am PC anschließen

4. **Anderen USB-Port testen**
   - USB 2.0 oft stabiler als 3.0

---

### Upload schlägt fehl

**Symptome:**
- "Failed to connect to ESP32"
- "Timed out waiting for packet header"

**Lösungen:**

1. **BOOT-Button halten**
   - Button während Upload gedrückt halten
   - Loslassen wenn "Connecting..." erscheint

2. **Board-Einstellung prüfen**
   - Werkzeuge → Board → ESP32 Dev Module

3. **Upload Speed reduzieren**
   - Werkzeuge → Upload Speed → 115200

4. **Anderen ESP32 testen**
   - Eventuell defektes Board

---

## WiFi-Probleme

### Access Point nicht sichtbar

**Symptome:**
- "ForellenWaechter" WiFi nicht in Liste

**Lösungen:**

1. **Warten**
   - AP startet ~10 Sekunden nach Boot
   - Serial Monitor prüfen

2. **ESP32 neu starten**
   - Reset-Button drücken

3. **Serial Monitor prüfen**
   - Fehlermeldungen?

4. **2.4GHz Band nutzen**
   - ESP32 unterstützt nur 2.4GHz
   - Nicht 5GHz

---

### Kann nicht verbinden

**Symptome:**
- "Falsches Passwort"
- Verbindung bricht ab

**Lösungen:**

1. **Passwort prüfen**
   - Standard: `Lucas2024`
   - Im Code änderbar

2. **Gespeichertes Netzwerk löschen**
   - WiFi auf Gerät vergessen
   - Neu verbinden

3. **Andere Geräte trennen**
   - Max. ~4-5 Clients gleichzeitig

---

### Dashboard lädt nicht

**Symptome:**
- Browser zeigt "Verbindung fehlgeschlagen"
- Seite bleibt weiß

**Lösungen:**

1. **Richtige IP?**
   - AP: http://192.168.4.1
   - NICHT https://

2. **Cache leeren**
   - Strg+F5 / Cmd+Shift+R

3. **mDNS probieren**
   - http://forellenwaechter.local
   - Funktioniert nicht auf allen Geräten

4. **Anderen Browser testen**
   - Chrome/Firefox empfohlen

---

## Sensor-Probleme

### Temperatursensor zeigt -127°C oder 0

**Ursachen & Lösungen:**

1. **Verkabelung prüfen**
   ```
   VCC → 3.3V (ROT)
   GND → GND (SCHWARZ)
   DATA → GPIO4 (GELB)
   ```

2. **Pull-up Widerstand vergessen**
   - 4.7kΩ zwischen DATA und VCC

3. **Falsche Sensor-Adresse**
   - Automatische Erkennung läuft
   - Serial Monitor zeigt gefundene Sensoren

4. **Sensor defekt**
   - Mit Multimeter prüfen
   - Anderen Sensor testen

---

### pH-Wert unrealistisch (0 oder 14)

**Lösungen:**

1. **Kalibrierung nötig**
   - Im Code: `phValue = 7.0 + (2.5 - voltage) * 3.5;`
   - Mit Pufferlösung kalibrieren

2. **Verkabelung prüfen**
   - Ao → GPIO34
   - VCC → 5V (nicht 3.3V!)

3. **Elektrode prüfen**
   - In Aufbewahrungslösung lagern
   - Nicht austrocknen lassen

---

### TDS zeigt immer 0

**Lösungen:**

1. **Sonde im Wasser?**
   - Beide Elektroden müssen eingetaucht sein

2. **Kabel prüfen**
   - Stecker richtig eingesteckt?

3. **Analog-Pin richtig?**
   - Ao → GPIO35

---

### Float Switch funktioniert nicht

**Lösungen:**

1. **Pull-up aktiviert?**
   - Im Code: `INPUT_PULLUP`
   - Automatisch bei GPIO33

2. **Schwimmer bewegt sich?**
   - Mechanisch blockiert?
   - Position prüfen

3. **Logik invertiert?**
   - LOW = Wasser OK
   - HIGH = Wasser niedrig

---

## SD-Karten Probleme

### SD-Karte wird nicht erkannt

**Symptome:**
- "SD-Karte nicht gefunden" im Serial Monitor

**Lösungen:**

1. **FAT32 formatieren**
   - Windows: Rechtsklick → Formatieren → FAT32
   - Max. 32GB empfohlen

2. **Verkabelung prüfen**
   ```
   CS   → GPIO5
   MOSI → GPIO23
   MISO → GPIO19
   SCK  → GPIO18
   VCC  → 3.3V
   GND  → GND
   ```

3. **Kontakte reinigen**
   - Karte raus/rein
   - Kontakte mit Radiergummi säubern

4. **Andere Karte testen**
   - Nicht alle Karten kompatibel

---

### Keine Daten auf SD-Karte

**Lösungen:**

1. **Datei existiert**
   - Suche nach `/logs/day_XXX.csv`
   - oder `/log_XXXX.csv` (v1.3)

2. **Intervall abwarten**
   - Standard: Alle 5 Minuten

3. **Speicher voll?**
   - Alte Dateien löschen

---

## Dashboard-Probleme

### Charts zeigen keine Daten

**Lösungen:**

1. **History-Daten brauchen Zeit**
   - Alle 5 Minuten neuer Datenpunkt
   - Nach 1 Stunde sollten 12 Punkte da sein

2. **Browser-Cache**
   - Strg+Shift+R

3. **API testen**
   - http://192.168.4.1/api/history
   - Sind Daten vorhanden?

---

### Werte aktualisieren nicht

**Lösungen:**

1. **JavaScript-Fehler?**
   - F12 → Console prüfen

2. **API erreichbar?**
   - http://192.168.4.1/api/sensors

3. **ESP32 hängt?**
   - Watchdog sollte nach 60 Sek resetten
   - LED blinkt noch?

---

## Allgemeine Probleme

### ESP32 startet ständig neu (Boot-Loop)

**Ursachen & Lösungen:**

1. **Stromversorgung zu schwach**
   - USB-Port liefert nicht genug
   - Externes 5V Netzteil nutzen

2. **Watchdog greift**
   - Code hängt irgendwo
   - Serial Monitor prüfen

3. **Kurzschluss**
   - Verkabelung prüfen
   - Alles abstecken, einzeln testen

---

### System reagiert nicht mehr

**Lösungen:**

1. **Reset-Button drücken**
   - Kleiner Button am ESP32

2. **Strom trennen**
   - 10 Sekunden warten
   - Wieder anschließen

3. **Code neu flashen**
   - Manchmal hilft frischer Upload

---

### Speicher voll (Heap)

**Symptome:**
- Crashes nach einigen Stunden
- "Out of memory" Fehler

**Lösungen:**

1. **ESP32 neu starten**
   - Regelmäßig (z.B. täglich)

2. **History-Größe reduzieren**
   ```cpp
   #define HISTORY_SIZE 144  // statt 288
   ```

3. **Debug-Output reduzieren**
   ```cpp
   #define DEBUG_MODE false
   ```

---

## Debug-Tipps

### Serial Monitor nutzen

1. **Öffnen:** Werkzeuge → Serieller Monitor
2. **Baudrate:** 115200
3. **Alle Meldungen lesen!**

### TEST_MODE aktivieren

```cpp
#define TEST_MODE true
```
- Testet System ohne echte Sensoren
- Fake-Werte im Dashboard

### Einzelne Sensoren testen

Einfacher Test-Sketch für DS18B20:

```cpp
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(4);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  sensors.begin();
  Serial.printf("Gefunden: %d Sensoren\n", sensors.getDeviceCount());
}

void loop() {
  sensors.requestTemperatures();
  Serial.printf("Temp: %.2f°C\n", sensors.getTempCByIndex(0));
  delay(1000);
}
```

---

## Noch Probleme?

1. **GitHub Issues** erstellen
2. **Serial Monitor Output** anhängen
3. **Genaue Fehlerbeschreibung** geben
