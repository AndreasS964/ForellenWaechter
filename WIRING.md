# 🔌 Verkabelungsanleitung ForellenWächter

## Übersicht

Diese Anleitung zeigt die komplette Verkabelung des ForellenWächter Systems.

## ⚠️ Sicherheitshinweise

- **Stromversorgung trennen** vor allen Arbeiten
- **Keine Arbeiten bei Nässe** durchführen
- **Wasserdichte Gehäuse** für Außenbereiche verwenden
- **Schutzleiter** bei 230V Komponenten anschließen

## 📋 Teileliste

### Elektronik
- 1x ESP32 Development Board
- 1x 4-Kanal Relay Modul (5V)
- 1x SD-Karten Modul
- 1x Breadboard oder Lochrasterplatine
- 3x 4.7kΩ Widerstände (Pull-up)
- Jumper Kabel (männlich-männlich, männlich-weiblich)

### Sensoren
- 1x DS18B20 wasserdichter Temperatursensor (Wasser)
- 1x DS18B20 Temperatursensor (Luft)
- 1x Analog pH-Sensor Modul
- 1x TDS/EC Sensor
- 2x Float Switch (Schwimmerschalter)

### Stromversorgung
- 1x 5V 3A Netzteil (USB oder DC)
- Optional: 12V Netzteil für Relays/Pumpen

## 🔧 ESP32 Pin-Belegung

```
ESP32           Komponente              Beschreibung
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
GPIO 4    ─────► DS18B20 Data         Wassertemperatur
GPIO 5    ─────► DS18B20 Data         Lufttemperatur
GPIO 34   ─────► pH Sensor Analog     pH-Messung (Input Only!)
GPIO 35   ─────► TDS Sensor Analog    TDS-Messung (Input Only!)
GPIO 32   ─────► Float Switch 1       Wasserlevel Low
GPIO 33   ─────► Float Switch 2       Wasserlevel High
GPIO 18   ─────► SD SCK               SD-Karte Clock
GPIO 19   ─────► SD MISO              SD-Karte Data Out
GPIO 23   ─────► SD MOSI              SD-Karte Data In
GPIO 25   ─────► SD CS                SD-Karte Chip Select
GPIO 26   ─────► Relay 1 IN           Belüftung
GPIO 27   ─────► Relay 2 IN           Reserve
GPIO 14   ─────► Relay 3 IN           Reserve
GPIO 12   ─────► Relay 4 IN           Reserve
3.3V      ─────► Sensoren VCC         Stromversorgung
GND       ─────► Common GND           Masse
5V        ─────► Relay VCC            Relay Stromversorgung
```

## 📐 Detaillierte Verkabelung

### 1. Wassertemperatur-Sensor (DS18B20)

```
DS18B20 (wasserdicht)
┌─────────────┐
│   Rot       │ ──────► 3.3V
│   Schwarz   │ ──────► GND
│   Gelb      │ ──────► GPIO 4
└─────────────┘
                │
            4.7kΩ Pull-up zwischen GPIO 4 und 3.3V
```

**Hinweise:**
- Wasserdichte Variante mit Edelstahlhülse verwenden
- Kabel mit Schrumpfschlauch abdichten
- Im fließenden Wasser positionieren

### 2. Lufttemperatur-Sensor (DS18B20)

```
DS18B20 (normal)
┌─────────────┐
│   Rot       │ ──────► 3.3V
│   Schwarz   │ ──────► GND
│   Gelb      │ ──────► GPIO 5
└─────────────┘
                │
            4.7kΩ Pull-up zwischen GPIO 5 und 3.3V
```

**Hinweise:**
- Vor direkter Sonneneinstrahlung schützen
- In schattiger, gut belüfteter Position
- Mindestens 30cm über Wasseroberfläche

### 3. pH-Sensor

```
pH-Sensor Modul
┌─────────────┐
│   VCC       │ ──────► 5V (manche Module: 3.3V)
│   GND       │ ──────► GND
│   PO/Analog │ ──────► GPIO 34
└─────────────┘

BNC-Sonde im Wasser positionieren
```

**Hinweise:**
- GPIO 34 ist "Input Only" - perfekt für Analog-Sensoren
- Sonde regelmäßig kalibrieren (pH 4.0 und 7.0)
- Elektrode feucht halten (KCl-Lösung)
- Im fließenden Wasser für beste Ergebnisse

### 4. TDS-Sensor

```
TDS-Sensor Modul
┌─────────────┐
│   VCC       │ ──────► 3.3V oder 5V
│   GND       │ ──────► GND
│   Analog    │ ──────► GPIO 35
└─────────────┘

Sonde komplett ins Wasser tauchen
```

**Hinweise:**
- GPIO 35 ist "Input Only"
- Sonde vollständig eintauchen
- Elektroden sauber halten
- Kalibrierung mit 1413 µS/cm Lösung

### 5. Float Switches (Wasserlevel)

```
Float Switch 1 (Low Level)
┌─────────────┐
│   COM       │ ──────► 3.3V
│   NO/NC     │ ──────► GPIO 32
└─────────────┘
                │
              10kΩ Pull-down zu GND (optional)

Float Switch 2 (High Level)
┌─────────────┐
│   COM       │ ──────► 3.3V
│   NO/NC     │ ──────► GPIO 33
└─────────────┘
                │
              10kΩ Pull-down zu GND (optional)
```

**Hinweise:**
- Low Level: Warnung bei zu niedrigem Wasserstand
- High Level: Warnung bei Überlauf
- NO (Normally Open) verwenden
- Montage: Low tiefer als High

### 6. SD-Karten Modul

```
SD Card Module
┌─────────────┐
│   VCC       │ ──────► 3.3V (WICHTIG: Nicht 5V!)
│   GND       │ ──────► GND
│   MISO      │ ──────► GPIO 19
│   MOSI      │ ──────► GPIO 23
│   SCK       │ ──────► GPIO 18
│   CS        │ ──────► GPIO 25
└─────────────┘
```

**Hinweise:**
- Nur 3.3V! 5V zerstört das Modul
- SD-Karte FAT32 formatieren
- Max. 32GB Kapazität
- Class 10 für bessere Performance

### 7. Relay Modul (4-Kanal)

```
Relay Module
┌─────────────┐
│   VCC       │ ──────► 5V
│   GND       │ ──────► GND
│   IN1       │ ──────► GPIO 26 (Belüftung)
│   IN2       │ ──────► GPIO 27 (Reserve)
│   IN3       │ ──────► GPIO 14 (Reserve)
│   IN4       │ ──────► GPIO 12 (Reserve)
└─────────────┘

Relay 1 - Belüftungspumpe
┌─────────────┐
│   COM       │ ──────► 230V Phase (oder 12V+)
│   NO        │ ──────► Pumpe/Belüfter
└─────────────┘
```

**Hinweise:**
- Separate 5V Stromversorgung für Relays empfohlen
- Bei 230V: Elektriker hinzuziehen!
- Relays sind LOW-aktiv (invertiert)
- Optokoppler-Isolation vorhanden

## 🔋 Stromversorgung

### Option 1: USB-Versorgung (5V)
```
USB Netzteil 5V/3A
     │
     ├─────► ESP32 USB (empfohlen)
     │
     └─────► Relay Modul VCC (über Breadboard)
```

### Option 2: Separate Versorgung
```
5V/3A Netzteil ──────► ESP32 5V Pin
12V/2A Netzteil ─────► Relay Modul VCC (bei 12V Verbrauchern)
```

**Wichtig:**
- ESP32 benötigt min. 500mA
- Relay Modul: ca. 70mA pro Relay
- Sensoren: ca. 10-50mA
- **Gesamt: mind. 2A empfohlen, 3A sicher**

## 🏗️ Aufbau-Schritte

### Schritt 1: Breadboard-Layout
1. ESP32 in Mitte des Breadboards
2. Sensoren an den Seiten
3. Relay Modul separat (Störungen vermeiden)

### Schritt 2: Stromversorgung
1. 3.3V und 5V Rails verbinden
2. GND Rails verbinden
3. Alle GND zusammenführen (Common Ground!)

### Schritt 3: Sensoren anschließen
1. DS18B20 mit Pull-up Widerständen
2. Analog-Sensoren (pH, TDS)
3. Float Switches
4. SD-Karten Modul

### Schritt 4: Relays
1. Relay-Eingänge (IN1-IN4)
2. Relay-Ausgänge an Verbraucher

### Schritt 5: Test
1. Ohne Verbraucher testen
2. Seriellen Monitor beobachten
3. Sensoren einzeln prüfen

## 🔍 Troubleshooting

### DS18B20 nicht gefunden
- Pull-up Widerstand prüfen (4.7kΩ)
- Adresse mit Beispiel-Sketch auslesen
- Mehrere Sensoren: Adressen notieren

### pH/TDS zeigt 0 oder 4095
- VCC/GND vertauscht?
- Richtiger ADC-Pin? (34/35 sind Input-Only)
- Sensor defekt?

### SD-Karte nicht erkannt
- 3.3V verwenden, nicht 5V!
- FAT32 Format
- CS-Pin GPIO 25

### Relay schaltet nicht
- LOW-aktiv: invertierte Logik
- Separate 5V Versorgung
- LED am Relay leuchtet?

## 📊 Schaltplan

```
                    ForellenWächter Gesamtschaltplan
                    
    3.3V ────┬──────┬──────┬──────┬──────┬──────┐
             │      │      │      │      │      │
         DS18B20  DS18B20  Float  Float  SD    pH/TDS
         (Wasser) (Luft)   Sw1    Sw2   Card   Sensors
             │      │      │      │      │      │
    GPIO ────┴──────┴──────┴──────┴──────┴──────┘
    
    5V ──────┬──────┐
             │      │
          Relay   (Reserve)
          Module
             │
    230V ────┴───► Belüftung


    Alle GND zusammen führen!
```

## ✅ Checkliste vor Inbetriebnahme

- [ ] Alle Verbindungen doppelt prüfen
- [ ] 3.3V vs 5V korrekt?
- [ ] Alle GND verbunden?
- [ ] Pull-up Widerstände bei DS18B20
- [ ] SD-Karte eingesteckt und formatiert
- [ ] Relays ohne Last testen
- [ ] Wasserdichte Sensoren im Wasser
- [ ] Gehäuse wasserdicht
- [ ] Stromversorgung ausreichend
- [ ] Verkabelung gegen Kurzschluss prüfen

## 🎯 Tipps für die Praxis

### Installation am Teich
1. **Gehäuse**: IP65 oder besser
2. **Kabelführung**: Zugentlastung verwenden
3. **Sensoren**: Mit Kabelbindern fixieren
4. **Position**: Schattiger, trockener Ort
5. **Zugang**: Für Wartung erreichbar

### Wartung
- Sensoren monatlich reinigen
- Elektroden kalibrieren (quartalsweise)
- Connections auf Korrosion prüfen
- SD-Karte backup

---

Bei Fragen: Issues auf GitHub öffnen!

🔌 *Vorsicht ist besser als Nachsicht - besonders bei Wasser und Strom!*
