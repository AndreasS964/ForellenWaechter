# 🎯 Sensor-Kalibrierung

Vollständige Anleitung zur Kalibrierung der Sensoren im ForellenWächter-System.

---

## Warum Kalibrieren?

Sensoren liefern ab Werk oft ungenaue Werte. Durch Kalibrierung mit bekannten Referenzwerten erreichst du **±0.1 pH Genauigkeit** und **±5% TDS Genauigkeit**.

**Ohne Kalibrierung:** pH ±0.5, TDS ±20% Fehler
**Mit Kalibrierung:** pH ±0.1, TDS ±5% Fehler

---

## 📋 Benötigte Materialien

### Für pH-Kalibrierung:
- **pH 4.0 Pufferlösung** (rot/orange)
- **pH 7.0 Pufferlösung** (gelb/grün)
- Destilliertes Wasser zum Spülen
- Weiches Tuch

**Bezugsquelle:** Amazon (~15€ für Set), Apotheke, Aquaristik-Shop

### Für TDS-Kalibrierung:
- **1413 µS/cm Kalibrierlösung** (= 1413 ppm)
  - ODER 1000 ppm / 500 ppm Lösung
- Destilliertes Wasser
- Sauberes Glas

**Bezugsquelle:** Amazon (~10€)

---

## 🧪 pH-Kalibrierung (2-Punkt)

### Schritt 1: pH 4.0 Pufferlösung

1. **Sensor reinigen**
   ```
   - Mit destilliertem Wasser abspülen
   - Mit weichem Tuch trocken tupfen
   - 5 Minuten an Luft trocknen lassen
   ```

2. **Sensor eintauchen**
   ```
   - pH 4.0 Lösung in sauberes Glas gießen
   - Sensor vollständig eintauchen
   - 2 Minuten warten (Stabilisierung)
   ```

3. **API-Call Schritt 1**
   ```bash
   curl -X POST http://192.168.4.1/api/calibration/ph \
     -H "Content-Type: application/json" \
     -d '{"step": 1, "buffer_value": 4.0}'
   ```

   **Response:**
   ```json
   {
     "success": true,
     "message": "Schritt 1 gespeichert",
     "adc": 2456
   }
   ```

4. **Sensor spülen & trocknen**

### Schritt 2: pH 7.0 Pufferlösung

1. **Sensor vorbereiten**
   ```
   - Mit destilliertem Wasser gründlich spülen
   - Trocken tupfen
   - 5 Minuten warten
   ```

2. **Sensor eintauchen**
   ```
   - pH 7.0 Lösung in sauberes Glas
   - Sensor eintauchen
   - 2 Minuten warten
   ```

3. **API-Call Schritt 2**
   ```bash
   curl -X POST http://192.168.4.1/api/calibration/ph \
     -H "Content-Type: application/json" \
     -d '{"step": 2, "buffer_value": 7.0}'
   ```

   **Response:**
   ```json
   {
     "success": true,
     "message": "Kalibrierung abgeschlossen",
     "slope": 0.0024,
     "offset": -4.1234
   }
   ```

4. **✅ Fertig!** Kalibrierung ist gespeichert

### Was passiert intern?

Das System berechnet eine **Geradengleichung** (y = mx + b):

```
pH = slope * ADC_Wert + offset

Beispiel:
- Bei pH 4.0: ADC = 2456
- Bei pH 7.0: ADC = 3012
- Slope = (7.0 - 4.0) / (3012 - 2456) = 0.0054
- Offset = 4.0 - (0.0054 * 2456) = -9.25
```

Die Werte werden im **EEPROM** gespeichert und überleben Neustarts!

---

## 📊 TDS-Kalibrierung (1-Punkt)

### Vorbereitung

1. **Kalibrierlösung vorbereiten**
   ```
   - 1413 µS/cm Lösung in sauberes Glas gießen
   - Raumtemperatur (20-25°C) abwarten
   ```

2. **Sensor reinigen**
   ```
   - Mit destilliertem Wasser abspülen
   - Trocken tupfen
   ```

### Kalibrierung

1. **Sensor eintauchen**
   ```
   - Sensor vollständig in Lösung
   - Leicht bewegen (Luftblasen entfernen)
   - 1 Minute warten
   ```

2. **API-Call**
   ```bash
   curl -X POST http://192.168.4.1/api/calibration/tds \
     -H "Content-Type: application/json" \
     -d '{"reference_value": 1413}'
   ```

   **Response:**
   ```json
   {
     "success": true,
     "message": "TDS kalibriert",
     "factor": 0.5234
   }
   ```

3. **✅ Fertig!**

### Alternative Lösungen

Falls du andere Kalibrierlösungen hast:

```bash
# 500 ppm Lösung
curl -X POST http://192.168.4.1/api/calibration/tds \
  -d '{"reference_value": 500}'

# 1000 ppm Lösung
curl -X POST http://192.168.4.1/api/calibration/tds \
  -d '{"reference_value": 1000}'
```

---

## 🔍 Kalibrierung prüfen

### Status abfragen

```bash
curl http://192.168.4.1/api/calibration
```

**Response:**
```json
{
  "ph": {
    "calibrated": true,
    "slope": 0.0024,
    "offset": -4.1234,
    "buffer1_adc": 2456,
    "buffer2_adc": 3012,
    "buffer1_value": 4.0,
    "buffer2_value": 7.0
  },
  "tds": {
    "calibrated": true,
    "factor": 0.5234,
    "reference_adc": 2701,
    "reference_value": 1413.0
  },
  "do": {
    "calibrated": false,
    "slope": 1.0,
    "offset": 0.0
  }
}
```

### Messwerte testen

```bash
curl http://192.168.4.1/api/sensors
```

Vergleiche die Werte mit deinen Referenzen:
- pH 7.0 Lösung → sollte ~7.0 anzeigen
- 1413 ppm Lösung → sollte ~1413 anzeigen

---

## 🔄 Kalibrierung zurücksetzen

Falls etwas schiefgeht:

```bash
curl -X POST http://192.168.4.1/api/calibration/reset
```

Setzt alle Kalibrierungen auf Standard-Werte zurück.

---

## ⏰ Wann neu kalibrieren?

| Sensor | Intervall | Grund |
|--------|-----------|-------|
| **pH** | Alle 3-6 Monate | Elektrodendrift |
| **TDS** | Alle 12 Monate | Sehr stabil |
| **DO** | Alle 6 Monate | Membranalterung |

**Sofort neu kalibrieren wenn:**
- Sensor getauscht wurde
- Unplausible Werte angezeigt werden
- Sensor versehentlich getrocknet wurde (pH)
- Nach längerer Lagerung

---

## 💡 Tipps & Tricks

### pH-Sensor Pflege

✅ **DO:**
- Immer feucht lagern (destilliertes Wasser)
- Schutzkappe fest verschließen
- Regelmäßig nachfüllen

❌ **DON'T:**
- Sensor niemals komplett austrocknen lassen!
- Nicht in Leitungswasser lagern (Kalk)
- Nicht mit Finger berühren

### TDS-Sensor Pflege

✅ **DO:**
- Regelmäßig mit Zitronensäure entkalken
- In destilliertem Wasser lagern
- Trocken lagern bei Langzeitpause

❌ **DON'T:**
- Nicht mit harten Bürsten reinigen
- Nicht in Salzlösungen dauerhaft lagern

### Kalibrierungslösungen

- **Haltbarkeit:** 12-24 Monate (original verschlossen)
- **Nach Öffnen:** 6 Monate (dunkel & kühl lagern)
- **Temperatur:** Bei Raumtemperatur kalibrieren!
- **Wiederverwendung:** Pufferlösungen NICHT wiederverwenden

---

## 🧮 Mathematik (Optional)

### pH 2-Punkt Kalibrierung

```cpp
// Gegeben:
float adc1 = 2456;  // pH 4.0
float adc2 = 3012;  // pH 7.0

// Berechnung:
float slope = (7.0 - 4.0) / (adc2 - adc1);
float offset = 4.0 - (slope * adc1);

// Messung:
float adc_messung = analogRead(PH_PIN);
float ph = (slope * adc_messung) + offset;
```

### TDS 1-Punkt Kalibrierung

```cpp
// Gegeben:
float reference_ppm = 1413.0;
float adc_reference = 2701;

// Berechnung:
float factor = reference_ppm / adc_reference;

// Messung:
float adc_messung = analogRead(TDS_PIN);
float tds = adc_messung * factor;

// Temperaturkompensation:
float temp_coeff = 1.0 + 0.02 * (water_temp - 25.0);
tds = tds / temp_coeff;
```

---

## 🆘 Troubleshooting

| Problem | Lösung |
|---------|--------|
| "Invalid JSON" Fehler | JSON-Syntax prüfen, Anführungszeichen korrekt? |
| pH schwankt stark | Sensor 5 Min. stabilisieren lassen, Pufferlösung frisch? |
| TDS zeigt 0 | Sensor richtig eingetaucht? Lösung leitfähig? |
| Kalibrierung geht verloren | EEPROM defekt? ESP32 tauschen |
| Werte unplausibel nach Kalibr. | Reset durchführen, neu kalibrieren |

---

## 📱 Web-UI Kalibrierung (geplant v1.5)

In Zukunft wird ein grafischer Kalibrierungs-Assistent im Dashboard integriert:

```
┌─────────────────────────────────────────┐
│  🎯 Sensor-Kalibrierung                 │
├─────────────────────────────────────────┤
│  pH-Sensor:                             │
│  [ ] Schritt 1: pH 4.0 Pufferlösung     │
│  [ ] Schritt 2: pH 7.0 Pufferlösung     │
│  [Kalibrieren]                          │
├─────────────────────────────────────────┤
│  TDS-Sensor:                            │
│  Referenzwert: [1413] ppm               │
│  [Kalibrieren]                          │
└─────────────────────────────────────────┘
```

---

**Fragen?** Siehe [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

**Letzte Aktualisierung:** 2024-12-01
