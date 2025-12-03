# 🔌 Hardware-Dokumentation

Vollständige Hardware-Anleitung für den ForellenWächter v1.6 Turbine & Power Edition.

---

## Inhaltsverzeichnis

1. [Einkaufsliste](#einkaufsliste)
2. [Pinbelegung](#pinbelegung)
3. [Schaltplan](#schaltplan)
4. [Verdrahtung im Detail](#verdrahtung-im-detail)
5. [Stromversorgung](#stromversorgung)
6. [Gehäuse](#gehäuse)

---

## Einkaufsliste

### Hauptkomponenten

| Komponente | Menge | Bezugsquelle |
|------------|-------|--------------|
| ESP32 DevKit V1 | 1 | Amazon/AliExpress |
| DS18B20 wasserdicht (3m Kabel) | 2 | Amazon |
| pH-Sensor Modul | 1 | Amazon |
| TDS-Sensor Modul | 1 | Amazon |
| Schwimmschalter | 1 | Amazon |
| 4-Kanal Relais Modul (5V) | 1 | Amazon |
| microSD-Karten Modul | 1 | Amazon |
| microSD Karte (8-32GB) | 1 | Amazon |
| Step-Down Converter (12V→5V) | 1 | Amazon |
| 12V Lithium Batterie (30Ah) | 1 | Amazon/Elektronikfachhandel |

### Kleinteile

| Teil | Menge | Hinweis |
|------|-------|---------|
| Widerstand 4.7kΩ | 1 | Pullup für DS18B20 |
| Widerstand 10kΩ | 1 | Spannungsteiler |
| Widerstand 3.3kΩ | 1 | Spannungsteiler |
| Jumper-Kabel | ~30 | Male-Female, Male-Male |
| Lötkolben + Lötzinn | 1 | Für sichere Verbindungen |
| Schrumpfschlauch | div. | Isolierung |
| Kabelbinder | div. | Kabelmanagement |

### Gehäuse

| Teil | Spezifikation |
|------|---------------|
| Gehäuse | IP65+ wasserdicht, mind. 200x150x100mm |
| Kabelverschraubungen | M16 oder M20, wasserdicht |
| Silikondichtungen | Für Sensorkabel-Durchführungen |

---

## Pinbelegung

### ESP32 DevKit V1 - Verwendete Pins

```
┌────────────────────────────────────────────────────────────────┐
│                        ESP32 DevKit V1                          │
│                                                                  │
│   3.3V ●────────────────────────────────────────────● VIN (5V)  │
│   GND  ●────────────────────────────────────────────● GND       │
│   GPIO15 ●──── Buzzer                             ● GPIO13 ────● Alarm LED
│   GPIO2  ●                                        ● GPIO12      │
│   GPIO4  ●──── DS18B20 DATA                       ● GPIO14      │
│   GPIO16 ●──── LTE RX                             ● GPIO27      │
│   GPIO17 ●──── LTE TX                             ● GPIO26 ────● RELAY 4 (Belüftung)
│   GPIO5  ●──── SD CS                              ● GPIO25 ────● RELAY 3 (Reserve 2)
│   GPIO18 ●──── SD CLK                             ● GPIO33 ────● RELAY 2 (Reserve 1)
│   GPIO19 ●──── SD MISO                            ● GPIO32 ────● RELAY 1 (Alarm)
│   GPIO21 ●                                        ● GPIO35 ────● TDS SENSOR
│   GPIO3  ●                                        ● GPIO34 ────● pH SENSOR
│   GPIO1  ●                                        ● GPIO36 ────● DO SENSOR
│   GPIO22 ●                                        ● GPIO39 ────● WATER LEVEL
│   GPIO23 ●──── STATUS LED / SD MOSI               ● EN         │
│                                                                  │
└────────────────────────────────────────────────────────────────┘
```

### Pin-Übersicht Tabelle (v1.5 LTE)

| Pin | Funktion | Richtung | Hinweis |
|-----|----------|----------|---------|
| GPIO 4 | DS18B20 Data | Digital I/O | + 4.7kΩ Pullup nach 3.3V |
| GPIO 34 | pH Sensor | Analog In | ADC1, 0-3.3V |
| GPIO 35 | TDS Sensor | Analog In | ADC1, 0-3.3V |
| GPIO 36 | DO Sensor | Analog In | ADC1, 0-3.3V (optional) |
| GPIO 39 | Wasserstand | Digital In | INPUT_PULLUP |
| GPIO 32 | Relais 1 | Digital Out | LOW = EIN (Alarm) |
| GPIO 33 | Relais 2 | Digital Out | LOW = EIN (Reserve 1) |
| GPIO 25 | Relais 3 | Digital Out | LOW = EIN (Reserve 2) |
| GPIO 26 | Relais 4 | Digital Out | LOW = EIN (Belüftung) |
| GPIO 23 | Status LED | Digital Out | Onboard oder extern |
| GPIO 13 | Alarm LED | Digital Out | Externe rote LED |
| GPIO 15 | Buzzer | Digital Out | Piezo-Buzzer |
| GPIO 5 | SD Card CS | Digital Out | SPI |
| GPIO 18 | SD Card CLK | Digital Out | SPI |
| GPIO 19 | SD Card MISO | Digital In | SPI |
| GPIO 23 | SD Card MOSI | Digital Out | SPI (shared mit Status LED) |
| GPIO 16 | LTE RX | Serial | HardwareSerial(1) |
| GPIO 17 | LTE TX | Serial | HardwareSerial(1) |

---

## Schaltplan

### Übersichts-Schaltplan (ASCII)

```
                              ┌─────────────────┐
                              │   12V BATTERIE  │
                              │     (30Ah)      │
                              └────────┬────────┘
                                       │
                    ┌──────────────────┼──────────────────┐
                    │                  │                  │
                    ▼                  ▼                  ▼
            ┌──────────────┐   ┌──────────────┐   ┌──────────────┐
            │  Step-Down   │   │  [R1: 10kΩ]  │   │   [R3: 10kΩ] │
            │  12V → 5V    │   │      │       │   │      │       │
            └──────┬───────┘   │      ├───────┼───┤      │       │
                   │           │      │ GPIO36│   │      │       │
                   ▼           │  [R2: 3.3kΩ] │   │  [R4: 3.3kΩ] │
            ┌──────────────┐   │      │       │   │      │       │
            │    ESP32     │   └──────┴───────┘   └──────┴───────┘
            │              │           │                  │
            │  5V  ──●─────┼───────────┴──────────────────┘
            │  GND ──●─────┼──────────────────────────────────────
            │              │
            └──────────────┘


╔══════════════════════════════════════════════════════════════════════════╗
║                            HAUPTSCHALTPLAN                                ║
╠══════════════════════════════════════════════════════════════════════════╣
║                                                                           ║
║   DS18B20 TEMPERATURSENSOREN                                             ║
║   ─────────────────────────                                               ║
║                                                                           ║
║   DS18B20 #1 (Wasser)        DS18B20 #2 (Luft)                           ║
║   ┌─────────┐                ┌─────────┐                                 ║
║   │ VCC (R) ├────┬───────────┤ VCC (R) ├──────────── 3.3V ESP32          ║
║   │ DATA(Y) ├────┼───────────┤ DATA(Y) ├────┬─────── GPIO 4              ║
║   │ GND (B) ├────┼───────────┤ GND (B) ├────┼─────── GND                 ║
║   └─────────┘    │           └─────────┘    │                            ║
║                  │                          │                            ║
║                  └──────── [4.7kΩ] ─────────┘                            ║
║                          Pullup nach 3.3V                                ║
║                                                                           ║
║                                                                           ║
║   pH SENSOR                                                               ║
║   ─────────                                                               ║
║   ┌─────────────┐                                                        ║
║   │ pH-Modul    │                                                        ║
║   │             │                                                        ║
║   │ VCC   ──────┼──────────────────────────── 5V ESP32                   ║
║   │ GND   ──────┼──────────────────────────── GND                        ║
║   │ Signal ─────┼──────────────────────────── GPIO 34                    ║
║   │ (BNC-Buchse │← pH-Sonde                                              ║
║   └─────────────┘                                                        ║
║                                                                           ║
║                                                                           ║
║   TDS SENSOR                                                              ║
║   ──────────                                                              ║
║   ┌─────────────┐                                                        ║
║   │ TDS-Modul   │                                                        ║
║   │             │                                                        ║
║   │ VCC   ──────┼──────────────────────────── 3.3V-5V                    ║
║   │ GND   ──────┼──────────────────────────── GND                        ║
║   │ Signal ─────┼──────────────────────────── GPIO 35                    ║
║   │ (Sonde) ────┼← TDS-Elektrode                                         ║
║   └─────────────┘                                                        ║
║                                                                           ║
║                                                                           ║
║   SCHWIMMSCHALTER (Wasserstand)                                          ║
║   ─────────────────────────────                                           ║
║                                                                           ║
║   ┌─────────┐                                                            ║
║   │ Schwimm-│                                                            ║
║   │ schalter│                                                            ║
║   │         │                                                            ║
║   │ Pin 1 ──┼──────────────────────────────── GPIO 25                    ║
║   │ Pin 2 ──┼──────────────────────────────── GND                        ║
║   └─────────┘                                                            ║
║              (INPUT_PULLUP im Code aktiviert)                            ║
║                                                                           ║
║                                                                           ║
║   BATTERIE-MESSUNG (Spannungsteiler)                                     ║
║   ──────────────────────────────────                                      ║
║                                                                           ║
║   Batterie 12V ────[R1: 10kΩ]────┬────[R2: 3.3kΩ]──── GND               ║
║                                  │                                        ║
║                                  └────────────────── GPIO 36              ║
║                                                                           ║
║   WICHTIG: GPIO 36 verträgt max 3.3V!                                    ║
║   Bei 12.6V Batterie → 3.12V am GPIO (OK)                                ║
║   Faktor: (10 + 3.3) / 3.3 = 4.03                                        ║
║                                                                           ║
║                                                                           ║
║   4-KANAL RELAIS MODUL                                                   ║
║   ────────────────────                                                    ║
║   ┌─────────────────────────────────────┐                                ║
║   │          RELAIS MODUL               │                                ║
║   │                                     │                                ║
║   │  VCC ────────────────────────────── 5V ESP32                         ║
║   │  GND ────────────────────────────── GND                              ║
║   │  IN1 ────────────────────────────── GPIO 26 (Belüftung)              ║
║   │  IN2 ────────────────────────────── GPIO 27                          ║
║   │  IN3 ────────────────────────────── GPIO 14                          ║
║   │  IN4 ────────────────────────────── GPIO 12                          ║
║   │                                     │                                ║
║   │  [NO1] [COM1] [NC1]  ← Relais 1     │                                ║
║   │  [NO2] [COM2] [NC2]  ← Relais 2     │                                ║
║   │  [NO3] [COM3] [NC3]  ← Relais 3     │                                ║
║   │  [NO4] [COM4] [NC4]  ← Relais 4     │                                ║
║   └─────────────────────────────────────┘                                ║
║                                                                           ║
║   LOW-LEVEL-TRIGGER: LOW = EIN, HIGH = AUS                               ║
║                                                                           ║
║                                                                           ║
║   SD-KARTEN MODUL (SPI)                                                  ║
║   ─────────────────────                                                   ║
║   ┌─────────────┐                                                        ║
║   │ SD-Modul    │                                                        ║
║   │             │                                                        ║
║   │ VCC   ──────┼──────────────────────────── 5V (oder 3.3V)             ║
║   │ GND   ──────┼──────────────────────────── GND                        ║
║   │ CS    ──────┼──────────────────────────── GPIO 5                     ║
║   │ MOSI  ──────┼──────────────────────────── GPIO 23                    ║
║   │ MISO  ──────┼──────────────────────────── GPIO 19                    ║
║   │ SCK   ──────┼──────────────────────────── GPIO 18                    ║
║   └─────────────┘                                                        ║
║                                                                           ║
╚══════════════════════════════════════════════════════════════════════════╝
```

---

## Verdrahtung im Detail

### DS18B20 Temperatursensoren

**Farbcode (Standard):**
- 🔴 Rot = VCC (3.3V)
- 🟡 Gelb = DATA
- ⚫ Schwarz = GND

**Wichtig:** 4.7kΩ Pullup-Widerstand zwischen DATA und VCC!

```
3.3V ────┬──── DS18B20 VCC (beide)
         │
      [4.7kΩ]
         │
GPIO 4 ──┴──── DS18B20 DATA (beide parallel)

GND ───────── DS18B20 GND (beide)
```

### Spannungsteiler für Batterie

**KRITISCH:** GPIO 36 verträgt nur 0-3.3V!

```
Batterie (+) ────[10kΩ]────┬────[3.3kΩ]──── GND
                           │
                           └───────────── GPIO 36
```

**Berechnung:**
- Vout = Vin × R2 / (R1 + R2)
- Bei 12.6V: 12.6 × 3.3 / 13.3 = 3.13V ✓
- Bei 14V: 14 × 3.3 / 13.3 = 3.47V ⚠️ (knapp!)

---

## Stromversorgung

### Empfohlene Konfiguration

```
┌─────────────────┐
│  12V Lithium    │
│  Batterie 30Ah  │
└────────┬────────┘
         │
         ├─────────────────────────────┐
         │                             │
         ▼                             ▼
┌─────────────────┐           ┌─────────────────┐
│   Step-Down     │           │  Spannungs-     │
│   12V → 5V      │           │  teiler zur     │
│   (z.B. LM2596) │           │  Messung        │
└────────┬────────┘           └─────────────────┘
         │
         ▼
    ESP32 5V Pin
```

### Batterie-Laufzeit

| Modus | Stromverbrauch | 30Ah Batterie |
|-------|----------------|---------------|
| 24/7 (WiFi an) | ~320 mA | ~4 Tage |
| Deep Sleep | ~5 mA (avg) | ~8 Monate |

---

## Gehäuse

### Anforderungen

- **IP65** oder besser (wasserdicht)
- Mindestgröße: 200 × 150 × 100 mm
- Material: ABS oder Polycarbonat

### Kabeldurchführungen

- Verwende wasserdichte Kabelverschraubungen (M16/M20)
- Sensorkabel mit Silikondichtung abdichten
- Entlüftungsventil gegen Kondenswasser (optional)

### Wärmemanagement

- Bei direkter Sonneneinstrahlung: weiß/helles Gehäuse
- Oder: Sonnenschutz-Dach
- Belüftungsöffnungen mit Labyrinth-Dichtung

---

## Tipps & Best Practices

### Kabellängen

| Sensor | Max. Kabellänge | Hinweis |
|--------|-----------------|---------|
| DS18B20 | 10-20m | Mit 4.7kΩ Pullup |
| pH-Sensor | 2-5m | BNC-Verlängerung |
| TDS-Sensor | 2-3m | Original-Kabel |
| Schwimmschalter | 10m+ | Einfaches Schaltsignal |

### Erdung

- Alle GND-Leitungen sternförmig verbinden
- Separate Erdung für Relais-Modul (Störimpulse!)

### EMV-Schutz

- Sensorkabel geschirmt verlegen (wenn möglich)
- Relais-Leitungen von Sensorkabeln trennen
- Ferritringe an Zuleitungen (optional)

---

## NEU in v1.6: Wasserturbine & Batterie-Monitoring

### Zusätzliche Hardware für v1.6

| Komponente | Spezifikation | Bezugsquelle | Preis (ca.) |
|------------|---------------|--------------|-------------|
| **Mini Wasserturbine** | 12V DC, 10W, mit Hall-Sensor | Amazon | ~15€ |
| **Laderegler LM2596** | Step-Down mit Display, 4-40V → 1.25-37V | Amazon | ~8€ |
| **Blei-Gel Batterie** | 12V, 7-20Ah | Elektronikfachhandel | 20-50€ |
| **Spannungsteiler** | 10kΩ + 3.3kΩ Widerstände | Amazon/Reichelt | ~1€ |

### Empfohlene Produkte

**Wasserturbine:**
- **"Wasserkraft Generator 12V DC 10W"** (Amazon)
- Startdruck: 0.05 MPa
- Durchfluss: ~90 L/h (1.5 L/min)
- Hall-Sensor: Ja (Pulsmessung)
- Anschlüsse: 1/2" Gewinde

**Laderegler:**
- **AZ-Delivery LM2596 Step-Down** oder ähnlich
- Eingangsspannung: 4-40V
- Ausgangsspannung: einstellbar (auf 13.8V einstellen!)
- LED-Anzeige für Spannung

### Schaltplan Stromversorgung v1.6

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         STROMVERSORGUNG v1.6                             │
│                                                                           │
│  Wasserdurchfluss                                                         │
│       ↓                                                                   │
│  ┌─────────────┐                                                         │
│  │ Turbine     │                                                         │
│  │  12V, 10W   │───┐                                                     │
│  │ Hall-Sensor │   │ 12V DC                                              │
│  └─────────────┘   │                                                     │
│       │            │                                                     │
│  (GPIO 2) ←────────┤                                                     │
│  Flow Sensor       │                                                     │
│                    ↓                                                     │
│              ┌──────────────┐                                            │
│              │  LM2596      │                                            │
│              │  Laderegler  │                                            │
│              │  In: 12V     │                                            │
│              │  Out: 13.8V  │                                            │
│              └──────────────┘                                            │
│                    │                                                     │
│                    ↓ 13.8V (Ladespannung)                                │
│              ┌──────────────┐                                            │
│              │  Pb/Gel Akku │                                            │
│              │  12V, 7-20Ah │                                            │
│              └──────────────┘                                            │
│                    │                                                     │
│                    ├─────────────┐ (Spannungsmessung)                    │
│                    │             │                                       │
│                    │        ┌────┴────┐                                  │
│                    │        │  10kΩ   │                                  │
│                    │        └────┬────┘                                  │
│                    │             │                                       │
│                    │        ┌────┴────┐                                  │
│                    │        │  3.3kΩ  │──→ GPIO 36 (ADC, max 3.3V)      │
│                    │        └────┬────┘                                  │
│                    │             │                                       │
│                    │            GND                                      │
│                    │                                                     │
│                    ↓ 12V                                                 │
│              ┌──────────────┐                                            │
│              │ Step-Down    │                                            │
│              │ 12V → 5V     │                                            │
│              │ (für ESP32)  │                                            │
│              └──────────────┘                                            │
│                    │ 5V                                                  │
│                    ↓                                                     │
│              ┌──────────────┐                                            │
│              │  ESP32 VIN   │                                            │
│              └──────────────┘                                            │
│                                                                           │
└───────────────────────────────────────────────────────────────────────────┘
```

### Verdrahtung Turbine & Batterie

#### 1. Wasserturbine anschließen

**Turbinen-Hauptanschlüsse:**
```
Rot (+)   →  LM2596 IN+
Schwarz (-) →  LM2596 IN- & GND
```

**Hall-Sensor (Flow-Messung):**
```
Turbine Signal-Pin  →  GPIO 2 (mit 10kΩ Pullup nach 3.3V)
Turbine GND         →  ESP32 GND
```

#### 2. Laderegler LM2596 konfigurieren

1. **VOR dem Anschluss der Batterie:** Ausgangsspannung einstellen
2. Multimeter an OUT+ und OUT- anschließen
3. Poti drehen bis Display **13.8V** anzeigt (Ladespannung für Pb/Gel)
4. Erst dann Batterie anschließen!

#### 3. Spannungsteiler für Batterie-Monitoring

**Schaltung:**
```
Batterie +12V ────┬──→ (zur Versorgung)
                  │
                 10kΩ  (R1)
                  │
                  ├────→ GPIO 36 (max 3.3V!)
                  │
                3.3kΩ  (R2)
                  │
                 GND
```

**Berechnung:**
- V_out = V_in × R2 / (R1 + R2)
- Bei 12.6V: V_out = 12.6 × 3.3 / 13.3 = 3.13V ✓ (sicher unter 3.3V!)
- Bei 13.8V: V_out = 13.8 × 3.3 / 13.3 = 3.42V ⚠️ (knapp über 3.3V!)

**Alternative für mehr Sicherheit:** 10kΩ + 2.2kΩ (max 3.0V bei 13.8V)

### Erweiterte Pin-Tabelle v1.6

| Pin | Funktion | v1.5 | v1.6 | Hinweis |
|-----|----------|------|------|---------|
| GPIO 2 | Flow Sensor | - | ✅ | Hall-Sensor Turbine, Interrupt |
| GPIO 27 | DO Sensor | - | ✅ | DO-Sensor verschoben (war GPIO 36) |
| GPIO 36 | ADC | DO | Battery | Batterie-Spannung (Spannungsteiler) |

### Kalibrierung

#### Flow-Sensor Kalibrierung

1. **Durchfluss messen:** Mit externem Durchflussmesser 1 Minute lang messen
2. **Impulse zählen:** Serial Monitor zeigt `turbinePulseCount` (nach 1 min notieren)
3. **Faktor berechnen:**
   ```
   TURBINE_PULSES_PER_LITER = pulseCount / gemessene_Liter
   ```
4. In `config.h` eintragen und neu flashen

**Beispiel:**
- Gemessen: 1.5 Liter in 1 Minute
- Impulse: 675
- Faktor: 675 / 1.5 = **450 Impulse/Liter**

#### Batterie-Kalibrierung

1. **Batterie voll laden** (bis LM2596 auf 13.8V geht)
2. **Spannung mit Multimeter messen** (z.B. 13.2V)
3. **Serial Monitor prüfen:** Zeigt ESP32 die gleiche Spannung?
4. Falls nicht: Widerstandswerte im Spannungsteiler nachmessen und in `config.h` korrigieren

---

Nächste Schritte: [KONFIGURATION.md](KONFIGURATION.md)
