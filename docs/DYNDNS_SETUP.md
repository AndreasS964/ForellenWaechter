# 🌐 DynDNS Setup - Dashboard von überall erreichen

**ForellenWächter v1.6.1 - DuckDNS Integration**

---

## 🎯 Was bringt DynDNS?

- **Dashboard von überall**: `http://forellenwaechter.duckdns.org`
- **Keine monatlichen Kosten**: DuckDNS ist kostenlos
- **Automatische Updates**: ESP32 aktualisiert IP alle 5 Minuten
- **Einfach**: Keine DNS-Kenntnisse nötig

---

## 📋 Voraussetzungen

1. **Öffentliche IP-Adresse** vom Internet-Provider
2. **Router mit Port-Forwarding** (oder LTE-Router)
3. **DuckDNS Account** (kostenlos, 2 Minuten Setup)

---

## ⚠️ Wichtiger Hinweis: CG-NAT Problem

**Viele Mobilfunkanbieter nutzen CG-NAT** (Carrier-Grade NAT):
- Du bekommst KEINE öffentliche IP
- DynDNS funktioniert dann NICHT
- **Alternative:** Telegram Bot (funktioniert immer!)

**Prüfe deine IP:**
1. Gehe zu: https://www.wieistmeineip.de
2. Vergleiche mit deiner Router-IP
3. Unterschiedlich? → CG-NAT aktiv → DynDNS geht nicht

**Lösung bei CG-NAT:**
- Telegram Bot nutzen (immer kostenlos)
- VPN/Tunnel (Tailscale, ngrok)
- Provider anrufen → "Echte öffentliche IP" buchen (5-15€/Monat)

---

## 🔧 Setup-Anleitung

### Schritt 1: DuckDNS Account erstellen

1. **Gehe zu:** https://www.duckdns.org
2. **Login mit:** GitHub, Google, Twitter oder Reddit
3. **Subdomain erstellen:**
   - Subdomain: z.B. "forellenwaechter"
   - Volle URL: `forellenwaechter.duckdns.org`
4. **Token kopieren:**
   - Oben rechts: Dein Token angezeigt
   - Format: `abc12345-def6-7890-ghij-klmnopqrstuv`
   - **Speichern!**

### Schritt 2: ESP32 konfigurieren

1. **Öffne:** `config.h`
2. **Setze:**
   ```cpp
   #define ENABLE_DYNDNS true

   const char* DYNDNS_DOMAIN = "forellenwaechter.duckdns.org";
   const char* DYNDNS_TOKEN = "abc12345-def6-7890-ghij-klmnopqrstuv";
   ```
3. **Flashen**

### Schritt 3: Port-Forwarding im Router einrichten

**Beispiel Fritzbox:**
1. Gehe zu: http://fritz.box
2. Internet → Freigaben → Portfreigabe
3. Neue Portfreigabe:
   - Gerät: ESP32 (nach IP-Adresse suchen)
   - Anwendung: HTTP-Server
   - Port: 80
   - Protokoll: TCP
   - An Gerät: ESP32 IP-Adresse

**Beispiel Vodafone GigaCube / Telekom Speedbox:**
1. Router-Interface öffnen (siehe Handbuch)
2. NAT/Port-Forwarding
3. Regel erstellen:
   - Extern: Port 80
   - Intern: ESP32-IP, Port 80

**LTE-Router ohne Port-Forwarding?**
- → Telegram Bot nutzen (Push-Benachrichtigungen)
- → Oder VPN/Tunnel-Lösung

### Schritt 4: Testen

1. **Warte 5 Minuten** (erste DynDNS-Update)
2. **Prüfe Serial Monitor:**
   ```
   ✅ DynDNS Update erfolgreich
   🌐 Domain: forellenwaechter.duckdns.org
   ```
3. **Teste im Browser:**
   - Von außerhalb deines Heimnetzes
   - z.B. mit Handy (mobile Daten, kein WiFi!)
   - Gehe zu: `http://forellenwaechter.duckdns.org`

4. **Dashboard sollte laden!** 🎉

---

## 🔒 Sicherheit

### Basic Auth (empfohlen!)

DynDNS macht dein Dashboard **öffentlich erreichbar!**

**Schützen mit Passwort:**
```cpp
// In setup() hinzufügen:
server.on("/", HTTP_GET, []() {
  if (!server.authenticate("admin", "DeinPasswort123")) {
    return server.requestAuthentication();
  }
  handleRoot();
});
```

### HTTPS (optional)

HTTP ist unverschlüsselt. Für sensible Daten:
- Let's Encrypt Zertifikat (komplex für ESP32)
- Oder: Reverse Proxy (Nginx, Caddy)
- Oder: Cloudflare Tunnel (kostenlos, einfach)

### IP Whitelist (optional)

Nur von bestimmten IPs erreichbar:
```cpp
String clientIP = server.client().remoteIP().toString();
if (clientIP != "123.45.67.89") {
  server.send(403, "text/plain", "Forbidden");
  return;
}
```

---

## 📊 DynDNS Update-Prozess

**Wie funktioniert es?**

1. ESP32 holt seine öffentliche IP (via http://api.ipify.org)
2. ESP32 sendet Update an DuckDNS:
   ```
   https://www.duckdns.org/update?domains=forellenwaechter&token=abc123&ip=123.45.67.89
   ```
3. DuckDNS aktualisiert DNS-Eintrag
4. Deine Domain zeigt auf deine IP

**Update-Intervall:**
- Standard: 5 Minuten
- Änderbar: `DYNDNS_UPDATE_INTERVAL`

---

## 🐛 Troubleshooting

### "Domain nicht erreichbar"

**Checken:**
1. **Port-Forwarding aktiv?**
   - Test: `telnet deine-domain.duckdns.org 80`
   - Sollte verbinden

2. **ESP32 online?**
   - Lokaler Zugriff funktioniert?
   - `http://[ESP32-IP]` im Browser

3. **Router Firewall?**
   - Manche Router blockieren Port 80
   - Alternative: Port 8080 nutzen

4. **CG-NAT?**
   - Provider nutzt CG-NAT → Nicht lösbar
   - Lösung: Telegram Bot oder VPN

### "KO" Response von DuckDNS

**Serial Monitor zeigt:**
```
❌ DynDNS Update fehlgeschlagen: KO
```

**Ursachen:**
- Token falsch → Prüfe Token auf duckdns.org
- Domain falsch → Format: `subdomain.duckdns.org`
- Zu viele Requests → Max. 1 Request/Minute

### Keine öffentliche IP

**Problem:** ESP32 zeigt private IP (10.x, 192.168.x, 172.x)

**Lösung:**
- LTE-Router mit öffentlicher IP nutzen
- Oder Provider kontaktieren

---

## 📚 Erweiterte Konfiguration

### Update-Intervall ändern

```cpp
#define DYNDNS_UPDATE_INTERVAL 300000  // 5 Minuten (Standard)
// Ändern auf z.B.:
#define DYNDNS_UPDATE_INTERVAL 600000  // 10 Minuten
```

### Andere DynDNS-Provider

DuckDNS ist empfohlen, aber es geht auch:

**NoIP:**
```cpp
String url = "http://username:password@dynupdate.no-ip.com/nic/update?hostname=yourhostname&myip=" + ip;
```

**Strato:**
```cpp
String url = "https://dyndns.strato.com/nic/update?hostname=yourdomain.de&myip=" + ip;
```

---

## 💡 Alternative: Cloudflare Tunnel

**Falls DynDNS nicht geht** (CG-NAT, kein Port-Forwarding):

**Cloudflare Tunnel** (kostenlos):
- Kein Port-Forwarding nötig
- Eigene Domain möglich
- HTTPS inklusive
- Braucht: Raspberry Pi / PC am Standort

**Anleitung:** https://developers.cloudflare.com/cloudflare-one/connections/connect-apps/

---

## 🎬 Nächste Schritte

Dashboard von überall erreichbar?
- [Telegram Bot Setup](TELEGRAM_SETUP.md) → Push-Alarme einrichten
- [Hardware Setup](HARDWARE.md) → Sensoren & Turbine anschließen

---

**Support:** [GitHub Issues](https://github.com/AndreasS964/ForellenWaechter/issues)
