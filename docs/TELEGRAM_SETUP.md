# 📱 Telegram Bot Setup

**ForellenWächter v1.6.1 - Fernsteuerung via Telegram**

---

## 🎯 Was bringt der Telegram Bot?

- **Push-Alarme**: Sofortige Benachrichtigung bei Problemen
- **Remote-Abfrage**: Status von überall checken
- **Befehle**: Relais schalten, Daten abfragen
- **Kostenlos**: Keine monatlichen Kosten!

---

## 📋 Voraussetzungen

1. **Telegram App** auf dem Handy installiert
2. **WiFi oder LTE Verbindung** am ESP32
3. **5 Minuten** Zeit für Setup

---

## 🔧 Setup-Anleitung

### Schritt 1: Telegram Bot erstellen

1. **Öffne Telegram** auf dem Handy
2. **Suche nach:** `@BotFather`
3. **Sende:**  `/newbot`
4. **Bot-Name wählen**: z.B. "ForellenWächter Monitor"
5. **Username wählen**: z.B. "ForellenWaechter_Bot" (muss auf "_bot" enden)
6. **Token kopieren**: Du bekommst einen Token wie:
   ```
   123456789:ABCdefGhIJKlmNoPQRsTUVwxyZ
   ```
   → **Speichern!** Das ist dein `TELEGRAM_BOT_TOKEN`

### Schritt 2: Chat-ID herausfinden

1. **Suche nach:** `@userinfobot`
2. **Sende:** `/start`
3. **Kopiere die ID**: Du bekommst etwas wie:
   ```
   Your ID: 987654321
   ```
   → **Speichern!** Das ist deine `TELEGRAM_CHAT_ID`

### Schritt 3: ESP32 konfigurieren

1. **Öffne:** `config.h` (oder erstelle sie aus `config.example.h`)
2. **Setze:**
   ```cpp
   #define ENABLE_TELEGRAM true

   const char* TELEGRAM_BOT_TOKEN = "123456789:ABCdefGhIJKlmNoPQRsTUVwxyZ";
   const char* TELEGRAM_CHAT_ID = "987654321";
   ```
3. **Installiere Library:**
   - Arduino IDE → Tools → Manage Libraries
   - Suche: "UniversalTelegramBot" by Brian Lough
   - Klick: Install
4. **Flashen & Testen**

### Schritt 4: Ersten Test

1. **Öffne Chat** mit deinem Bot in Telegram
2. **Sende:** `/start`
3. **Du solltest bekommen:**
   ```
   🐟 ForellenWächter Bot aktiv!

   Verfügbare Befehle:
   /status - Alle Sensordaten
   /temp - Temperaturen
   /power - Turbine & Batterie
   /alarm - Alarm-Status
   ```

---

## 📱 Verfügbare Befehle

| Befehl | Beschreibung | Beispiel |
|--------|--------------|----------|
| `/start` | Bot aktivieren | - |
| `/status` | Alle Sensordaten | Temp, pH, TDS, O₂, Flow, Batterie |
| `/temp` | Nur Temperaturen | Wasser & Luft |
| `/water` | Wasserqualität | pH, TDS, O₂ |
| `/power` | Turbine & Batterie | Flow (L/min), Power (W), Batterie (V, %) |
| `/alarm` | Alarm-Status | Aktuelle Alarme & Gründe |
| `/relay1` | Relais 1 schalten | Toggle An/Aus |
| `/relay2` | Relais 2 schalten | Toggle An/Aus |
| `/relay3` | Relais 3 schalten | Toggle An/Aus |
| `/relay4` | Relais 4 schalten | Belüftung An/Aus |

---

## 🚨 Push-Alarme

Der Bot sendet **automatisch** Nachrichten bei:

- ❄️ **Temperatur kritisch** (> 16°C oder < 8°C)
- 🧪 **pH außerhalb Bereich** (< 6.5 oder > 8.5)
- 💧 **Durchfluss zu niedrig** (< 5 L/min)
- 🔋 **Batterie niedrig** (< 11.5V)
- 🌊 **Wasserlevel niedrig**

**Cooldown:** 30 Minuten zwischen Alarmen (konfigurierbar)

---

## 🔒 Sicherheit

- **Token geheim halten!** Niemand anderes sollte deinen Bot-Token kennen
- **Chat-ID prüfen:** Bot antwortet nur auf deine Chat-ID
- **Kein Passwort nötig:** Telegram-Authentifizierung ist sicher

---

## 🐛 Troubleshooting

### "Bot antwortet nicht"

**Mögliche Ursachen:**
1. Token falsch → Prüfe Tippfehler
2. Chat-ID falsch → Nochmal bei @userinfobot checken
3. Keine Internet-Verbindung → WiFi/LTE prüfen
4. Library fehlt → UniversalTelegramBot installieren

**Check Serial Monitor:**
```
✅ Telegram Bot initialisiert
🤖 Bot prüft Nachrichten...
```

### "Certificate verify failed"

**Lösung:** In v1.6.1 ist `telegramClient.setInsecure()` gesetzt (für ESP32 ok).

### "Too many requests"

Telegram API Limit: 30 Nachrichten/Sekunde
- Bot fragt nur alle 2 Sekunden nach neuen Nachrichten
- Sollte nicht passieren

---

## 📚 Erweiterte Konfiguration

### Alarm-Cooldown ändern

```cpp
#define TELEGRAM_ALARM_COOLDOWN 1800000  // 30 Minuten in ms
```

### Check-Intervall ändern

```cpp
#define TELEGRAM_CHECK_INTERVAL 2000  // 2 Sekunden (Standard)
```

### Mehrere Chat-IDs

Aktuell: Nur 1 Chat-ID möglich.
Erweiterung: Array mit mehreren IDs möglich.

---

## 💡 Tipps

1. **Bot-Name**: Wähle einen eindeutigen Namen (z.B. "Forellenzucht_Waldbach_Bot")
2. **Notifications**: Telegram-Benachrichtigungen für diesen Chat aktivieren
3. **Gruppen**: Du kannst den Bot auch in einer Gruppe einsetzen
4. **Test-Modus**: Teste erst mit `TEST_MODE true` bevor du Hardware anschließt

---

## 🎬 Nächste Schritte

Nachdem Telegram läuft:
- [DynDNS Setup](DYNDNS_SETUP.md) → Dashboard von überall erreichen
- [Hardware Setup](HARDWARE.md) → Turbine & Sensoren anschließen

---

**Support:** [GitHub Issues](https://github.com/AndreasS964/ForellenWaechter/issues)
