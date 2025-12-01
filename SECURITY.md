# 🔒 Sicherheitsrichtlinien

## Sensible Daten schützen

### ⚠️ NIEMALS committen:

- **Passwörter** (WiFi, API-Keys, etc.)
- **E-Mail-Adressen**
- **IFTTT Webhook-URLs** mit Keys
- **Persönliche Namen** oder Firmendaten
- **Telefonnummern**
- **GPS-Koordinaten** (falls verwendet)

### ✅ Best Practices:

1. **Nutze config.h**
   - Kopiere `config.example.h` → `config.h`
   - Trage DEINE Daten in `config.h` ein
   - `config.h` wird von `.gitignore` ignoriert!

2. **Ändere Standard-Passwörter**
   - WiFi-Passwort mindestens 12 Zeichen
   - Keine Namen oder Geburtsdaten verwenden
   - Nutze Passwort-Manager

3. **Webhook-URLs geheim halten**
   - IFTTT Keys sind wie Passwörter
   - Bei Leak: Sofort regenerieren!

4. **SD-Karte verschlüsseln** (optional)
   - Logs können sensible Daten enthalten
   - Bei Diebstahl geschützt

5. **HTTPS für Remote-Zugriff**
   - Bei öffentlicher IP: VPN oder Reverse Proxy nutzen
   - Keine offenen Ports ohne Verschlüsselung

## Schwachstellen melden

Sicherheitsprobleme bitte **NICHT** öffentlich als Issue melden!

Stattdessen:
- E-Mail an Repository-Maintainer
- Oder GitHub Security Advisory nutzen

## Update-Policy

- Regelmäßig Arduino ESP32 Core aktualisieren
- Bibliotheken auf Sicherheits-Updates prüfen
- Firmware-Updates über sichere Verbindung

## LTE/SIM-Karte Sicherheit

- **PIN deaktivieren** (ESP32 kann keine PIN eingeben)
- **PUK notieren** (sicher aufbewahren!)
- **Datenlimit setzen** (gegen Missbrauch)
- **Prepaid bevorzugen** (kein finanzielles Risiko)

## Netzwerk-Sicherheit

### Access Point Mode
- Starkes Passwort (min. 12 Zeichen)
- WPA2 aktiviert (Standard)
- SSID nicht verstecken (bringt keine Sicherheit)

### Station Mode
- Nur vertrauenswürdige Netzwerke
- Firewall-Regeln im Router setzen

## Physische Sicherheit

- **Gehäuse abschließen** (gegen Manipulation)
- **SD-Karte verschlüsseln** (bei sensiblen Daten)
- **Backup-Strategie** (Datenverlust vermeiden)

## Code-Sicherheit

### Input Validation
- API-Endpunkte validieren Eingaben
- Keine SQL-Injection möglich (kein SQL verwendet)
- XSS-Schutz im Dashboard (keine User-Inputs im HTML)

### Command Injection
- Keine Systemaufrufe mit User-Input
- Keine Shell-Befehle

## Datenschutz (DSGVO)

Falls das System persönliche Daten verarbeitet:

1. **Datenminimierung**
   - Nur nötige Daten sammeln
   - Keine IP-Adressen loggen (außer nötig)

2. **Speicherdauer**
   - Alte Logs regelmäßig löschen
   - SD-Karte Rotation (z.B. 30 Tage)

3. **Zugriffskontrolle**
   - Dashboard nur für berechtigte Personen
   - Bei Weitergabe: Daten anonymisieren

4. **Transparenz**
   - Dokumentieren, welche Daten gespeichert werden
   - Bei Kamera-Erweiterung: Datenschutzhinweis!

## Lizenz-Compliance

- MIT License erlaubt kommerzielle Nutzung
- Copyright-Hinweise müssen erhalten bleiben
- Keine Garantie oder Haftung (siehe LICENSE)

---

**Letzte Aktualisierung:** 2024-12-01
