# 📤 GitHub Upload Anleitung - ForellenWächter

## 🎯 Schritt-für-Schritt Anleitung

### Voraussetzungen

1. **GitHub Account** erstellen unter https://github.com
2. **Git installieren**:
   - **Windows**: https://git-scm.com/download/win
   - **macOS**: `brew install git` oder https://git-scm.com/download/mac
   - **Linux**: `sudo apt install git` (Ubuntu/Debian)

3. **Git konfigurieren** (einmalig):
```bash
git config --global user.name "Dein Name"
git config --global user.email "deine@email.com"
```

---

## 📁 Projektstruktur vorbereiten

Dein Projekt sollte so aussehen:

```
ForellenWaechter/
├── ForellenWaechter.ino       # Hauptcode
├── README.md                   # Dokumentation (erstellt)
├── LICENSE                     # MIT Lizenz (erstellt)
├── .gitignore                  # Ignorierliste (erstellt)
├── WIRING.md                   # Verkabelung (erstellt)
├── CHANGELOG.md                # Versionshistorie (optional)
├── images/                     # Screenshots & Fotos
│   ├── dashboard.png
│   ├── hardware.jpg
│   └── wiring_diagram.png
├── docs/                       # Zusätzliche Docs
│   ├── API.md
│   └── CALIBRATION.md
└── examples/                   # Beispiel-Sketches
    ├── sensor_test.ino
    └── wifi_setup.ino
```

---

## 🚀 Methode 1: GitHub Web Interface (Einfach)

### Schritt 1: Repository erstellen
1. Gehe zu https://github.com
2. Klicke auf **"+"** → **"New repository"**
3. Einstellungen:
   - **Name**: `ForellenWaechter` oder `trout-guardian`
   - **Description**: "ESP32-based aquaculture monitoring system"
   - **Public** oder **Private** (deine Wahl)
   - ✅ **Add README file** (NICHT ankreuzen - du hast schon eine!)
   - ✅ **Add .gitignore** (NICHT ankreuzen)
   - **License**: MIT (oder NICHT ankreuzen - du hast schon eine)
4. **Create repository** klicken

### Schritt 2: Dateien hochladen
1. Auf der Repository-Seite: **"Add file"** → **"Upload files"**
2. Alle Dateien per Drag & Drop hochziehen:
   - `README.md`
   - `LICENSE`
   - `.gitignore`
   - `WIRING.md`
   - `ForellenWaechter.ino`
   - Alle anderen Dateien
3. **Commit message**: "Initial commit - v1.2"
4. **Commit changes** klicken

✅ **Fertig!** Dein Code ist auf GitHub!

---

## 💻 Methode 2: Git Command Line (Professionell)

### Schritt 1: Repository auf GitHub erstellen
1. https://github.com → **"+"** → **"New repository"**
2. Name: `ForellenWaechter`
3. **Public/Private** wählen
4. **KEINE** README, .gitignore oder Lizenz hinzufügen
5. **Create repository** klicken

### Schritt 2: Lokales Repository initialisieren

**Terminal/CMD öffnen** und zu deinem Projektordner navigieren:

```bash
cd /pfad/zu/deinem/ForellenWaechter
```

**Git initialisieren:**
```bash
git init
```

### Schritt 3: Dateien hinzufügen

**Alle Dateien zum Repository hinzufügen:**
```bash
git add .
```

Oder **einzelne Dateien:**
```bash
git add README.md
git add LICENSE
git add .gitignore
git add WIRING.md
git add ForellenWaechter.ino
```

### Schritt 4: Ersten Commit erstellen

```bash
git commit -m "Initial commit - ForellenWächter v1.2"
```

### Schritt 5: Mit GitHub verbinden

**Remote Repository hinzufügen:**
```bash
git remote add origin https://github.com/DEIN-USERNAME/ForellenWaechter.git
```

**Ersetze `DEIN-USERNAME`** mit deinem GitHub Namen!

### Schritt 6: Code hochladen

**Branch umbenennen (falls nötig):**
```bash
git branch -M main
```

**Pushen:**
```bash
git push -u origin main
```

**Benutzername & Passwort eingeben** wenn gefragt.

✅ **Fertig!** Projekt ist online!

---

## 🔐 GitHub Authentication

### Option A: Personal Access Token (PAT) - Empfohlen

1. GitHub → **Settings** → **Developer settings** → **Personal access tokens** → **Tokens (classic)**
2. **Generate new token (classic)**
3. Name: "ForellenWaechter Development"
4. Scope auswählen: ✅ **repo** (voller Zugriff)
5. **Generate token** → **Token kopieren!**
6. Bei `git push` nach Passwort: **Token einfügen** (nicht sichtbar)

### Option B: SSH Key (Fortgeschritten)

```bash
# SSH Key generieren
ssh-keygen -t ed25519 -C "deine@email.com"

# Key zu GitHub hinzufügen
cat ~/.ssh/id_ed25519.pub
```

→ GitHub: **Settings** → **SSH and GPG keys** → **New SSH key**

Dann mit SSH-URL clonen:
```bash
git remote set-url origin git@github.com:DEIN-USERNAME/ForellenWaechter.git
```

---

## 📝 Weitere Commits (Änderungen hochladen)

Wenn du später Änderungen machst:

```bash
# Status prüfen
git status

# Änderungen hinzufügen
git add .

# Commit mit Nachricht
git commit -m "Update: Sensor-Kalibrierung verbessert"

# Hochladen
git push
```

---

## 🏷️ Releases erstellen

### Schritt 1: Tag erstellen
```bash
git tag -a v1.2 -m "Version 1.2 - Production Ready"
git push origin v1.2
```

### Schritt 2: Release auf GitHub
1. GitHub Repository → **Releases** → **Create a new release**
2. Tag auswählen: `v1.2`
3. Release title: "v1.2 - Production Ready"
4. Beschreibung:
```markdown
## 🎉 Version 1.2 - Production Ready

### Features
- ✅ SD-Karten Datenlogging
- ✅ 24h Historische Charts
- ✅ WebSocket Live-Updates
- ✅ Lucas Haug Branding

### Hardware Support
- ESP32 mit 4 Relays
- Wasser-/Lufttemperatur
- pH & TDS Sensoren
- Float Switches

### Installation
1. Arduino IDE installieren
2. Libraries installieren (siehe README)
3. Code hochladen
4. WiFi konfigurieren

**Getestet mit:** ESP32 Dev Module, Arduino IDE 2.x
```

5. Optional: **Attach binaries** (.bin Datei hochladen)
6. **Publish release**

---

## 📊 README auf GitHub verschönern

### Badges hinzufügen

Am Anfang deiner README.md:

```markdown
![Version](https://img.shields.io/badge/version-1.2-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![ESP32](https://img.shields.io/badge/platform-ESP32-red)
![Arduino](https://img.shields.io/badge/Arduino-IDE-00979D)
![Status](https://img.shields.io/badge/status-production-success)
```

### Screenshots einbinden

1. Screenshots in `images/` Ordner legen
2. In README einbinden:

```markdown
## 📱 Screenshots

### Dashboard
![Dashboard](images/dashboard.png)

### Hardware Setup
![Hardware](images/hardware.jpg)
```

---

## 🌿 Branches für Entwicklung

```bash
# Neuen Feature-Branch erstellen
git checkout -b feature/neue-funktion

# Änderungen machen, dann:
git add .
git commit -m "Feature: Neue Funktion hinzugefügt"
git push origin feature/neue-funktion

# Auf GitHub: Pull Request erstellen
# Nach Merge: Branch löschen
git checkout main
git pull
git branch -d feature/neue-funktion
```

---

## ❓ Häufige Probleme

### "Permission denied"
→ PAT verwenden oder SSH Key einrichten

### "Repository not found"
→ URL prüfen: `git remote -v`
→ Richtig ändern: `git remote set-url origin https://...`

### "Merge conflict"
```bash
git pull origin main
# Konflikte manuell lösen
git add .
git commit -m "Merge conflict resolved"
git push
```

### Letzten Commit rückgängig machen
```bash
git reset --soft HEAD~1
```

### Alles zurücksetzen (VORSICHT!)
```bash
git reset --hard HEAD
```

---

## 📋 Checkliste vor dem Upload

- [ ] Code kompiliert ohne Fehler
- [ ] Sensible Daten entfernt (WiFi-Passwörter!)
- [ ] README.md aktuell
- [ ] WIRING.md vollständig
- [ ] LICENSE vorhanden
- [ ] .gitignore vorhanden
- [ ] Screenshots hinzugefügt
- [ ] Beispiel-Sketches getestet
- [ ] Version-Tag gesetzt
- [ ] Commit-Messages aussagekräftig

---

## 🎓 Git Befehle Zusammenfassung

```bash
# Repository initialisieren
git init

# Status anzeigen
git status

# Alle Dateien hinzufügen
git add .

# Commit erstellen
git commit -m "Beschreibung"

# Remote hinzufügen
git remote add origin URL

# Pushen
git push -u origin main

# Pullen (Updates holen)
git pull

# Branch erstellen
git checkout -b branch-name

# Branch wechseln
git checkout main

# Log anzeigen
git log --oneline

# Änderungen anzeigen
git diff
```

---

## 🚀 Nächste Schritte nach Upload

1. **README auf GitHub prüfen** - sieht alles gut aus?
2. **Repository-Settings**:
   - Description hinzufügen
   - Topics hinzufügen: `esp32`, `iot`, `aquaculture`, `arduino`
   - Website-Link (falls vorhanden)
3. **Issues aktivieren** für Bug-Reports
4. **Wiki erstellen** für erweiterte Docs
5. **GitHub Pages** für Projekt-Website (optional)

---

## 💡 Tipps & Best Practices

### Gute Commit-Messages
```
✅ "Fix: pH-Sensor Kalibrierung korrigiert"
✅ "Feature: TDS-Alarm hinzugefügt"
✅ "Update: README mit Installation erweitert"
❌ "fixed stuff"
❌ "update"
❌ "asdf"
```

### Häufig committen
- Kleine, logische Änderungen
- Nicht alles auf einmal
- Mindestens täglich pushen

### Branch-Strategie
- `main` = stabile Version
- `develop` = aktive Entwicklung
- `feature/*` = neue Features
- `hotfix/*` = dringende Fixes

---

## 📞 Support

**Git lernen:**
- https://git-scm.com/doc
- https://try.github.io/
- https://learngitbranching.js.org/

**GitHub Docs:**
- https://docs.github.com

**Probleme?**
- GitHub Issues öffnen
- Stack Overflow

---

🎉 **Viel Erfolg mit deinem GitHub-Projekt!**

*Bei Fragen einfach melden!*
