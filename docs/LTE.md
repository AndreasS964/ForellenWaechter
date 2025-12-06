# 📡 LTE/4G Setup Guide

**ForellenWächter v1.6.2** - Remote-Zugriff über Mobilfunk

---

## 📋 Übersicht

Mit dem LTE-Modul kann der ForellenWächter von überall aus überwacht werden - perfekt für abgelegene Standorte ohne WiFi. Diese Anleitung beschreibt die Einrichtung für **v1.6.2**.

### Was wird benötigt?

| Komponente | Beschreibung | Preis (ca.) |
|------------|--------------|-------------|
| **SIM7600E/G Modul** | LTE Cat-1 Modul (SIMCOM) | 25-35€ |
| **SIM-Karte** | Daten-SIM (min. 100MB/Monat) | ab 3€/Monat |
| **LTE-Antenne** | 4G Antenne (empfohlen) | 5-10€ |

**Alternativen:**
- LILYGO T-SIM7600 (ESP32 + LTE integriert) - ~40€
- SIM800L (nur 2G, günstiger aber langsamer) - ~10€

---

## 🔌 Hardware-Anschluss

### Pin-Belegung (v1.6.2)

```
SIM7600E Modul    →    ESP32
─────────────────────────────
VCC (5V)          →    VIN (5V)
GND               →    GND
TXD               →    GPIO 16 (RX)
RXD               →    GPIO 17 (TX)
PWR_KEY           →    GPIO 25 (optional)
```

### Wichtige Hinweise

⚠️ **Spannungsversorgung:**
- SIM7600E benötigt **5V** und bis zu **2A** bei Sendeleistung
- ESP32 VIN-Pin verwenden, NICHT 3.3V!
- Empfohlen: Separates 5V Netzteil oder starker Step-Down (LM2596)

⚠️ **Logik-Pegel:**
- SIM7600E hat 3.3V Logik → direkt mit ESP32 kompatibel
- Keine Level-Shifter nötig

### Schaltplan

```
┌─────────────────┐
│  12V Batterie   │
└────────┬────────┘
         │
         ├──────────────────────────┐
         │                          │
         ▼                          ▼
  ┌──────────────┐          ┌──────────────┐
  │  Step-Down   │          │  Step-Down   │
  │  12V → 5V    │          │  12V → 5V    │
  │  (3A, ESP)   │          │  (3A, LTE)   │
  └──────┬───────┘          └──────┬───────┘
         │                          │
         ▼                          ▼
  ┌──────────────┐          ┌──────────────┐
  │    ESP32     │          │   SIM7600E   │
  │              │          │              │
  │  GPIO16 (RX)─┼──────────┤  TXD         │
  │  GPIO17 (TX)─┼──────────┤  RXD         │
  │  GPIO25 ─────┼──────────┤  PWR_KEY     │
  │  GND ────────┼──────────┤  GND         │
  │  VIN (5V) ───┼─────┬────┤  VCC         │
  │              │     │    │              │
  └──────────────┘     │    └──────────────┘
                       │
                       │
                  Antenne → SIM7600E ANT
```

---

## 📱 SIM-Karte vorbereiten

### 1. PIN deaktivieren ⚠️

**WICHTIG:** Die SIM-Karte darf **KEINE PIN** haben!

1. SIM in Handy einlegen
2. Einstellungen → SIM-Karte → PIN deaktivieren
3. SIM ins Modul einlegen

### 2. Provider & APN

**Empfohlene Provider (Deutschland):**

| Provider | APN | Notizen |
|----------|-----|---------|
| **1NCE** | `iot.1nce.net` | IoT-SIM, 500MB für 10 Jahre (10€ Flat) |
| **Telekom** | `internet.t-mobile` | Zuverlässig, beste Abdeckung |
| **Vodafone** | `web.vodafone.de` | Gute LTE-Geschwindigkeit |
| **O2** | `pinternet.interkom.de` | Günstiger, schwächere Abdeckung |
| **Congstar** | `internet` | Telekom-Netz, Prepaid flexibel |

**IoT-SIM Empfehlung:** 1NCE ist perfekt für ForellenWächter - einmalig 10€ für 500MB und 10 Jahre Laufzeit!

### 3. Datenverbrauch

**Geschätzter Verbrauch pro Monat:**

| Nutzung | MB/Monat |
|---------|----------|
| Dashboard alle 5 Min öffnen | ~50 MB |
| Telegram Alarme (10x/Monat) | ~1 MB |
| E-Mail Alarme (10x/Monat) | ~0.5 MB |
| **Gesamt** | **~50-100 MB** |

→ **100 MB/Monat** reichen völlig aus!

---

## ⚙️ Software-Konfiguration

### 1. APN im Code eintragen

Öffne `ForellenWaechter_v1.6.1_LTE.ino` und ändere:

```cpp
// --- LTE Konfiguration ---
#define LTE_APN "internet"           // Dein APN hier
#define LTE_USER ""                  // Meist leer
#define LTE_PASS ""                  // Meist leer
```

**Beispiele:**
```cpp
// Telekom
#define LTE_APN "internet.t-mobile"

// 1NCE IoT
#define LTE_APN "iot.1nce.net"

// Vodafone
#define LTE_APN "web.vodafone.de"
```

### 2. LTE aktivieren

```cpp
// --- Feature Toggles ---
#define ENABLE_LTE true              // LTE Mobilfunk
```

### 3. Code hochladen

- Board: **ESP32 Dev Module**
- Upload Speed: **921600**
- Port auswählen & hochladen

---

## 🔍 Verbindung testen

### 1. Serial Monitor öffnen

Einstellungen: **115200 Baud**

### 2. Erwartete Ausgabe

```
🐟 ForellenWächter v1.6.2 - Stable Edition
📡 LTE wird initialisiert...
   Modul: SIMCOM SIM7600E
   Firmware: SIM7600M22_V2.0
✅ LTE verbunden!
   Operator: Telekom.de
   Signal: 18/31 (gut)
   IP: 93.xxx.xxx.xxx
```

### 3. Signal-Qualität prüfen

**CSQ Werte:**

| CSQ | Signal | Status |
|-----|--------|--------|
| 0-9 | Sehr schwach | ❌ Verbindung instabil |
| 10-14 | Schwach | ⚠️ Funktioniert, aber langsam |
| 15-19 | Mittel | ✅ OK für Monitoring |
| 20-31 | Stark | ✅ Perfekt |

**Bei schwachem Signal:**
- Externe Antenne verwenden
- Modul an Fenster/höher platzieren
- Antennenkabel verlängern (max. 5m)

---

## 🌐 Remote-Zugriff einrichten

### Option 1: Direkte IP (öffentliche IP)

Viele Provider geben **keine** öffentliche IP mehr (Carrier-Grade NAT). Prüfen:

1. Im Dashboard unter "Verbindung" die öffentliche IP ablesen
2. Im Browser von Handy (mit mobilen Daten): `http://ÖFFENTLICHE_IP`
3. Funktioniert? → Du hast eine öffentliche IP! 🎉

**Falls nicht erreichbar:**
- Provider nutzt CGNAT (Carrier-Grade NAT)
- → Option 2 oder 3 verwenden

### Option 2: DynDNS (empfohlen)

Mit DynDNS bekommst du eine feste Domain wie `forelle.duckdns.org`:

1. **DuckDNS Account:** https://www.duckdns.org
2. **Domain erstellen:** z.B. `forelle`
3. **Token kopieren** (wird in v1.6.1 automatisch alle 10 Min aktualisiert)

Siehe: [DYNDNS_SETUP.md](DYNDNS_SETUP.md)

### Option 3: VPN (sicherste Variante)

Richte einen VPN-Server auf einem Raspberry Pi ein:

- **WireGuard** - Modern, schnell, einfach
- ESP32 verbindet sich zum VPN
- Zugriff über VPN von überall

Tutorial: [ROADMAP.md](ROADMAP.md) (geplant für v1.7)

---

## 🐛 Troubleshooting

### Problem: "LTE verbindet nicht"

**Checkliste:**

1. ✅ SIM-PIN deaktiviert?
   - SIM in Handy testen
   - PIN in Handy ausschalten

2. ✅ APN korrekt?
   - Provider-Website prüfen
   - Im Code korrekt eingetragen?

3. ✅ Stromversorgung ausreichend?
   - 5V mit min. 2A?
   - Bei schwacher Spannung: LTE schaltet ab

4. ✅ Antenne angeschlossen?
   - Externe Antenne verwenden!
   - Position optimieren

5. ✅ Netzabdeckung vorhanden?
   - Mit Handy (gleiche SIM) testen
   - CSQ im Serial Monitor prüfen

### Problem: "CSQ = 0" oder sehr schwach

**Lösungen:**

- **Externe Antenne verwenden** (bringt meist +10 CSQ)
- **Antenne mit Kabel verlängern** (SMA-Verlängerung, max. 5m)
- **Höher montieren** (1-2m Höhe hilft oft)
- **Fenster-Montage** (Indoor: nahe Fenster)
- **Provider wechseln** (Telekom hat beste Netzabdeckung)

### Problem: "Verbindung bricht ab"

**Ursachen & Fixes:**

- **Stromversorgung zu schwach:**
  - Separate 5V 3A Versorgung für LTE-Modul
  - Batterie mit mindestens 7Ah

- **Watchdog Timeout:**
  - LTE-Initialisierung kann 30-60s dauern
  - Watchdog-Timer erhöhen: `#define WATCHDOG_TIMEOUT 120`

- **Überhitzung:**
  - LTE-Modul mit Kühlkörper versehen
  - Gehäuse belüften

### Problem: "Modul antwortet nicht"

```
📡 LTE wird initialisiert...
   Timeout: Keine Antwort
```

**Lösungen:**

1. **Pin-Belegung prüfen:**
   - TX/RX vertauscht?
   - ESP32 GPIO 16 (RX) → SIM7600 TXD
   - ESP32 GPIO 17 (TX) → SIM7600 RXD

2. **Baud-Rate korrekt?**
   - Standard: 115200
   - Im Code: `LTESerial.begin(115200, SERIAL_8N1, LTE_RX, LTE_TX);`

3. **Power-Cycle:**
   - ESP32 + LTE-Modul komplett stromlos machen (10 Sek)
   - Dann neu starten

---

## 📊 Datenverbrauch optimieren

### Tipps für minimalen Verbrauch

1. **Dashboard weniger oft öffnen:**
   - Nur bei Bedarf
   - Telegram-Bot nutzen statt Dashboard

2. **Chart-Daten reduzieren:**
   - Nur 24h statt 7d anzeigen
   - Reduziert API-Antwort-Größe

3. **E-Mail Alarme minimieren:**
   - Cooldown auf 60 Min erhöhen
   - Nur bei kritischen Alarmen

4. **Bilder vermeiden:**
   - Dashboard ohne Hintergrundbilder

**Mit diesen Tipps:** ~30-50 MB/Monat

---

## 🔐 Sicherheit

### Wichtig bei öffentlicher IP

- **AP-Passwort ändern:** `const char* AP_PASSWORD = "DEIN_SICHERES_PASSWORT";`
- **OTA-Passwort ändern:** `const char* OTA_PASSWORD = "DEIN_OTA_PASSWORT";`
- **Keine Standard-Passwörter verwenden!**

### Empfehlungen

- **Firewall:** Nur Port 80 (HTTP) öffnen
- **HTTPS:** Für v1.7 geplant (mit Let's Encrypt)
- **VPN:** Sicherste Lösung (kein offener Port)

---

## 💰 Kosten-Beispiel

### Einmalige Kosten

| Komponente | Preis |
|------------|-------|
| SIM7600E Modul | 30€ |
| LTE Antenne | 8€ |
| **Gesamt** | **38€** |

### Laufende Kosten

**Option 1: 1NCE IoT-SIM**
- 10€ einmalig für 10 Jahre
- → **0,08€ / Monat** 🎉

**Option 2: Congstar Prepaid**
- 10€ für 1GB (3 Monate gültig)
- → **~3,30€ / Monat**

**Option 3: Telekom Datentarif**
- 5€ für 500MB / Monat
- → **5€ / Monat**

**Empfehlung:** 1NCE für Forellenzucht-Monitoring!

---

## 🔮 Nächste Schritte

Nach erfolgreicher LTE-Einrichtung:

1. **Telegram Bot einrichten:** [TELEGRAM_SETUP.md](TELEGRAM_SETUP.md)
2. **DynDNS konfigurieren:** [DYNDNS_SETUP.md](DYNDNS_SETUP.md)
3. **E-Mail Alarme:** Siehe README.md
4. **Sensoren kalibrieren:** [CALIBRATION.md](CALIBRATION.md)

---

## 📚 Weitere Infos

- **Hardware-Details:** [HARDWARE.md](HARDWARE.md)
- **API-Dokumentation:** [API.md](API.md)
- **Troubleshooting:** [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

---

*Made with ❤️ für abgelegene Forellenzuchten* 🐟📡
