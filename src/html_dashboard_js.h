/*
 * ═══════════════════════════════════════════════════════════════════════════════════
 * html_dashboard_js.h - ForellenWächter v1.6 Dashboard JavaScript
 * ═══════════════════════════════════════════════════════════════════════════════════
 */

#ifndef HTML_DASHBOARD_JS_H
#define HTML_DASHBOARD_JS_H

#include <pgmspace.h>

const char DASHBOARD_JS[] PROGMEM = R"rawliteral(
  <script>
    let tempChart, qualityChart;
    let relayModes = [2, 2, 2, 0];
    
    function initCharts() {
      const defaultOptions = {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { position: 'top', labels: { color: 'rgba(255,255,255,0.8)', padding: 20 } }
        },
        scales: {
          x: { grid: { color: 'rgba(255,255,255,0.1)' }, ticks: { color: 'rgba(255,255,255,0.6)' } },
          y: { grid: { color: 'rgba(255,255,255,0.1)' }, ticks: { color: 'rgba(255,255,255,0.6)' } }
        }
      };
      
      tempChart = new Chart(document.getElementById('tempChart'), {
        type: 'line',
        data: {
          labels: [],
          datasets: [{
            label: 'Wasser °C', data: [], borderColor: '#0ea5e9',
            backgroundColor: 'rgba(14,165,233,0.1)', fill: true, tension: 0.4, borderWidth: 2
          }, {
            label: 'Luft °C', data: [], borderColor: '#f59e0b',
            backgroundColor: 'rgba(245,158,11,0.1)', fill: true, tension: 0.4, borderWidth: 2
          }]
        },
        options: defaultOptions
      });
      
      qualityChart = new Chart(document.getElementById('qualityChart'), {
        type: 'line',
        data: {
          labels: [],
          datasets: [
            { label: 'pH', data: [], borderColor: '#a855f7', yAxisID: 'y', tension: 0.4, borderWidth: 2 },
            { label: 'O₂ mg/L', data: [], borderColor: '#10b981', yAxisID: 'y1', tension: 0.4, borderWidth: 2 },
            { label: 'TDS ppm', data: [], borderColor: '#f59e0b', yAxisID: 'y2', tension: 0.4, borderWidth: 2 }
          ]
        },
        options: {
          ...defaultOptions,
          scales: {
            x: defaultOptions.scales.x,
            y: { ...defaultOptions.scales.y, position: 'left', min: 5, max: 10 },
            y1: { ...defaultOptions.scales.y, position: 'right', min: 0, max: 15, grid: { display: false } },
            y2: { ...defaultOptions.scales.y, position: 'right', min: 0, max: 500, grid: { display: false } }
          }
        }
      });
    }
    
    async function fetchSensors() {
      try {
        const res = await fetch('/api/sensors');
        const data = await res.json();
        updateSensorDisplay(data);
      } catch (e) { console.error('Fetch error:', e); }
    }
    
    async function fetchStatus() {
      try {
        const res = await fetch('/api/status');
        const data = await res.json();
        updateStatusDisplay(data);
      } catch (e) {}
    }
    
    async function fetchHistory() {
      try {
        const res = await fetch('/api/history');
        const data = await res.json();
        updateCharts(data);
      } catch (e) {}
    }
    
    function updateSensorDisplay(data) {
      document.getElementById('waterTemp').textContent = data.waterTemp.toFixed(1);
      document.getElementById('airTemp').textContent = data.airTemp.toFixed(1);
      document.getElementById('phValue').textContent = data.ph.toFixed(2);
      document.getElementById('tdsValue').textContent = Math.round(data.tds);
      document.getElementById('doValue').textContent = data.dissolvedOxygen.toFixed(1);
      document.getElementById('waterLevel').textContent = data.waterLevel ? 'OK' : 'NIEDRIG';
      
      updateCardStatus('cardWater', data.waterTemp, 8, 14, 16);
      updateCardStatus('cardPH', data.ph, 6.5, 8.5);
      updateCardStatus('cardTDS', data.tds, 0, 500);
      updateCardStatus('cardDO', data.dissolvedOxygen, 6, 999);
      document.getElementById('cardLevel').className = 'card ' + (data.waterLevel ? 'ok' : 'danger');
      
      const banner = document.getElementById('alarmBanner');
      if (data.alarm) {
        banner.classList.add('show');
        document.getElementById('alarmReason').textContent = data.alarmReason || 'ALARM AKTIV';
      } else {
        banner.classList.remove('show');
      }
    }
    
    function updateCardStatus(id, value, min, max, critical) {
      const card = document.getElementById(id);
      if (!card) return;
      card.classList.remove('ok', 'warning', 'danger');
      if (critical && value > critical) card.classList.add('danger');
      else if (value < min || value > max) card.classList.add('warning');
      else card.classList.add('ok');
    }
    
    function updateStatusDisplay(data) {
      const wifiDot = document.getElementById('dotWifi');
      document.getElementById('statusWifi').textContent = data.wifiConnected ? `WiFi: ${data.wifiRSSI} dBm` : 'WiFi: Offline';
      wifiDot.className = 'dot ' + (data.wifiConnected ? '' : 'warning');
      
      const lteDot = document.getElementById('dotLTE');
      document.getElementById('statusLTE').textContent = data.lteConnected ? `LTE: ${data.lteSignal}/31` : 'LTE: Offline';
      lteDot.className = 'dot ' + (data.lteConnected ? '' : 'warning');
      
      document.getElementById('statusUptime').textContent = `Uptime: ${formatUptime(data.uptime)}`;
      document.getElementById('wifiRSSI').textContent = data.wifiRSSI + ' dBm';
      document.getElementById('lteSignal').textContent = data.lteSignal + '/31';
      document.getElementById('lteOperator').textContent = data.lteOperator || '--';
      document.getElementById('publicIP').textContent = data.publicIP || '--';
      document.getElementById('firmware').textContent = 'v' + data.firmware;
      document.getElementById('freeHeap').textContent = Math.round(data.freeHeap / 1024) + ' KB';
      document.getElementById('sdCard').textContent = data.sdCard ? 'OK' : 'Fehlt';
      document.getElementById('dailyAlarms').textContent = data.dailyAlarms;
    }
    
    function updateCharts(data) {
      const labels = data.waterTemp.map((_, i) => {
        const minutes = i * 15;
        return minutes >= 60 ? Math.floor(minutes / 60) + 'h' : minutes + 'm';
      });
      tempChart.data.labels = labels;
      tempChart.data.datasets[0].data = data.waterTemp;
      tempChart.data.datasets[1].data = data.airTemp;
      tempChart.update('none');
      
      qualityChart.data.labels = labels;
      qualityChart.data.datasets[0].data = data.ph;
      qualityChart.data.datasets[1].data = data.do || [];
      qualityChart.data.datasets[2].data = data.tds || [];
      qualityChart.update('none');
    }
    
    function formatUptime(seconds) {
      const d = Math.floor(seconds / 86400);
      const h = Math.floor((seconds % 86400) / 3600);
      const m = Math.floor((seconds % 3600) / 60);
      if (d > 0) return `${d}d ${h}h`;
      if (h > 0) return `${h}h ${m}m`;
      return `${m}m`;
    }
    
    async function toggleRelay(num) {
      try {
        const res = await fetch(`/api/relay?relay=${num}`, { method: 'POST' });
        if (res.ok) {
          const data = await res.json();
          relayModes[num - 1] = data.mode;
          updateRelayButton(num, data.mode);
        }
      } catch (e) { console.error('Relay error:', e); }
    }
    
    function updateRelayButton(num, mode) {
      const btn = document.getElementById(`relay${num}`);
      const label = btn.querySelector('.mode-label');
      btn.classList.remove('mode-auto', 'mode-on', 'mode-off');
      if (mode === 0) { btn.classList.add('mode-auto'); label.textContent = 'Auto'; }
      else if (mode === 1) { btn.classList.add('mode-on'); label.textContent = 'An'; }
      else { btn.classList.add('mode-off'); label.textContent = 'Aus'; }
    }
    
    async function fetchDashboardWeather() {
      const zip = localStorage.getItem('weatherZip');
      if (!zip) return;
      const weatherDashboard = document.getElementById('weatherDashboard');
      const weatherConfigHint = document.getElementById('weatherConfigHint');
      const weatherDashboardData = document.getElementById('weatherDashboardData');
      try {
        weatherDashboardData.innerHTML = '⏳ Lade...';
        weatherDashboard.style.display = 'block';
        weatherConfigHint.style.display = 'none';
        const response = await fetch(`https://wttr.in/${zip}?format=j1`);
        const data = await response.json();
        const current = data.current_condition[0];
        weatherDashboardData.innerHTML = `
          <p style="margin: 5px 0; font-size: 1.05em;">🌡️ <strong>${current.temp_C}°C</strong> (gefühlt ${current.FeelsLikeC}°C)</p>
          <p style="margin: 5px 0; font-size: 0.95em;">☁️ ${current.lang_de[0].value}</p>
          <p style="margin: 5px 0; font-size: 0.9em;">💧 ${current.humidity}% | 💨 ${current.windspeedKmph} km/h</p>`;
      } catch (e) {
        weatherDashboard.style.display = 'none';
        weatherConfigHint.style.display = 'block';
      }
    }
    
    function setRange(chart, hours) {
      event.target.parentNode.querySelectorAll('.chart-tab').forEach(t => t.classList.remove('active'));
      event.target.classList.add('active');
    }
    
    initCharts();
    fetchSensors();
    fetchStatus();
    fetchHistory();
    fetchDashboardWeather();
    setInterval(fetchSensors, 2000);
    setInterval(fetchStatus, 10000);
    setInterval(fetchHistory, 60000);
    setInterval(fetchDashboardWeather, 1800000);
  </script>
</body>
</html>
)rawliteral";

#endif // HTML_DASHBOARD_JS_H
