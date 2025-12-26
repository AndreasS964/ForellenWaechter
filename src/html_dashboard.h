/*
 * ═══════════════════════════════════════════════════════════════════════════════════
 * html_dashboard.h - ForellenWächter v1.6 Dashboard HTML
 * ═══════════════════════════════════════════════════════════════════════════════════
 * 
 * HTML/CSS/JS für das Haupt-Dashboard.
 * Gespeichert in PROGMEM um RAM zu sparen!
 */

#ifndef HTML_DASHBOARD_H
#define HTML_DASHBOARD_H

#include <pgmspace.h>

// ═══════════════════════════════════════════════════════════════════════════════════
// DASHBOARD HTML - Aufgeteilt in Chunks für bessere Speicherverwaltung
// ═══════════════════════════════════════════════════════════════════════════════════

const char DASHBOARD_HTML_HEAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ForellenWächter v1.6</title>
  <link rel="icon" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='.9em' font-size='90'>🐟</text></svg>">
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
)rawliteral";

const char DASHBOARD_CSS[] PROGMEM = R"rawliteral(
  <style>
    :root {
      --primary: #0ea5e9;
      --primary-dark: #0284c7;
      --secondary: #10b981;
      --danger: #ef4444;
      --warning: #f59e0b;
      --dark: #1e293b;
      --darker: #0f172a;
      --light: #f8fafc;
      --glass: rgba(255,255,255,0.1);
      --glass-border: rgba(255,255,255,0.2);
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: 'Segoe UI', system-ui, -apple-system, sans-serif;
      background: linear-gradient(135deg, var(--darker) 0%, #1a365d 50%, #134e4a 100%);
      min-height: 100vh;
      color: var(--light);
    }
    .container { max-width: 1400px; margin: 0 auto; padding: 20px; }
    header {
      display: flex;
      align-items: center;
      gap: 20px;
      padding: 30px 0;
      margin-bottom: 30px;
    }
    header > div:first-child, header > div:nth-child(2) { text-align: center; }
    .logo { font-size: 3.5em; animation: swim 3s ease-in-out infinite; }
    @keyframes swim {
      0%, 100% { transform: translateX(0) rotate(0deg); }
      25% { transform: translateX(10px) rotate(5deg); }
      75% { transform: translateX(-10px) rotate(-5deg); }
    }
    header h1 {
      font-size: 2em; font-weight: 300; letter-spacing: 2px; text-transform: uppercase;
      background: linear-gradient(90deg, var(--primary), var(--secondary));
      -webkit-background-clip: text; -webkit-text-fill-color: transparent; background-clip: text;
      margin: 0;
    }
    header .subtitle { color: rgba(255,255,255,0.6); font-size: 0.9em; margin-top: 5px; }
    .settings-btn {
      font-size: 2em; text-decoration: none; opacity: 0.7;
      transition: opacity 0.3s, transform 0.3s; cursor: pointer;
    }
    .settings-btn:hover { opacity: 1; transform: rotate(45deg); }
    .status-bar {
      display: flex; justify-content: center; flex-wrap: wrap;
      gap: 15px; margin-bottom: 30px;
    }
    .status-pill {
      display: flex; align-items: center; gap: 8px;
      padding: 10px 20px; background: var(--glass);
      backdrop-filter: blur(10px); border: 1px solid var(--glass-border);
      border-radius: 50px; font-size: 0.85em; transition: all 0.3s;
    }
    .status-pill:hover { background: rgba(255,255,255,0.15); transform: translateY(-2px); }
    .status-pill .dot {
      width: 8px; height: 8px; border-radius: 50%;
      background: var(--secondary); animation: pulse 2s infinite;
    }
    .status-pill .dot.warning { background: var(--warning); }
    .status-pill .dot.danger { background: var(--danger); animation: pulse 0.5s infinite; }
    @keyframes pulse {
      0%, 100% { opacity: 1; transform: scale(1); }
      50% { opacity: 0.5; transform: scale(1.2); }
    }
    .cards-grid {
      display: grid; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
      gap: 20px; margin-bottom: 30px;
    }
    .card {
      background: var(--glass); backdrop-filter: blur(10px);
      border: 1px solid var(--glass-border); border-radius: 20px;
      padding: 25px; text-align: center;
      transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
      position: relative; overflow: hidden;
    }
    .card::before {
      content: ''; position: absolute; top: 0; left: 0; right: 0; height: 3px;
      background: linear-gradient(90deg, var(--primary), var(--secondary));
      opacity: 0; transition: opacity 0.3s;
    }
    .card:hover { transform: translateY(-5px); box-shadow: 0 20px 40px rgba(0,0,0,0.3); }
    .card:hover::before { opacity: 1; }
    .card.ok::before { background: var(--secondary); opacity: 1; }
    .card.warning::before { background: var(--warning); opacity: 1; }
    .card.danger::before { background: var(--danger); opacity: 1; animation: pulse 0.5s infinite; }
    .card .icon { font-size: 2.5em; margin-bottom: 15px; filter: drop-shadow(0 4px 6px rgba(0,0,0,0.3)); }
    .card .value {
      font-size: 2.5em; font-weight: 700;
      background: linear-gradient(135deg, #fff, rgba(255,255,255,0.8));
      -webkit-background-clip: text; -webkit-text-fill-color: transparent; background-clip: text;
    }
    .card .unit { font-size: 0.8em; opacity: 0.7; font-weight: 300; }
    .card .label {
      font-size: 0.8em; text-transform: uppercase; letter-spacing: 1px;
      color: rgba(255,255,255,0.6); margin-top: 10px;
    }
    .card .range { font-size: 0.7em; color: rgba(255,255,255,0.4); margin-top: 5px; }
    .charts-section {
      display: grid; grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
      gap: 20px; margin-bottom: 30px;
    }
    .chart-card {
      background: var(--glass); backdrop-filter: blur(10px);
      border: 1px solid var(--glass-border); border-radius: 20px; padding: 25px;
    }
    .chart-container { position: relative; height: 300px; max-height: 300px; width: 100%; overflow: hidden; }
    .chart-card h3 {
      font-size: 1em; font-weight: 500; margin-bottom: 20px;
      display: flex; align-items: center; gap: 10px;
    }
    .chart-tabs { display: flex; gap: 10px; margin-left: auto; }
    .chart-tab {
      padding: 5px 12px; background: rgba(255,255,255,0.1); border: none;
      border-radius: 15px; color: white; font-size: 0.75em; cursor: pointer; transition: all 0.3s;
    }
    .chart-tab:hover { background: rgba(255,255,255,0.2); }
    .chart-tab.active { background: var(--primary); }
    .system-grid {
      display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px; margin-bottom: 30px;
    }
    .info-card {
      background: var(--glass); backdrop-filter: blur(10px);
      border: 1px solid var(--glass-border); border-radius: 20px; padding: 25px;
    }
    .info-card h3 {
      font-size: 0.9em; text-transform: uppercase; letter-spacing: 1px;
      color: rgba(255,255,255,0.6); margin-bottom: 20px;
      display: flex; align-items: center; gap: 10px;
    }
    .info-row {
      display: flex; justify-content: space-between; padding: 10px 0;
      border-bottom: 1px solid rgba(255,255,255,0.1);
    }
    .info-row:last-child { border-bottom: none; }
    .info-label { color: rgba(255,255,255,0.6); }
    .info-value { font-weight: 500; }
    .controls-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; }
    .relay-btn {
      padding: 15px; background: rgba(255,255,255,0.1);
      border: 1px solid rgba(255,255,255,0.2); border-radius: 15px;
      color: white; cursor: pointer; transition: all 0.3s;
      display: flex; flex-direction: column; align-items: center; gap: 8px;
    }
    .relay-btn:hover { background: rgba(255,255,255,0.2); }
    .relay-btn.active { background: var(--secondary); border-color: var(--secondary); }
    .relay-btn .icon { font-size: 1.5em; }
    .relay-btn .name { font-size: 0.8em; font-weight: 500; }
    .relay-btn .mode-label { font-size: 0.7em; margin-top: 5px; padding: 3px 8px; border-radius: 10px; font-weight: 500; }
    .relay-btn.mode-auto { background: rgba(14,165,233,0.3); border-color: rgba(14,165,233,0.5); }
    .relay-btn.mode-auto .mode-label { background: rgba(14,165,233,0.5); color: white; }
    .relay-btn.mode-on { background: rgba(16,185,129,0.3); border-color: rgba(16,185,129,0.5); }
    .relay-btn.mode-on .mode-label { background: rgba(16,185,129,0.5); color: white; }
    .relay-btn.mode-off { opacity: 0.5; }
    .relay-btn.mode-off .mode-label { background: rgba(255,255,255,0.2); color: rgba(255,255,255,0.7); }
    .alarm-banner {
      display: none; background: linear-gradient(90deg, var(--danger), #dc2626);
      padding: 15px 25px; border-radius: 15px; margin-bottom: 20px; animation: alarm-pulse 1s infinite;
    }
    .alarm-banner.show { display: flex; align-items: center; gap: 15px; }
    @keyframes alarm-pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.8; } }
    .alarm-banner .icon { font-size: 1.5em; }
    .alarm-banner .text { flex: 1; }
    .alarm-banner .reason { font-weight: 700; }
    footer { text-align: center; padding: 30px; color: rgba(255,255,255,0.4); font-size: 0.85em; }
    footer a { color: var(--primary); text-decoration: none; }
    footer a:hover { text-decoration: underline; }
    @media (max-width: 768px) {
      .container { padding: 15px; }
      header h1 { font-size: 1.5em; }
      .cards-grid { grid-template-columns: repeat(2, 1fr); }
      .charts-section { grid-template-columns: 1fr; }
      .chart-card { min-width: 0; }
      .chart-container { height: 250px; max-height: 250px; }
    }
  </style>
</head>
)rawliteral";

const char DASHBOARD_HTML_BODY1[] PROGMEM = R"rawliteral(
<body>
  <div class="container">
    <header>
      <div class="logo">🐟</div>
      <div style="flex: 1;">
        <h1>ForellenWächter</h1>
        <p class="subtitle">IoT Monitoring für Aquakultur • v1.6</p>
      </div>
      <a href="/settings" class="settings-btn" title="Einstellungen">⚙️</a>
    </header>
    
    <div class="alarm-banner" id="alarmBanner">
      <span class="icon">🚨</span>
      <div class="text">
        <div class="reason" id="alarmReason">ALARM</div>
        <div>Sofortige Überprüfung erforderlich!</div>
      </div>
    </div>
    
    <div class="status-bar">
      <div class="status-pill">
        <span class="dot" id="dotWifi"></span>
        <span id="statusWifi">WiFi: --</span>
      </div>
      <div class="status-pill">
        <span class="dot" id="dotLTE"></span>
        <span id="statusLTE">LTE: --</span>
      </div>
      <div class="status-pill">
        <span class="dot" id="dotSystem"></span>
        <span id="statusUptime">Uptime: --</span>
      </div>
    </div>
    
    <div class="cards-grid">
      <div class="card" id="cardWater">
        <div class="icon">💧</div>
        <div class="value"><span id="waterTemp">--</span><span class="unit">°C</span></div>
        <div class="label">Wassertemperatur</div>
        <div class="range">Optimal: 8-14°C</div>
      </div>
      <div class="card" id="cardAir">
        <div class="icon">🌡️</div>
        <div class="value"><span id="airTemp">--</span><span class="unit">°C</span></div>
        <div class="label">Lufttemperatur</div>
        <div class="range">Umgebung</div>
      </div>
      <div class="card" id="cardPH">
        <div class="icon">🧪</div>
        <div class="value"><span id="phValue">--</span></div>
        <div class="label">pH-Wert</div>
        <div class="range">Optimal: 6.5-8.5</div>
      </div>
      <div class="card" id="cardTDS">
        <div class="icon">📊</div>
        <div class="value"><span id="tdsValue">--</span><span class="unit">ppm</span></div>
        <div class="label">TDS</div>
        <div class="range">Max: 500 ppm</div>
      </div>
      <div class="card" id="cardDO">
        <div class="icon">🫧</div>
        <div class="value"><span id="doValue">--</span><span class="unit">mg/L</span></div>
        <div class="label">Sauerstoff</div>
        <div class="range">Min: 6.0 mg/L</div>
      </div>
      <div class="card" id="cardLevel">
        <div class="icon">🌊</div>
        <div class="value" id="waterLevel">--</div>
        <div class="label">Wasserlevel</div>
        <div class="range">Float Switch</div>
      </div>
    </div>
)rawliteral";

const char DASHBOARD_HTML_BODY2[] PROGMEM = R"rawliteral(
    <div class="charts-section">
      <div class="chart-card">
        <h3>📈 Temperaturverlauf
          <div class="chart-tabs">
            <button class="chart-tab active" onclick="setRange('temp', 24)">24h</button>
            <button class="chart-tab" onclick="setRange('temp', 168)">7d</button>
          </div>
        </h3>
        <div class="chart-container"><canvas id="tempChart"></canvas></div>
      </div>
      <div class="chart-card">
        <h3>🧪 Wasserqualität
          <div class="chart-tabs">
            <button class="chart-tab active" onclick="setRange('quality', 24)">24h</button>
            <button class="chart-tab" onclick="setRange('quality', 168)">7d</button>
          </div>
        </h3>
        <div class="chart-container"><canvas id="qualityChart"></canvas></div>
      </div>
    </div>
    
    <div class="system-grid">
      <div class="info-card">
        <h3>📡 Verbindung</h3>
        <div class="info-row"><span class="info-label">WiFi RSSI</span><span class="info-value" id="wifiRSSI">-- dBm</span></div>
        <div class="info-row"><span class="info-label">LTE Signal</span><span class="info-value" id="lteSignal">--/31</span></div>
        <div class="info-row"><span class="info-label">Operator</span><span class="info-value" id="lteOperator">--</span></div>
        <div class="info-row"><span class="info-label">Public IP</span><span class="info-value" id="publicIP">--</span></div>
      </div>
      <div class="info-card">
        <h3>⚙️ System</h3>
        <div class="info-row"><span class="info-label">Firmware</span><span class="info-value" id="firmware">v1.6.0</span></div>
        <div class="info-row"><span class="info-label">Free Heap</span><span class="info-value" id="freeHeap">-- KB</span></div>
        <div class="info-row"><span class="info-label">SD-Karte</span><span class="info-value" id="sdCard">--</span></div>
        <div class="info-row"><span class="info-label">Alarme heute</span><span class="info-value" id="dailyAlarms">0</span></div>
      </div>
      <div class="info-card">
        <h3>🎛️ Steuerung</h3>
        <div class="controls-grid">
          <button class="relay-btn mode-off" id="relay1" onclick="toggleRelay(1)">
            <span class="icon">🔔</span><span class="name">Alarm</span><span class="mode-label">Aus</span>
          </button>
          <button class="relay-btn mode-off" id="relay2" onclick="toggleRelay(2)">
            <span class="icon">⚡</span><span class="name">Reserve 1</span><span class="mode-label">Aus</span>
          </button>
          <button class="relay-btn mode-off" id="relay3" onclick="toggleRelay(3)">
            <span class="icon">⚡</span><span class="name">Reserve 2</span><span class="mode-label">Aus</span>
          </button>
          <button class="relay-btn mode-auto" id="relay4" onclick="toggleRelay(4)">
            <span class="icon">💨</span><span class="name">Belüftung</span><span class="mode-label">Auto</span>
          </button>
        </div>
      </div>
      <div class="info-card">
        <h3>🌤️ Wetter</h3>
        <div id="weatherWidget">
          <div id="weatherDashboard" style="display:none;">
            <div id="weatherDashboardData" style="font-size: 0.9em;"></div>
          </div>
          <p id="weatherConfigHint" style="color: rgba(255,255,255,0.6); font-size: 0.9em;">
            PLZ in <a href="/settings" style="color: #0ea5e9;">Einstellungen</a> konfigurieren
          </p>
        </div>
      </div>
    </div>
    <footer>ForellenWächter v1.6 LTE Edition • <a href="/api/sensors">API</a> • © 2024 Andreas Sika</footer>
  </div>
)rawliteral";

#endif // HTML_DASHBOARD_H
