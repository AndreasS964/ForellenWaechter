# 🔄 OTA Updates - Anleitung

Over-The-Air Updates ermöglichen Firmware-Updates ohne physischen Zugriff auf das Gerät.

---

## ✨ Vorteile

- **Kein Kabel nötig:** Updates via WiFi
- **Remote-Updates:** Auch von unterwegs möglich
- **Zeitersparnis:** Keine Leiter, kein Gehäuse öffnen
- **Sicher:** Passwort-geschützt

---

## ⚙️ Konfiguration

### 1. Passwort ändern!

**WICHTIG:** Standard-Passwort UNBEDINGT ändern!

```cpp
// In der .ino Datei (Zeile ~70):
const char* OTA_PASSWORD = "DeinSicheresPasswort123!";  // ÄNDERN!
```

### 2. OTA aktivieren

```cpp
#define ENABLE_OTA true  // Standardmäßig aktiviert
```

### 3. WiFi erforderlich

OTA funktioniert nur mit aktiviertem WiFi:

```cpp
#define ENABLE_WIFI true
```

---

## 📡 OTA Update durchführen

### Methode 1: Arduino IDE (empfohlen für Einsteiger)

1. **ESP32 im gleichen Netzwerk**
   - Verbinde deinen Computer mit dem ForellenWächter WiFi ODER
   - Stelle sicher, dass ESP32 mit deinem Heimnetz verbunden ist

2. **Arduino IDE öffnen**
   - **Werkzeuge** → **Port**
   - Du siehst jetzt: `forellenwaechter at 192.168.4.1` (Netzwerk-Port!)

3. **Port auswählen**
   - Wähle den Netzwerk-Port (nicht USB!)

4. **Upload**
   - Klicke auf **Upload**
   - Passwort eingeben wenn gefragt
   - Warten bis "Upload complete"

**Fortschritt in Serial Monitor:**
```
🔄 OTA Update gestartet: Sketch
   Progress: 10%
   Progress: 20%
   ...
   Progress: 100%
✅ OTA Update abgeschlossen!
```

### Methode 2: PlatformIO

Füge zu `platformio.ini` hinzu:

```ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino

upload_protocol = espota
upload_port = 192.168.4.1  ; IP des ESP32
upload_flags =
    --port=3232
    --auth=DeinSicheresPasswort123!
```

Dann einfach:
```bash
pio run --target upload
```

### Methode 3: Python-Skript (espota.py)

```bash
python espota.py -i 192.168.4.1 -p 3232 \
  --auth=DeinSicheresPasswort123! \
  -f ForellenWaechter_v1.4_LTE.ino.bin
```

---

## 🔍 OTA-Status prüfen

### Via Serial Monitor

```
✅ OTA bereit!
```

Bedeutet: OTA ist aktiv und wartet auf Updates.

### Via API

```bash
curl http://192.168.4.1/api/status
```

```json
{
  "firmware": "1.4.1",
  "uptime": 86400,
  ...
}
```

---

## 🛡️ Sicherheit

### Passwort-Schutz

OTA ist **IMMER passwort-geschützt**. Ohne korrektes Passwort ist kein Upload möglich.

**Standard-Passwort:** `forellenadmin123`
**⚠️ BITTE ÄNDERN!**

### Firewall-Regeln (optional)

Falls ESP32 im Heimnetz:

```bash
# Nur von bestimmter IP erlauben
iptables -A INPUT -p tcp --dport 3232 -s 192.168.1.100 -j ACCEPT
iptables -A INPUT -p tcp --dport 3232 -j DROP
```

### HTTPS (geplant)

In v1.5 ist verschlüsseltes OTA geplant.

---

## ⚡ Was passiert beim Update?

1. **OTA-Modus aktiviert**
   - ESP32 stoppt normale Operationen
   - SD-Karte wird sicher beendet
   - Watchdog pausiert

2. **Firmware-Download**
   - Neue Firmware wird empfangen
   - Fortschritt: 0% → 100%
   - CRC-Check

3. **Flash-Schreiben**
   - Alte Firmware wird überschrieben
   - Partition wird verifiziert

4. **Neustart**
   - ESP32 startet automatisch neu
   - Neue Firmware läuft

**Dauer:** ~30 Sekunden (je nach Firmware-Größe)

---

## 🚨 Fehlerbehandlung

### "Auth Failed"

**Ursache:** Falsches Passwort

**Lösung:**
```cpp
// Passwort in Code prüfen
const char* OTA_PASSWORD = "...";
```

### "Connect Failed"

**Ursache:** ESP32 nicht erreichbar

**Lösung:**
```bash
# IP-Adresse prüfen
ping 192.168.4.1

# mDNS prüfen
ping forellenwaechter.local

# Serial Monitor: WiFi connected?
```

### "Begin Failed"

**Ursache:** Nicht genug Speicher / falsche Partition

**Lösung:**
- Partition Scheme: "Default 4MB with spiffs"
- Firmware-Größe < 1.5MB

### "Receive Failed"

**Ursache:** Netzwerk-Timeout, WiFi instabil

**Lösung:**
- Näher an Access Point
- WiFi-Signal prüfen (RSSI > -70 dBm)
- Erneut versuchen

### "End Failed"

**Ursache:** Flash-Schreiben fehlgeschlagen

**Lösung:**
- ESP32 neustarten (Power Cycle)
- Via USB-Kabel flashen (Fallback)
- ESP32 könnte defekt sein

---

## 💡 Best Practices

### Vor dem Update:

- [ ] **Backup der Kalibrierung** (siehe [CALIBRATION.md](CALIBRATION.md))
- [ ] **SD-Karte Daten sichern** (optional)
- [ ] **Stabile WiFi-Verbindung** (-60 dBm oder besser)
- [ ] **Aktuelle Firmware testen** (auf zweitem ESP32)

### Während dem Update:

- [ ] **Nicht unterbrechen!** (kein Power-Off)
- [ ] **Keine anderen Operationen** (kein API-Call)
- [ ] **Geduld haben** (~30 Sekunden)

### Nach dem Update:

- [ ] **Serial Monitor prüfen** (Boot-OK?)
- [ ] **Sensoren testen** (`/api/sensors`)
- [ ] **Kalibrierung prüfen** (`/api/calibration`)
- [ ] **Dashboard testen** (http://192.168.4.1)

---

## 🔧 Rollback (Downgrade)

Falls neue Firmware Probleme macht:

### 1. Alte Firmware flashen

```bash
# Via Arduino IDE: Alte .ino Datei öffnen und uploaden
```

### 2. Oder via USB-Kabel

```bash
esptool.py --port /dev/ttyUSB0 write_flash 0x10000 old_firmware.bin
```

### 3. Kalibrierung bleibt erhalten!

EEPROM wird NICHT überschrieben → Kalibrierung bleibt gespeichert.

---

## 📊 OTA vs. USB

| Kriterium | OTA | USB |
|-----------|-----|-----|
| **Geschwindigkeit** | ~30s | ~15s |
| **Bequemlichkeit** | ⭐⭐⭐⭐⭐ | ⭐⭐ |
| **Sicherheit** | Passwort | Physisch |
| **Zuverlässigkeit** | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| **Remote möglich** | ✅ Ja | ❌ Nein |
| **Failsafe** | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ |

**Empfehlung:**
- **Normale Updates:** OTA
- **Erste Installation:** USB
- **Notfall-Recovery:** USB

---

## 🌐 Remote-OTA (über Internet)

**WARNUNG:** Nur für fortgeschrittene Benutzer!

### Option 1: VPN

```bash
# WireGuard VPN einrichten
# ESP32 ist dann im VPN erreichbar
```

### Option 2: Port Forwarding

```bash
# Router: Port 3232 → ESP32 IP
# ⚠️ SICHERHEITSRISIKO! Nur mit starkem Passwort!
```

### Option 3: Reverse Proxy (sicher)

```nginx
# nginx config
location /ota {
    proxy_pass http://192.168.4.1:3232;
    auth_basic "OTA Access";
    auth_basic_user_file /etc/nginx/.htpasswd;
}
```

---

## 🚀 Automatische Updates (Zukunft)

In v1.6 geplant:

```cpp
// Automatisch nach Updates suchen
#define OTA_AUTO_UPDATE true
#define OTA_UPDATE_URL "https://releases.forellenwächter.de/latest.bin"
#define OTA_UPDATE_CHECK_INTERVAL 86400  // 24h
```

---

## 📚 Weitere Ressourcen

- [Arduino OTA Doku](https://arduino-esp32.readthedocs.io/en/latest/api/ota_updates.html)
- [ESP32 OTA Guide](https://randomnerdtutorials.com/esp32-over-the-air-ota-programming/)

---

**Letzte Aktualisierung:** 2024-12-01
