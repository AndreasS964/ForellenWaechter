/*
 * ═══════════════════════════════════════════════════════════════════════════════════
 * html_settings.h - ForellenWächter v1.6 Settings Page
 * ═══════════════════════════════════════════════════════════════════════════════════
 */

#ifndef HTML_SETTINGS_H
#define HTML_SETTINGS_H

#include <pgmspace.h>

const char SETTINGS_HTML_HEAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Einstellungen - ForellenWächter</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
      background: linear-gradient(135deg, #1e3a8a 0%, #0f172a 100%);
      color: white; min-height: 100vh; padding: 20px;
    }
    .container { max-width: 900px; margin: 0 auto; }
    .header { display: flex; align-items: center; gap: 15px; margin-bottom: 30px; }
    .back-btn {
      font-size: 2.5em; text-decoration: none; color: white; opacity: 1;
      transition: all 0.3s; cursor: pointer; padding: 5px;
      filter: drop-shadow(0 2px 4px rgba(0,0,0,0.3));
    }
    .back-btn:hover { transform: translateX(-5px) scale(1.1); }
    h1 { font-size: 1.8em; font-weight: 300; }
    .tabs { display: flex; gap: 10px; margin-bottom: 20px; border-bottom: 1px solid rgba(255,255,255,0.1); }
    .tab {
      padding: 12px 24px; background: none; border: none;
      color: rgba(255,255,255,0.6); cursor: pointer; font-size: 1em;
      border-bottom: 2px solid transparent; transition: all 0.3s;
    }
    .tab.active { color: #0ea5e9; border-bottom-color: #0ea5e9; }
    .tab:hover { color: white; }
    .tab-content { display: none; }
    .tab-content.active { display: block; }
    .card {
      background: rgba(255,255,255,0.1); backdrop-filter: blur(10px);
      border: 1px solid rgba(255,255,255,0.2); border-radius: 15px;
      padding: 25px; margin-bottom: 20px;
    }
    .card h2 { font-size: 1.2em; margin-bottom: 20px; font-weight: 400; }
    .form-group { margin-bottom: 20px; }
    label { display: block; margin-bottom: 8px; color: rgba(255,255,255,0.8); font-size: 0.9em; }
    input, select {
      width: 100%; padding: 14px; background: rgba(255,255,255,0.15);
      border: 2px solid rgba(255,255,255,0.3); border-radius: 8px;
      color: white; font-size: 1.1em; font-weight: 500;
    }
    select {
      cursor: pointer; appearance: none;
      background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='12' height='12' viewBox='0 0 12 12'%3E%3Cpath fill='white' d='M6 9L1 4h10z'/%3E%3C/svg%3E");
      background-repeat: no-repeat; background-position: right 12px center; padding-right: 40px;
    }
    select option { background: #1e3a8a; color: white; padding: 10px; font-size: 1em; }
    input:focus, select:focus { outline: none; border-color: #0ea5e9; background: rgba(255,255,255,0.2); }
    button {
      padding: 12px 24px; background: #0ea5e9; border: none; border-radius: 8px;
      color: white; font-size: 1em; cursor: pointer; transition: all 0.3s;
    }
    button:hover { background: #0284c7; transform: translateY(-2px); }
    button.secondary { background: rgba(255,255,255,0.1); }
    button.secondary:hover { background: rgba(255,255,255,0.2); }
    .btn-group { display: flex; gap: 10px; margin-top: 20px; }
    .info {
      background: rgba(14,165,233,0.2); border: 1px solid rgba(14,165,233,0.4);
      border-radius: 8px; padding: 12px; margin-bottom: 20px; font-size: 0.9em;
    }
    .success {
      background: rgba(16,185,129,0.2); border-color: rgba(16,185,129,0.4);
      padding: 12px; border-radius: 8px; margin-top: 15px; display: none;
    }
  </style>
</head>
)rawliteral";

const char SETTINGS_HTML_BODY[] PROGMEM = R"rawliteral(
<body>
  <div class="container">
    <div class="header">
      <a href="/" class="back-btn">←</a>
      <h1>⚙️ Einstellungen</h1>
    </div>

    <div class="tabs">
      <button class="tab active" onclick="showTab('calibration')">Kalibrierung</button>
      <button class="tab" onclick="showTab('fish')">Fischarten</button>
      <button class="tab" onclick="showTab('weather')">Wetter</button>
    </div>

    <div id="calibration" class="tab-content active">
      <div class="card">
        <h2>🧪 pH Kalibrierung (2-Punkt)</h2>
        <div class="info">💡 Tauche den pH-Sensor in pH 4.0 und pH 7.0 Kalibrierlösungen.</div>
        <div class="form-group">
          <label>Punkt 1: pH 4.0 Pufferlösung</label>
          <button onclick="calibratePH(1, 4.0)" style="margin-top:10px">Punkt 1 messen</button>
        </div>
        <div class="form-group">
          <label>Punkt 2: pH 7.0 Pufferlösung</label>
          <button onclick="calibratePH(2, 7.0)" style="margin-top:10px">Punkt 2 messen & Kalibrieren</button>
        </div>
        <button class="secondary" onclick="resetCalibration('ph')">Kalibrierung zurücksetzen</button>
        <div id="phSuccess" class="success">✅ pH Kalibrierung gespeichert!</div>
      </div>

      <div class="card">
        <h2>💧 TDS Kalibrierung (1-Punkt)</h2>
        <div class="info">💡 Verwende eine 707 ppm (1413 µS/cm) Kalibrierlösung.</div>
        <div class="form-group">
          <label>Bekannter TDS-Wert (ppm)</label>
          <input type="number" id="tdsKnown" value="707" step="1">
        </div>
        <button onclick="calibrateTDS()">TDS kalibrieren</button>
        <div id="tdsSuccess" class="success">✅ TDS Kalibrierung gespeichert!</div>
      </div>
    </div>

    <div id="fish" class="tab-content">
      <div class="card">
        <h2>🐟 Fischarten-Voreinstellungen</h2>
        <div class="info">💡 Wähle eine Fischart und die optimalen Parameter werden automatisch gesetzt.</div>
        <div class="form-group">
          <label>Fischart auswählen</label>
          <select id="fishSelect" onchange="applyFishPreset()">
            <option value="rainbow">🐟 Regenbogenforelle (8-14°C, pH 6.5-8.5)</option>
            <option value="brown">🐟 Bachforelle (4-18°C, pH 6.5-8.0)</option>
            <option value="brook">🐟 Bachsaibling (5-16°C, pH 6.0-8.0)</option>
            <option value="carp">🐟 Karpfen (18-24°C, pH 7.0-8.5)</option>
            <option value="tilapia">🐟 Tilapia (25-30°C, pH 6.5-9.0)</option>
            <option value="catfish">🐟 Wels (20-28°C, pH 6.5-8.0)</option>
            <option value="pike">🐟 Hecht (10-21°C, pH 6.5-8.5)</option>
            <option value="perch">🐟 Barsch (15-25°C, pH 7.0-8.5)</option>
            <option value="sturgeon">🐟 Stör (15-20°C, pH 7.0-8.0)</option>
            <option value="custom">⚙️ Benutzerdefiniert</option>
          </select>
        </div>
        <div class="form-group">
          <label>Temperatur Min (°C)</label>
          <input type="number" id="tempMin" value="8" step="0.5">
        </div>
        <div class="form-group">
          <label>Temperatur Max (°C)</label>
          <input type="number" id="tempMax" value="14" step="0.5">
        </div>
        <div class="form-group">
          <label>pH Min</label>
          <input type="number" id="phMin" value="6.5" step="0.1">
        </div>
        <div class="form-group">
          <label>pH Max</label>
          <input type="number" id="phMax" value="8.5" step="0.1">
        </div>
        <button onclick="saveSettings()">Einstellungen speichern</button>
        <div id="fishSuccess" class="success">✅ Einstellungen gespeichert!</div>
      </div>
    </div>

    <div id="weather" class="tab-content">
      <div class="card">
        <h2>🌤️ Wetter-Widget</h2>
        <div class="info">💡 Gib deine Postleitzahl ein für lokale Wetterdaten im Dashboard.</div>
        <div class="form-group">
          <label>Postleitzahl (PLZ)</label>
          <input type="text" id="weatherZip" placeholder="z.B. 72296" maxlength="10">
        </div>
        <button onclick="saveWeather()">Speichern</button>
        <div id="weatherSuccess" class="success">✅ PLZ gespeichert!</div>
      </div>
    </div>
  </div>
)rawliteral";

const char SETTINGS_JS[] PROGMEM = R"rawliteral(
  <script>
    const fishPresets = {
      rainbow: { tempMin: 8, tempMax: 14, phMin: 6.5, phMax: 8.5 },
      brown: { tempMin: 4, tempMax: 18, phMin: 6.5, phMax: 8.0 },
      brook: { tempMin: 5, tempMax: 16, phMin: 6.0, phMax: 8.0 },
      carp: { tempMin: 18, tempMax: 24, phMin: 7.0, phMax: 8.5 },
      tilapia: { tempMin: 25, tempMax: 30, phMin: 6.5, phMax: 9.0 },
      catfish: { tempMin: 20, tempMax: 28, phMin: 6.5, phMax: 8.0 },
      pike: { tempMin: 10, tempMax: 21, phMin: 6.5, phMax: 8.5 },
      perch: { tempMin: 15, tempMax: 25, phMin: 7.0, phMax: 8.5 },
      sturgeon: { tempMin: 15, tempMax: 20, phMin: 7.0, phMax: 8.0 }
    };
    
    function showTab(tabId) {
      document.querySelectorAll('.tab-content').forEach(t => t.classList.remove('active'));
      document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
      document.getElementById(tabId).classList.add('active');
      event.target.classList.add('active');
    }
    
    function applyFishPreset() {
      const select = document.getElementById('fishSelect');
      const preset = fishPresets[select.value];
      if (preset) {
        document.getElementById('tempMin').value = preset.tempMin;
        document.getElementById('tempMax').value = preset.tempMax;
        document.getElementById('phMin').value = preset.phMin;
        document.getElementById('phMax').value = preset.phMax;
      }
    }
    
    async function calibratePH(step, bufferValue) {
      try {
        const res = await fetch('/api/calibration/ph', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ step: step, buffer_value: bufferValue })
        });
        const data = await res.json();
        if (data.success) {
          document.getElementById('phSuccess').style.display = 'block';
          document.getElementById('phSuccess').textContent = '✅ ' + data.message;
          setTimeout(() => document.getElementById('phSuccess').style.display = 'none', 3000);
        }
      } catch (e) { alert('Fehler: ' + e); }
    }
    
    async function calibrateTDS() {
      const known = parseFloat(document.getElementById('tdsKnown').value);
      try {
        const res = await fetch('/api/calibration/tds', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ reference_value: known })
        });
        const data = await res.json();
        if (data.success) {
          document.getElementById('tdsSuccess').style.display = 'block';
          setTimeout(() => document.getElementById('tdsSuccess').style.display = 'none', 3000);
        }
      } catch (e) { alert('Fehler: ' + e); }
    }
    
    async function resetCalibration(sensor) {
      if (!confirm('Kalibrierung wirklich zurücksetzen?')) return;
      try {
        await fetch('/api/calibration/reset', { method: 'POST' });
        alert('Kalibrierung zurückgesetzt!');
      } catch (e) { alert('Fehler: ' + e); }
    }
    
    async function saveSettings() {
      const settings = {
        tempMin: parseFloat(document.getElementById('tempMin').value),
        tempMax: parseFloat(document.getElementById('tempMax').value),
        phMin: parseFloat(document.getElementById('phMin').value),
        phMax: parseFloat(document.getElementById('phMax').value)
      };
      try {
        const res = await fetch('/api/settings', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(settings)
        });
        if (res.ok) {
          document.getElementById('fishSuccess').style.display = 'block';
          setTimeout(() => document.getElementById('fishSuccess').style.display = 'none', 3000);
        }
      } catch (e) { alert('Fehler: ' + e); }
    }
    
    function saveWeather() {
      const zip = document.getElementById('weatherZip').value.trim();
      if (zip) {
        localStorage.setItem('weatherZip', zip);
        document.getElementById('weatherSuccess').style.display = 'block';
        setTimeout(() => document.getElementById('weatherSuccess').style.display = 'none', 3000);
      }
    }
    
    // Load saved values
    document.addEventListener('DOMContentLoaded', () => {
      const savedZip = localStorage.getItem('weatherZip');
      if (savedZip) document.getElementById('weatherZip').value = savedZip;
    });
  </script>
</body>
</html>
)rawliteral";

#endif // HTML_SETTINGS_H
