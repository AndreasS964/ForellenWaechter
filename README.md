# 🐟 ForellenWächter v1.6 - LTE Remote Edition (Refactored)

## ⚡ Was ist neu in v1.6?

**Kritische Bugfixes:**
- ✅ **HTML ausgelagert** - Kein RAM-Overflow mehr!
- ✅ **Streaming-Ausgabe** - HTML wird in Chunks gesendet statt als riesiger String
- ✅ **PROGMEM** - HTML/CSS/JS liegt im Flash, nicht im RAM
- ✅ **Watchdog-Resets behoben** - yield() und esp_task_wdt_reset() an kritischen Stellen
- ✅ **Konfiguration zentralisiert** - Alle Einstellungen in `config.h`

## 📁 Neue Dateistruktur

```
src/
├── ForellenWaechter_v1.6_LTE.ino   (1224 Zeilen - Hauptlogik)
├── config.h                         (133 Zeilen - Konfiguration)
├── html_dashboard.h                 (359 Zeilen - Dashboard HTML/CSS)
├── html_dashboard_js.h              (231 Zeilen - Dashboard JavaScript)
└── html_settings.h                  (293 Zeilen - Settings Page)
```

**Vorher v1.5:** ~3000 Zeilen in EINER Datei mit inline HTML!
**Jetzt v1.6:** Sauber aufgeteilt, ~40KB Hauptcode + ~50KB HTML in PROGMEM

## 🔧 Das Problem (ESP32 Bug)

Der v1.5 Code hatte das HTML direkt eingebettet:

```cpp
// SCHLECHT - v1.5
String getHTML() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    ... 500+ Zeilen HTML/CSS/JS ...
  )rawliteral";
  return html;  // ❌ Erstellt ~50KB String im RAM!
}
```

Das führte zu:
- **Stack Overflow** beim Erstellen des Strings
- **Heap Fragmentation** bei jedem Seitenaufruf
- **Watchdog Resets** weil die String-Erstellung zu lange dauert
- **Kompilierungsprobleme** bei manchen IDEs

## ✅ Die Lösung (v1.6)

```cpp
// GUT - v1.6
// HTML liegt in PROGMEM (Flash-Speicher)
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(...)rawliteral";

// Streaming-Ausgabe - kein großer String nötig!
void sendDashboardHTML() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  
  server.sendContent_P(DASHBOARD_HTML_HEAD);  // Direkt aus Flash
  yield(); esp_task_wdt_reset();              // Watchdog füttern
  
  server.sendContent_P(DASHBOARD_CSS);
  yield(); esp_task_wdt_reset();
  
  // ... weitere Chunks ...
}
```

## 🚀 Installation

1. **Alle Dateien** aus `src/` in deinen Arduino-Sketch-Ordner kopieren
2. **config.h anpassen:**
   - WiFi Passwort
   - OTA Passwort
   - E-Mail Webhook URL
3. **Libraries installieren** (Arduino Library Manager):
   - OneWire
   - DallasTemperature
   - ArduinoJson
4. **Board auswählen:** ESP32 Dev Module
5. **Hochladen!**

## 📊 RAM-Verbrauch Vergleich

| Version | Free Heap nach Start | Nach Dashboard-Aufruf |
|---------|---------------------|----------------------|
| v1.5    | ~150KB              | ~80KB (Crash-Risiko) |
| v1.6    | ~180KB              | ~175KB (Stabil!)     |

## 🐛 Debugging

Falls der ESP32 immer noch resettet:

```cpp
// In config.h
#define DEBUG_MODE true
#define WATCHDOG_TIMEOUT 180  // Erhöhen auf 3 Minuten
```

Serial Monitor (115200 baud) zeigt:
- Free Heap bei jedem Sensorread
- Watchdog-Resets als Stacktrace

## 📝 Upgrade von v1.5

1. Backup deiner `config.h` Einstellungen
2. Alle alten `.ino` Dateien löschen
3. Neue v1.6 Dateien kopieren
4. Einstellungen in neue `config.h` übertragen
5. Flashen!

## 🔗 Links

- GitHub: https://github.com/AndreasS964/ForellenWaechter
- Issues: Bugs hier melden!

---
**ForellenWächter v1.6** - Jetzt ohne RAM-Probleme! 🐟
