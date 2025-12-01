# 🔌 REST API Dokumentation

ForellenWächter v1.2 bietet eine REST API für externe Integrationen.

---

## Basis-URL

```
http://192.168.4.1        (Access Point)
http://forellenwaechter.local  (mDNS)
http://[IP-ADRESSE]       (Heimnetz)
```

---

## Endpunkte

### GET /api/sensors

Aktuelle Sensorwerte abrufen.

**Request:**
```bash
curl http://192.168.4.1/api/sensors
```

**Response:**
```json
{
  "waterTemp": 11.52,
  "airTemp": 18.24,
  "ph": 7.24,
  "tds": 185,
  "waterLevel": true,
  "aeration": false,
  "alarm": false,
  "timestamp": 123456789
}
```

**Felder:**

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| waterTemp | float | Wassertemperatur in °C |
| airTemp | float | Lufttemperatur in °C |
| ph | float | pH-Wert (0-14) |
| tds | float | TDS in ppm |
| waterLevel | bool | true = OK, false = niedrig |
| aeration | bool | Belüftung aktiv |
| alarm | bool | Alarm aktiv |
| timestamp | int | Messzeitpunkt (ms seit Boot) |

---

### GET /api/status

Systemstatus abrufen.

**Request:**
```bash
curl http://192.168.4.1/api/status
```

**Response:**
```json
{
  "uptime": 86400,
  "freeHeap": 180000,
  "wifiRSSI": -45,
  "sdCard": true
}
```

**Felder:**

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| uptime | int | Laufzeit in Sekunden |
| freeHeap | int | Freier Speicher in Bytes |
| wifiRSSI | int | WiFi Signalstärke in dBm |
| sdCard | bool | SD-Karte verfügbar |

---

### GET /api/history

Historische Sensordaten abrufen (Ring-Buffer, max. 288 Einträge = 24h).

**Request:**
```bash
curl http://192.168.4.1/api/history
```

**Response:**
```json
{
  "waterTemp": [11.5, 11.6, 11.4, ...],
  "airTemp": [18.2, 18.5, 18.1, ...],
  "ph": [7.24, 7.25, 7.23, ...],
  "tds": [185, 186, 184, ...]
}
```

**Hinweise:**
- Jeder Eintrag = 5-Minuten-Intervall
- Max. 288 Einträge (24 Stunden)
- Älteste Daten werden überschrieben

---

### GET /api/settings

Aktuelle Grenzwert-Einstellungen abrufen.

**Request:**
```bash
curl http://192.168.4.1/api/settings
```

**Response:**
```json
{
  "tempMin": 8.0,
  "tempMax": 14.0,
  "tempCritical": 15.5,
  "phMin": 6.5,
  "phMax": 8.5,
  "tdsMax": 500
}
```

---

### POST /api/relay

Relais manuell steuern.

**Request:**
```bash
curl -X POST "http://192.168.4.1/api/relay?relay=1&state=1"
```

**Parameter:**

| Parameter | Typ | Werte | Beschreibung |
|-----------|-----|-------|--------------|
| relay | int | 1-4 | Relais-Nummer |
| state | int | 0, 1 | 0=aus, 1=an |

**Response (Erfolg):**
```json
{
  "success": true
}
```

**Response (Fehler):**
```json
{
  "error": "Invalid relay"
}
```

---

## Beispiel-Integrationen

### Home Assistant

```yaml
sensor:
  - platform: rest
    name: "Forellen Wassertemperatur"
    resource: http://192.168.4.1/api/sensors
    value_template: "{{ value_json.waterTemp }}"
    unit_of_measurement: "°C"
    scan_interval: 60

  - platform: rest
    name: "Forellen pH"
    resource: http://192.168.4.1/api/sensors
    value_template: "{{ value_json.ph }}"
    scan_interval: 60

binary_sensor:
  - platform: rest
    name: "Forellen Alarm"
    resource: http://192.168.4.1/api/sensors
    value_template: "{{ value_json.alarm }}"
```

### Node-RED

```javascript
// HTTP Request Node
msg.url = "http://192.168.4.1/api/sensors";
return msg;

// Function Node (Parse)
var data = JSON.parse(msg.payload);
msg.waterTemp = data.waterTemp;
msg.alarm = data.alarm;
return msg;
```

### Python

```python
import requests

def get_sensor_data():
    response = requests.get('http://192.168.4.1/api/sensors')
    data = response.json()
    
    print(f"Wasser: {data['waterTemp']}°C")
    print(f"pH: {data['ph']}")
    
    if data['alarm']:
        print("⚠️ ALARM AKTIV!")
    
    return data

def set_relay(relay_num, state):
    params = {'relay': relay_num, 'state': 1 if state else 0}
    response = requests.post('http://192.168.4.1/api/relay', params=params)
    return response.json()

# Beispiel
data = get_sensor_data()
set_relay(1, True)  # Belüftung an
```

### Grafana + InfluxDB

```python
# Script zum Loggen in InfluxDB
from influxdb_client import InfluxDBClient, Point
import requests

client = InfluxDBClient(url="http://localhost:8086", token="...", org="...")
write_api = client.write_api()

data = requests.get('http://192.168.4.1/api/sensors').json()

point = Point("forellen") \
    .field("waterTemp", data['waterTemp']) \
    .field("airTemp", data['airTemp']) \
    .field("ph", data['ph']) \
    .field("tds", data['tds'])

write_api.write(bucket="monitoring", record=point)
```

---

## Fehler-Codes

| HTTP Code | Bedeutung |
|-----------|-----------|
| 200 | OK |
| 400 | Ungültige Parameter |
| 404 | Endpunkt nicht gefunden |
| 500 | Serverfehler |

---

## Rate Limiting

- Keine explizite Begrenzung
- Empfohlen: Max. 1 Request/Sekunde
- WebSocket für Live-Updates nutzen (wenn verfügbar)

---

## CORS

CORS ist standardmäßig deaktiviert. Für Browser-Zugriff von anderen Domains kann CORS im Code aktiviert werden:

```cpp
server.sendHeader("Access-Control-Allow-Origin", "*");
```
