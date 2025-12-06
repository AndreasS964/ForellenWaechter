# 🔌 Verkabelungsanleitung - Schritt für Schritt

**ForellenWächter v1.6.2** - Kompletter Aufbau von Null

---

## 📦 Was du brauchst

### Checkliste Komponenten

#### Elektronik (Hauptkomponenten)
- [ ] ESP32 DevKit V1
- [ ] Waveshare SIM7600E-H 4G HAT (oder SIM7600 Modul)
- [ ] 2x DS18B20 Wasserdicht (3m Kabel)
- [ ] pH-Sensor Modul + BNC-Sonde
- [ ] TDS-Sensor Modul + Sonde
- [ ] DO-Sensor (optional - DFRobot Gravity)
- [ ] Float Switch (Schwimmschalter)
- [ ] 4-Kanal Relais Modul (5V, Low-Level-Trigger)
- [ ] microSD-Karten Modul (SPI)
- [ ] microSD-Karte (8-32GB, Class 10)

#### Stromversorgung & Turbine
- [ ] Mini Wasserturbine (12V 10W) mit Hall-Sensor
- [ ] 12V Batterie (7-20Ah, Blei-Gel oder LiFePO4)
- [ ] Laderegler LM2596 (12V → 13.8V)
- [ ] Step-Down Modul #1 (12V → 5V, 3A für ESP32)
- [ ] Step-Down Modul #2 (12V → 5V, 3A für LTE-Modul)

#### Kleinteile
- [ ] Widerstand 4.7kΩ (für DS18B20 Pullup)
- [ ] Widerstand 10kΩ (Spannungsteiler R1)
- [ ] Widerstand 3.3kΩ (Spannungsteiler R2)
- [ ] Widerstand 10kΩ (Turbine Hall-Sensor Pullup)
- [ ] Jumper-Kabel Male-Female (ca. 20 Stück)
- [ ] Jumper-Kabel Male-Male (ca. 10 Stück)
- [ ] Dupont-Stecker & Crimp-Werkzeug (optional, für saubere Verbindungen)
- [ ] Schrumpfschlauch (verschiedene Größen)
- [ ] Kabelbinder (ca. 20 Stück)

#### Werkzeug
- [ ] Lötkolben + Lötzinn
- [ ] Abisolierzange
- [ ] Seitenschneider
- [ ] Multimeter (wichtig!)
- [ ] Schraubendreher (Kreuz + Schlitz)
- [ ] Heißluftföhn oder Feuerzeug (für Schrumpfschlauch)

#### Gehäuse
- [ ] Gehäuse IP65 (mind. 200×150×100mm)
- [ ] Kabelverschraubungen M16 oder M20 (5-8 Stück)
- [ ] Montageschienen oder Abstandshalter

---

## ⚡ Schritt 1: Stromversorgung aufbauen

**WICHTIG:** Fang IMMER mit der Stromversorgung an, bevor du Komponenten verbindest!

### 1.1 Laderegler einstellen

**VOR dem Anschluss der Batterie!**

```
1. LM2596 Laderegler nehmen
2. Multimeter an OUT+ und OUT- anschließen
3. 12V Netzteil (oder Batterie kurz) an IN+ und IN- anschließen
4. Poti mit Schraubendreher drehen
5. Spannung auf 13.8V einstellen (für Blei-Gel Batterie)
6. Netzteil trennen
```

⚠️ **Bei LiFePO4 Batterie:** 14.6V einstellen!

### 1.2 Step-Down Module einstellen

**Beide Step-Down-Module auf 5V einstellen:**

```
1. Step-Down #1 (ESP32) nehmen
2. Multimeter an OUT+ und OUT- anschließen
3. 12V an IN+ und IN- anlegen
4. Poti drehen bis Multimeter 5.0V zeigt
5. Wiederholen für Step-Down #2 (LTE)
```

### 1.3 Verkabelung Stromversorgung

```
Batterie 12V (+) ────┬───────────────────────────────┬───────────────┐
                     │                               │               │
                     │                               │               │
                  LM2596                       Step-Down #1    Step-Down #2
                  IN+                              IN+             IN+
                     │                               │               │
                  OUT+ (13.8V)                    OUT+ (5V)      OUT+ (5V)
                     │                               │               │
                     └──► Batterie 12V (+)           │               │
                         (Ladung)              ESP32 VIN      SIM7600 VCC
                                                     │               │
                                               ESP32 GND      SIM7600 GND
                                                     │               │
                                                    GND             GND
                                                     │               │
Batterie 12V (-) ────┴───────────────────────────────┴───────────────┘
```

**Wichtig:**
- Alle GND zusammen verbinden (Sternpunkt!)
- Laderegler OUT+ zurück zu Batterie + (Ladekreis)
- Step-Down-Module parallel zur Batterie

---

## 🌡️ Schritt 2: Temperatursensoren (DS18B20)

**Einfachster Start - hier kannst du nichts kaputt machen!**

### 2.1 Farbcode prüfen

Die DS18B20 haben meist 3 Adern:
- 🔴 **Rot** = VCC (3.3V)
- 🟡 **Gelb** = DATA
- ⚫ **Schwarz** = GND

⚠️ **Achtung:** Manche haben andere Farben! Mit Multimeter durchmessen.

### 2.2 Verkabelung

```
DS18B20 #1 (Wasser)          DS18B20 #2 (Luft)
     │                            │
     ├── VCC (Rot) ──────┬────────┼── VCC (Rot) ──────► ESP32 3.3V
     │                   │        │
     │              [4.7kΩ]       │
     │                   │        │
     ├── DATA (Gelb) ────┴────────┼── DATA (Gelb) ────► ESP32 GPIO 4
     │                            │
     └── GND (Schwarz) ───────────┴── GND (Schwarz) ──► ESP32 GND
```

**Wichtig:**
- 4.7kΩ Widerstand zwischen DATA und 3.3V löten!
- Beide Sensoren parallel auf die gleichen Leitungen
- Lötstellen mit Schrumpfschlauch isolieren

### 2.3 Test

```cpp
// Nach Upload: Serial Monitor öffnen
// Sollte anzeigen:
// 🌡️ Wasser: 18.2°C
// 🌡️ Luft: 21.5°C
```

---

## 🧪 Schritt 3: Sensoren (pH, TDS, DO)

### 3.1 pH-Sensor

```
pH-Modul                        ESP32
────────────────────────────────────────
VCC (5V)        ────────────────► VIN (5V)
GND             ────────────────► GND
Signal/PO       ────────────────► GPIO 34 (ADC)
```

**BNC-Sonde:** In BNC-Buchse am Modul stecken

### 3.2 TDS-Sensor

```
TDS-Modul                       ESP32
────────────────────────────────────────
VCC (3.3-5V)    ────────────────► 3.3V
GND             ────────────────► GND
Signal/A        ────────────────► GPIO 35 (ADC)
```

**TDS-Sonde:** 2-poliges Kabel an Modul anschließen

### 3.3 DO-Sensor (optional)

```
DO-Modul (DFRobot)              ESP32
────────────────────────────────────────
VCC (3.3-5V)    ────────────────► 3.3V
GND             ────────────────► GND
Signal          ────────────────► GPIO 27 (ADC)
```

**Wichtig:**
- Alle Sensoren brauchen Zeit zum Aufwärmen (ca. 1 Min)
- Sonden NICHT trocken lagern!
- pH-Sonde in KCl-Lösung (Kalibrierlösung) lagern

---

## 💧 Schritt 4: Wasserlevel-Sensor

### 4.1 Float Switch

```
Float Switch                    ESP32
────────────────────────────────────────
Pin 1           ────────────────► GPIO 39
Pin 2           ────────────────► GND
```

**Funktion:**
- Schwimmer OBEN = Schalter OFFEN = HIGH (Wasser OK)
- Schwimmer UNTEN = Schalter GESCHLOSSEN = LOW (Alarm!)

**Im Code:** `INPUT_PULLUP` aktiviert → keine externe Pullup nötig!

---

## 🔋 Schritt 5: Batterie-Messung (Spannungsteiler)

**⚠️ KRITISCH:** GPIO 36 verträgt nur 0-3.3V!

### 5.1 Spannungsteiler löten

```
              ┌─── Batterie + (12V)
              │
          [10kΩ]  ← R1
              │
              ├─────────────► GPIO 36 (ADC)
              │
          [3.3kΩ] ← R2
              │
              └─── GND
```

**Löten:**
1. R1 (10kΩ) an ein Bein löten
2. R2 (3.3kΩ) an das andere Ende von R1 löten
3. Verbindungspunkt → GPIO 36
4. Freies Ende R1 → Batterie +
5. Freies Ende R2 → GND

### 5.2 Mit Multimeter testen!

**Berechnung:**
- Bei 12.6V Batterie: 12.6 × 3.3 / 13.3 = **3.13V** ✅ OK
- Bei 13.8V (Ladung): 13.8 × 3.3 / 13.3 = **3.42V** ⚠️ Knapp!

**Wenn über 3.3V:** Statt 3.3kΩ → 2.2kΩ verwenden (dann max. 3.0V bei 13.8V)

---

## ⚡ Schritt 6: Turbine & Flow-Sensor

### 6.1 Turbine verkabeln

```
Turbine                         Laderegler LM2596
────────────────────────────────────────────────────
Rot (+)         ────────────────► IN+
Schwarz (-)     ────────────────► IN- & GND
```

### 6.2 Hall-Sensor (Flow-Messung)

Die Turbine hat meist 3 Adern:
- 🔴 **Rot** = 12V
- ⚫ **Schwarz** = GND
- 🟡 **Gelb/Weiß** = Hall-Sensor Signal

```
Turbine Signal (Gelb)           ESP32
              │
          [10kΩ]  ← Pullup nach 3.3V
              │
              └─────────────────► GPIO 2
```

**Wichtig:** 10kΩ Pullup-Widerstand zwischen GPIO 2 und 3.3V!

---

## 🎛️ Schritt 7: Relais-Modul

### 7.1 Relais anschließen

```
4-Kanal Relais                  ESP32
────────────────────────────────────────
VCC             ────────────────► VIN (5V)
GND             ────────────────► GND
IN1             ────────────────► GPIO 32
IN2             ────────────────► GPIO 33
IN3             ────────────────► GPIO 25
IN4             ────────────────► GPIO 26
```

**Funktion:**
- IN1 → Relais 1 → Alarm (Sirene/Lampe)
- IN2 → Relais 2 → Reserve 1
- IN3 → Relais 3 → Reserve 2
- IN4 → Relais 4 → Belüftungspumpe

### 7.2 Relais-Ausgänge

**Low-Level-Trigger:**
- GPIO LOW → Relais EIN (COM mit NO verbunden)
- GPIO HIGH → Relais AUS

```
Relais 1 (Alarm):
COM ──────► 12V+
NO  ──────► Alarmsirene (+)
NC  ──────► nicht verwendet

Relais 4 (Belüftung):
COM ──────► 12V+
NO  ──────► Belüftungspumpe (+)
NC  ──────► nicht verwendet
```

**Geräte-Masse:** Alle Geräte (-) mit GND verbinden!

---

## 💾 Schritt 8: SD-Karte

### 8.1 SD-Modul anschließen

```
SD-Modul                        ESP32
────────────────────────────────────────
VCC             ────────────────► 3.3V (oder 5V, je nach Modul)
GND             ────────────────► GND
CS              ────────────────► GPIO 5
MOSI            ────────────────► GPIO 23
MISO            ────────────────► GPIO 19
SCK             ────────────────► GPIO 18
```

**microSD-Karte:**
- Class 10 oder besser
- FAT32 formatieren
- Maximal 32GB

---

## 📡 Schritt 9: LTE-Modul (Waveshare SIM7600)

### 9.1 Verkabelung

```
Waveshare SIM7600               ESP32 / Stromversorgung
───────────────────────────────────────────────────────────
VCC (5V)        ────────────────► Step-Down #2 OUT+ (5V)
GND             ────────────────► GND (Sternpunkt)
TXD             ────────────────► GPIO 16 (RX)
RXD             ────────────────► GPIO 17 (TX)
PWR_KEY         ────────────────► GPIO 25 (optional)
```

**Antenne:** SMA-Antenne auf ANT-Buchse schrauben

### 9.2 SIM-Karte

1. SIM-PIN im Handy deaktivieren!
2. SIM in den Slot am Waveshare-Modul schieben
3. Klick sollte hörbar sein

---

## 🔍 Schritt 10: Überprüfung vor dem ersten Start

### Checkliste Vor-Start

- [ ] **Multimeter:** Batterie-Spannung prüfen (12-13V)
- [ ] **Spannungsteiler:** Mit Multimeter GPIO 36 prüfen (<3.3V?)
- [ ] **Kurzschlüsse:** Mit Multimeter Durchgang GND ↔ 3.3V prüfen (sollte OFFEN sein!)
- [ ] **Stromversorgung:** Step-Down-Module zeigen 5.0V?
- [ ] **Verkabelung:** Alle Pins nochmal mit Schaltplan abgleichen
- [ ] **Sensoren:** Alle Sonden angeschlossen?
- [ ] **SD-Karte:** Formatiert und eingesteckt?
- [ ] **Antenne:** LTE-Antenne aufgeschraubt?

### Visuelle Inspektion

- [ ] Keine blanken Kabel berühren sich
- [ ] Lötstellen sauber und isoliert
- [ ] Keine kalten Lötstellen (matt/grau)
- [ ] Jumper-Kabel fest eingesteckt
- [ ] Schrumpfschlauch über Lötstellen

---

## 🚀 Schritt 11: Erster Start

### 11.1 Code hochladen

1. Arduino IDE öffnen
2. `ForellenWaechter_v1.6.1_LTE.ino` öffnen
3. **WiFi & LTE Konfiguration anpassen:**
   ```cpp
   const char* AP_SSID = "ForellenWaechter";
   const char* AP_PASSWORD = "DeinPasswort123";  // ÄNDERN!
   #define LTE_APN "internet"  // Dein Provider-APN
   ```
4. Board: **ESP32 Dev Module**
5. Upload Speed: **921600**
6. Port auswählen
7. Upload!

### 11.2 Serial Monitor (115200 Baud)

Erwartete Ausgabe:
```
🐟 ForellenWächter v1.6.2 - Stable Edition
💾 Kalibrierungsdaten geladen
🌡️ Temperatursensoren: 2 gefunden
📡 LTE wird initialisiert...
   Modul: SIMCOM SIM7600E
✅ LTE verbunden!
   Operator: Telekom.de
   Signal: 18/31
📶 WiFi AP gestartet: ForellenWaechter
🌐 Webserver gestartet auf: http://192.168.4.1
```

### 11.3 Dashboard öffnen

1. Mit Handy zu WiFi "ForellenWaechter" verbinden
2. Passwort eingeben
3. Browser: `http://192.168.4.1`
4. Dashboard sollte laden! 🎉

---

## ⚙️ Schritt 12: Kalibrierung

### 12.1 Sensoren kalibrieren

1. Dashboard → Tab "Settings"
2. pH-Kalibrierung mit pH 4.0 und pH 7.0 Pufferlösung
3. TDS-Kalibrierung mit 1413 µS/cm Lösung
4. Details: [CALIBRATION.md](CALIBRATION.md)

### 12.2 Grenzwerte anpassen

- Tab "Fischarten" → "Regenbogenforelle" auswählen
- Oder eigene Werte einstellen

---

## 🐛 Häufige Fehler

### "ESP32 startet immer neu" (Bootloop)

**Ursache:** Kurzschluss oder zu wenig Strom

**Lösung:**
- Stromversorgung prüfen (5V, min. 1A)
- LTE-Modul trennen (braucht viel Strom beim Start)
- Mit Multimeter VCC ↔ GND prüfen

### "Sensoren zeigen 0.00"

**Ursache:** Sensor nicht angeschlossen oder falsche Pins

**Lösung:**
- Pin-Belegung nochmal prüfen
- TEST_MODE aktivieren:
  ```cpp
  #define TEST_MODE true
  ```
- Sensor mit Multimeter durchmessen (Spannung am Signal-Pin?)

### "LTE verbindet nicht"

**Ursache:** SIM-PIN aktiv, falscher APN, schwaches Signal

**Lösung:**
- SIM-PIN im Handy deaktivieren!
- APN korrekt? (Provider prüfen)
- Antenne angeschraubt?
- Serial Monitor: CSQ Wert prüfen

### "SD-Karte nicht erkannt"

**Ursache:** Karte nicht formatiert, falsche Pins

**Lösung:**
- FAT32 formatieren (nicht exFAT!)
- Karte neu einstecken (Klick hörbar?)
- SPI-Pins nochmal prüfen (CS=5, SCK=18, MISO=19, MOSI=23)

---

## 📸 Tipps für saubere Verkabelung

### Best Practices

1. **Farbcode verwenden:**
   - Rot = VCC/+
   - Schwarz = GND/-
   - Gelb/Grün = Signal/Data
   - Blau = andere Signale

2. **Kabel beschriften:**
   - Mit Klebeband und Stift
   - Oder Beschriftungsgerät

3. **Kabelbinder nutzen:**
   - Bündel bilden
   - An Gehäuse fixieren
   - Zugentlastung!

4. **Schrumpfschlauch:**
   - Über ALLE Lötstellen
   - Vor dem Löten aufziehen!

5. **Testen in Schritten:**
   - NICHT alles auf einmal anschließen
   - Nach jedem Schritt testen
   - Bei Fehler: nur eine Komponente trennen

---

## 📚 Weitere Hilfe

- **Hardware-Details:** [HARDWARE.md](HARDWARE.md)
- **LTE-Setup:** [LTE.md](LTE.md)
- **Kalibrierung:** [CALIBRATION.md](CALIBRATION.md)
- **Troubleshooting:** [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

---

## ✅ Fertig!

Wenn alles läuft:
- ✅ Dashboard erreichbar
- ✅ Sensoren zeigen Werte
- ✅ LTE verbunden
- ✅ SD-Karte loggt

**Nächste Schritte:**
1. Sensoren kalibrieren
2. Telegram Bot einrichten
3. Gehäuse wetterfest machen
4. Am Teich installieren

**Viel Erfolg beim Aufbau!** 🐟⚡

---

*Bei Fragen: GitHub Issues oder direkt nachfragen!*
