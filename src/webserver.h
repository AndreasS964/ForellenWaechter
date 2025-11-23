/*
 * ForellenWächter v2.1 - Web Server & UI
 * Modernes UI mit Dark Mode, Glassmorphism, PWA Support
 *
 * SECURITY v2.1:
 * - Web Authentication mit CredentialsManager
 * - Rate Limiting (DoS-Schutz)
 * - Path Traversal Protection
 * - WebSocket Token Authentication
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <WebServer.h>
#include <WebSocketsServer.h>
#include "config.h"
#include "sensors.h"

// Security & Network (v2.1)
#if ENABLE_CREDENTIALS_MANAGER
#include "credentials_manager.h"
extern CredentialsManager credentialsManager;
#endif

#if ENABLE_RATE_LIMITING
#include "rate_limiter.h"
extern RateLimiter rateLimiter;
#endif

#if ENABLE_INA219
#include "ina219_monitor.h"
extern INA219Monitor powerMonitor;
#endif

// Forward Declarations
extern SensorManager sensorManager;
extern bool aerationActive;
extern bool alarmActive;

// Web Server & WebSocket
WebServer server(WEB_SERVER_PORT);
WebSocketsServer webSocket(WEBSOCKET_PORT);

// SECURITY v2.1: Enhanced Session Management
struct ClientSession {
    IPAddress ip;
    String token;
    unsigned long expires;
    bool authenticated;
};

ClientSession activeSessions[5]; // Max 5 gleichzeitige Sessions
int sessionCount = 0;

// WebSocket Authentication Tokens
String wsAuthTokens[10]; // Max 10 WebSocket-Clients
int wsTokenCount = 0;

// ========== HTML TEMPLATE ==========

const char HTML_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="theme-color" content="#1e3a8a">
    <title>ForellenWächter v2.1</title>
    <link rel="manifest" href="/manifest.json">
    <link rel="icon" type="image/png" href="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==">
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        :root {
            --primary: #1e3a8a;
            --secondary: #3b82f6;
            --success: #10b981;
            --warning: #f59e0b;
            --danger: #ef4444;
            --bg-light: #f8fafc;
            --bg-dark: #0f172a;
            --card-light: #ffffff;
            --card-dark: #1e293b;
            --text-light: #1e293b;
            --text-dark: #f1f5f9;
            --border-light: #e2e8f0;
            --border-dark: #334155;
            --glass-bg: rgba(255, 255, 255, 0.1);
            --glass-border: rgba(255, 255, 255, 0.2);
        }

        [data-theme="light"] {
            --bg: var(--bg-light);
            --card-bg: var(--card-light);
            --text: var(--text-light);
            --border: var(--border-light);
            --shadow: rgba(0, 0, 0, 0.1);
        }

        [data-theme="dark"] {
            --bg: var(--bg-dark);
            --card-bg: var(--card-dark);
            --text: var(--text-dark);
            --border: var(--border-dark);
            --shadow: rgba(0, 0, 0, 0.3);
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: var(--bg);
            color: var(--text);
            min-height: 100vh;
            padding: 15px;
            transition: background 0.3s, color 0.3s;
        }

        /* Glassmorphism Container */
        .glass-container {
            background: var(--glass-bg);
            backdrop-filter: blur(10px);
            -webkit-backdrop-filter: blur(10px);
            border: 1px solid var(--glass-border);
            border-radius: 20px;
            padding: 20px;
            box-shadow: 0 8px 32px var(--shadow);
        }

        .container {
            max-width: 1400px;
            margin: 0 auto;
        }

        /* Header mit Gradient */
        .header {
            background: linear-gradient(135deg, var(--primary) 0%, var(--secondary) 100%);
            color: white;
            padding: 25px;
            border-radius: 20px;
            margin-bottom: 20px;
            box-shadow: 0 10px 30px rgba(30, 58, 138, 0.3);
            position: relative;
            overflow: hidden;
        }

        .header::before {
            content: '';
            position: absolute;
            top: -50%;
            right: -50%;
            width: 200%;
            height: 200%;
            background: radial-gradient(circle, rgba(255,255,255,0.1) 0%, transparent 70%);
            animation: float 20s infinite;
        }

        @keyframes float {
            0%, 100% { transform: translate(0, 0) rotate(0deg); }
            33% { transform: translate(30px, -30px) rotate(120deg); }
            66% { transform: translate(-30px, 30px) rotate(240deg); }
        }

        .header-content {
            position: relative;
            z-index: 1;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 15px;
        }

        .header h1 {
            font-size: 2em;
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .theme-toggle {
            background: rgba(255, 255, 255, 0.2);
            border: none;
            border-radius: 50px;
            padding: 8px 20px;
            color: white;
            cursor: pointer;
            font-size: 1.2em;
            transition: all 0.3s;
        }

        .theme-toggle:hover {
            background: rgba(255, 255, 255, 0.3);
            transform: scale(1.05);
        }

        .status-badge {
            display: inline-block;
            padding: 8px 20px;
            border-radius: 50px;
            font-weight: 600;
            font-size: 0.9em;
            animation: fadeIn 0.5s;
        }

        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(-10px); }
            to { opacity: 1; transform: translateY(0); }
        }

        .status-ok {
            background: var(--success);
            animation: fadeIn 0.5s;
        }

        .status-alarm {
            background: var(--danger);
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0%, 100% { opacity: 1; transform: scale(1); }
            50% { opacity: 0.8; transform: scale(1.05); }
        }

        /* Card Grid */
        .cards-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 15px;
            margin-bottom: 20px;
        }

        .card {
            background: var(--card-bg);
            border-radius: 15px;
            padding: 20px;
            box-shadow: 0 4px 6px var(--shadow);
            transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
            border: 1px solid var(--border);
            position: relative;
            overflow: hidden;
        }

        .card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 3px;
            background: linear-gradient(90deg, var(--primary), var(--secondary));
            transform: scaleX(0);
            transition: transform 0.3s;
        }

        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 8px 15px var(--shadow);
        }

        .card:hover::before {
            transform: scaleX(1);
        }

        .card-header {
            display: flex;
            align-items: center;
            gap: 10px;
            margin-bottom: 12px;
            color: var(--text);
            opacity: 0.7;
            font-size: 0.85em;
            font-weight: 600;
            text-transform: uppercase;
        }

        .card-icon {
            font-size: 1.5em;
        }

        .card-value {
            font-size: 2.2em;
            font-weight: 700;
            color: var(--secondary);
            margin-bottom: 5px;
            transition: all 0.3s;
        }

        .card-value.updating {
            animation: valueUpdate 0.5s;
        }

        @keyframes valueUpdate {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.1); }
        }

        .card-unit {
            color: var(--text);
            opacity: 0.6;
            font-size: 0.9em;
        }

        .card-status {
            margin-top: 10px;
            padding: 6px 12px;
            border-radius: 8px;
            font-size: 0.8em;
            font-weight: 600;
            text-align: center;
            transition: all 0.3s;
        }

        .status-normal {
            background: rgba(16, 185, 129, 0.1);
            color: var(--success);
            border: 1px solid rgba(16, 185, 129, 0.3);
        }

        .status-warning {
            background: rgba(245, 158, 11, 0.1);
            color: var(--warning);
            border: 1px solid rgba(245, 158, 11, 0.3);
        }

        .status-critical {
            background: rgba(239, 68, 68, 0.1);
            color: var(--danger);
            border: 1px solid rgba(239, 68, 68, 0.3);
            animation: pulse 2s infinite;
        }

        /* Stats Section */
        .stats-section {
            background: var(--card-bg);
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 20px;
            border: 1px solid var(--border);
        }

        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-top: 15px;
        }

        .stat-item {
            padding: 15px;
            background: var(--bg);
            border-radius: 10px;
            border: 1px solid var(--border);
        }

        .stat-label {
            font-size: 0.8em;
            opacity: 0.7;
            margin-bottom: 5px;
        }

        .stat-value {
            font-size: 1.3em;
            font-weight: 600;
            color: var(--secondary);
        }

        /* Controls */
        .controls-section {
            background: var(--card-bg);
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 20px;
            border: 1px solid var(--border);
        }

        .control-group {
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
            margin-top: 15px;
        }

        .btn {
            padding: 12px 24px;
            border: none;
            border-radius: 10px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            font-size: 0.95em;
            position: relative;
            overflow: hidden;
        }

        .btn::before {
            content: '';
            position: absolute;
            top: 50%;
            left: 50%;
            width: 0;
            height: 0;
            border-radius: 50%;
            background: rgba(255, 255, 255, 0.3);
            transform: translate(-50%, -50%);
            transition: width 0.6s, height 0.6s;
        }

        .btn:active::before {
            width: 300px;
            height: 300px;
        }

        .btn-success {
            background: var(--success);
            color: white;
        }

        .btn-success:hover {
            background: #059669;
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(16, 185, 129, 0.4);
        }

        .btn-danger {
            background: var(--danger);
            color: white;
        }

        .btn-danger:hover {
            background: #dc2626;
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(239, 68, 68, 0.4);
        }

        .btn-primary {
            background: var(--secondary);
            color: white;
        }

        .btn-primary:hover {
            background: var(--primary);
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(59, 130, 246, 0.4);
        }

        /* Connection Status */
        .connection-status {
            position: fixed;
            bottom: 20px;
            right: 20px;
            padding: 10px 20px;
            border-radius: 50px;
            background: var(--card-bg);
            border: 1px solid var(--border);
            display: flex;
            align-items: center;
            gap: 8px;
            font-size: 0.85em;
            box-shadow: 0 4px 12px var(--shadow);
            z-index: 1000;
        }

        .status-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background: var(--success);
            animation: blink 2s infinite;
        }

        .status-dot.disconnected {
            background: var(--danger);
            animation: none;
        }

        @keyframes blink {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }

        /* Footer */
        .footer {
            text-align: center;
            padding: 20px;
            opacity: 0.6;
            font-size: 0.85em;
        }

        /* Responsive */
        @media (max-width: 768px) {
            .header h1 {
                font-size: 1.5em;
            }

            .cards-grid {
                grid-template-columns: 1fr;
            }

            .connection-status {
                bottom: 10px;
                right: 10px;
                font-size: 0.75em;
            }
        }

        /* Loading Animation */
        .loading {
            display: inline-block;
            width: 20px;
            height: 20px;
            border: 3px solid rgba(255, 255, 255, 0.3);
            border-top-color: white;
            border-radius: 50%;
            animation: spin 1s linear infinite;
        }

        @keyframes spin {
            to { transform: rotate(360deg); }
        }

        /* Tooltip */
        [data-tooltip] {
            position: relative;
            cursor: help;
        }

        [data-tooltip]::after {
            content: attr(data-tooltip);
            position: absolute;
            bottom: 100%;
            left: 50%;
            transform: translateX(-50%) translateY(-5px);
            padding: 5px 10px;
            background: var(--card-bg);
            border: 1px solid var(--border);
            border-radius: 5px;
            font-size: 0.8em;
            white-space: nowrap;
            opacity: 0;
            pointer-events: none;
            transition: opacity 0.3s;
        }

        [data-tooltip]:hover::after {
            opacity: 1;
        }
    </style>
</head>
<body data-theme="dark">
    <div class="container">
        <!-- Header -->
        <div class="header">
            <div class="header-content">
                <h1>
                    <span>🐟</span>
                    <span>ForellenWächter v2.1</span>
                </h1>
                <button class="theme-toggle" onclick="toggleTheme()" aria-label="Theme umschalten">
                    <span id="theme-icon">☀️</span>
                </button>
            </div>
            <div style="margin-top: 10px;">
                <div id="systemStatus" class="status-badge status-ok">System OK</div>
            </div>
        </div>

        <!-- Sensor Cards -->
        <div class="cards-grid">
            <div class="card">
                <div class="card-header">
                    <span class="card-icon">💧</span>
                    <span>Wassertemperatur</span>
                </div>
                <div class="card-value" id="waterTemp">--</div>
                <div class="card-unit">°C</div>
                <div id="waterTempStatus" class="card-status status-normal">Normal</div>
            </div>

            <div class="card">
                <div class="card-header">
                    <span class="card-icon">🌡️</span>
                    <span>Lufttemperatur</span>
                </div>
                <div class="card-value" id="airTemp">--</div>
                <div class="card-unit">°C</div>
                <div id="airTempStatus" class="card-status status-normal">Normal</div>
            </div>

            <div class="card">
                <div class="card-header">
                    <span class="card-icon">⚗️</span>
                    <span>pH-Wert</span>
                </div>
                <div class="card-value" id="pH">--</div>
                <div class="card-unit">pH</div>
                <div id="pHStatus" class="card-status status-normal">Normal</div>
            </div>

            <div class="card">
                <div class="card-header">
                    <span class="card-icon">💎</span>
                    <span>TDS</span>
                </div>
                <div class="card-value" id="tds">--</div>
                <div class="card-unit">ppm</div>
                <div id="tdsStatus" class="card-status status-normal">Normal</div>
            </div>

            <div class="card">
                <div class="card-header">
                    <span class="card-icon">🌊</span>
                    <span>Wasserstand</span>
                </div>
                <div class="card-value" id="waterLevel" style="font-size: 1.5em;">--</div>
                <div id="waterLevelStatus" class="card-status status-normal">Normal</div>
            </div>

            <div class="card">
                <div class="card-header">
                    <span class="card-icon">🫧</span>
                    <span>Belüftung</span>
                </div>
                <div class="card-value" id="aeration" style="font-size: 1.5em;">--</div>
                <div id="aerationStatus" class="card-status status-normal">Aus</div>
            </div>
        </div>

        <!-- Statistics -->
        <div class="stats-section">
            <h2 style="margin-bottom: 10px;">📊 24h Statistiken</h2>
            <div class="stats-grid" id="statsGrid">
                <!-- Dynamisch gefüllt -->
            </div>
        </div>

        <!-- Controls -->
        <div class="controls-section">
            <h2 style="margin-bottom: 10px;">⚙️ Steuerung</h2>
            <div class="control-group">
                <button class="btn btn-success" onclick="controlAeration('on')">
                    ▶️ Belüftung AN
                </button>
                <button class="btn btn-danger" onclick="controlAeration('off')">
                    ⏸️ Belüftung AUS
                </button>
                <button class="btn btn-primary" onclick="resetAlarm()">
                    🔄 Alarm Zurücksetzen
                </button>
                <button class="btn btn-primary" onclick="downloadData()">
                    📥 Daten Download
                </button>
            </div>
        </div>

        <!-- Footer -->
        <div class="footer">
            <p>ForellenWächter v2.1 • Lucas Haug's Forellenzucht</p>
            <p style="margin-top: 5px;">Entwickelt von Andreas S. • Off-Grid Optimiert • -67% Stromverbrauch</p>
        </div>
    </div>

    <!-- Connection Status -->
    <div class="connection-status">
        <div class="status-dot" id="statusDot"></div>
        <span id="connectionText">Verbunden</span>
    </div>

    <script>
        let ws;
        let reconnectInterval;
        let statsUpdateInterval;

        // Theme Management
        function toggleTheme() {
            const body = document.body;
            const icon = document.getElementById('theme-icon');
            const currentTheme = body.getAttribute('data-theme');
            const newTheme = currentTheme === 'dark' ? 'light' : 'dark';

            body.setAttribute('data-theme', newTheme);
            icon.textContent = newTheme === 'dark' ? '☀️' : '🌙';
            localStorage.setItem('theme', newTheme);
        }

        // Theme aus LocalStorage laden
        const savedTheme = localStorage.getItem('theme') || 'dark';
        document.body.setAttribute('data-theme', savedTheme);
        document.getElementById('theme-icon').textContent = savedTheme === 'dark' ? '☀️' : '🌙';

        // WebSocket Verbindung
        function connectWebSocket() {
            const wsUrl = 'ws://' + window.location.hostname + ':81';
            ws = new WebSocket(wsUrl);

            ws.onopen = () => {
                console.log('✓ WebSocket verbunden');
                updateConnectionStatus(true);
                if (reconnectInterval) {
                    clearInterval(reconnectInterval);
                }
            };

            ws.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);
                    updateDisplay(data);
                } catch (e) {
                    console.error('Fehler beim Parsen der Daten:', e);
                }
            };

            ws.onclose = () => {
                console.log('✗ WebSocket getrennt');
                updateConnectionStatus(false);
                reconnectInterval = setInterval(() => {
                    console.log('Versuche Neuverbindung...');
                    connectWebSocket();
                }, 3000);
            };

            ws.onerror = (error) => {
                console.error('WebSocket Fehler:', error);
            };
        }

        function updateConnectionStatus(connected) {
            const dot = document.getElementById('statusDot');
            const text = document.getElementById('connectionText');

            if (connected) {
                dot.classList.remove('disconnected');
                text.textContent = 'Verbunden';
            } else {
                dot.classList.add('disconnected');
                text.textContent = 'Getrennt';
            }
        }

        function updateDisplay(data) {
            // Werte mit Animation aktualisieren
            updateValue('waterTemp', data.waterTemp.toFixed(1), TEMP_MIN, TEMP_MAX, 'waterTempStatus');
            updateValue('airTemp', data.airTemp.toFixed(1), -10, 40, 'airTempStatus');
            updateValue('pH', data.pH.toFixed(2), PH_MIN, PH_MAX, 'pHStatus');
            updateValue('tds', Math.round(data.tds), 0, TDS_MAX, 'tdsStatus');

            // Wasserstand
            const wl = document.getElementById('waterLevel');
            const wlStatus = document.getElementById('waterLevelStatus');
            wl.textContent = data.waterLevel ? 'OK ✓' : 'NIEDRIG ⚠';
            wlStatus.textContent = data.waterLevel ? 'Normal' : 'WARNUNG';
            wlStatus.className = 'card-status ' + (data.waterLevel ? 'status-normal' : 'status-critical');

            // Belüftung
            const aer = document.getElementById('aeration');
            const aerStatus = document.getElementById('aerationStatus');
            aer.textContent = data.aeration ? 'AN ⚡' : 'AUS';
            aerStatus.textContent = data.aeration ? 'Aktiv' : 'Aus';
            aerStatus.className = 'card-status ' + (data.aeration ? 'status-normal' : 'status-warning');

            // System Status
            const systemStatus = document.getElementById('systemStatus');
            if (data.alarm) {
                systemStatus.textContent = '⚠ ALARM AKTIV';
                systemStatus.className = 'status-badge status-alarm';
            } else {
                systemStatus.textContent = '✓ System OK';
                systemStatus.className = 'status-badge status-ok';
            }
        }

        function updateValue(elementId, value, min, max, statusId) {
            const element = document.getElementById(elementId);
            const statusElement = document.getElementById(statusId);

            // Animation
            element.classList.add('updating');
            setTimeout(() => element.classList.remove('updating'), 500);

            element.textContent = value;

            // Status
            const numValue = parseFloat(value);
            if (numValue < min || numValue > max) {
                statusElement.textContent = 'Warnung';
                statusElement.className = 'card-status status-critical';
            } else {
                statusElement.textContent = 'Normal';
                statusElement.className = 'card-status status-normal';
            }
        }

        // Steuerungsfunktionen
        function controlAeration(action) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send('aeration_' + action);
            }
        }

        function resetAlarm() {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send('reset_alarm');
            }
        }

        function downloadData() {
            window.location.href = '/api/download';
        }

        // Statistiken laden
        function loadStats() {
            fetch('/api/stats')
                .then(r => r.json())
                .then(data => {
                    const grid = document.getElementById('statsGrid');
                    grid.innerHTML = `
                        <div class="stat-item">
                            <div class="stat-label">Wasser Min/Max</div>
                            <div class="stat-value">${data.waterTemp.min}° / ${data.waterTemp.max}°</div>
                        </div>
                        <div class="stat-item">
                            <div class="stat-label">pH Min/Max</div>
                            <div class="stat-value">${data.pH.min} / ${data.pH.max}</div>
                        </div>
                        <div class="stat-item">
                            <div class="stat-label">TDS Durchschnitt</div>
                            <div class="stat-value">${Math.round(data.tds.avg)} ppm</div>
                        </div>
                        <div class="stat-item">
                            <div class="stat-label">Luft Durchschnitt</div>
                            <div class="stat-value">${data.airTemp.avg}°C</div>
                        </div>
                    `;
                })
                .catch(e => console.error('Fehler beim Laden der Statistiken:', e));
        }

        // Grenzwerte (vom Server)
        const TEMP_MIN = )=====" + String(TEMP_MIN) + R"=====(;
        const TEMP_MAX = )=====" + String(TEMP_MAX) + R"=====(;
        const PH_MIN = )=====" + String(PH_MIN) + R"=====(;
        const PH_MAX = )=====" + String(PH_MAX) + R"=====(;
        const TDS_MAX = )=====" + String(TDS_MAX) + R"=====(;

        // Initialisierung
        window.addEventListener('load', () => {
            connectWebSocket();
            loadStats();
            statsUpdateInterval = setInterval(loadStats, 60000); // Alle 60s
        });

        // PWA Installation
        let deferredPrompt;
        window.addEventListener('beforeinstallprompt', (e) => {
            e.preventDefault();
            deferredPrompt = e;
            console.log('PWA Installation verfügbar');
        });
    </script>
</body>
</html>
)=====";

// PWA Manifest
const char MANIFEST_JSON[] PROGMEM = R"=====(
{
  "name": "ForellenWächter v2.1",
  "short_name": "Forellen",
  "description": "Aquakultur Monitoring System - Off-Grid Optimiert",
  "start_url": "/",
  "display": "standalone",
  "background_color": "#0f172a",
  "theme_color": "#1e3a8a",
  "orientation": "portrait",
  "icons": [
    {
      "src": "/icon-192.png",
      "sizes": "192x192",
      "type": "image/png"
    },
    {
      "src": "/icon-512.png",
      "sizes": "512x512",
      "type": "image/png"
    }
  ]
}
)=====";

// ========== HANDLER FUNKTIONEN ==========

void handleRoot() {
    server.send_P(200, "text/html", HTML_PAGE);
}

void handleManifest() {
    server.send_P(200, "application/json", MANIFEST_JSON);
}

void handleAPIData() {
    // SECURITY v2.1: Rate Limiting
    #if ENABLE_RATE_LIMITING
    if (!rateLimiter.allowRequest(server.client().remoteIP())) {
        server.send(429, "application/json", "{\"error\":\"Too many requests\"}");
        return;
    }
    #endif

    SensorData data = sensorManager.getData();

    // OPTIMIERT v2.1: snprintf statt String-Konkatenation
    char json[512];
    int written = snprintf(json, sizeof(json),
        "{\"waterTemp\":%.2f,\"airTemp\":%.2f,\"pH\":%.2f,\"tds\":%.0f,"
        "\"waterLevel\":%s,\"alarm\":%s,\"aeration\":%s",
        data.waterTemp,
        data.airTemp,
        data.pH,
        data.tds,
        data.waterLevel ? "true" : "false",
        alarmActive ? "true" : "false",
        aerationActive ? "true" : "false"
    );

    // Optional: DO-Sensor Werte (v2.1)
    #if ENABLE_DO_SENSOR
    if (data.valid) {
        written += snprintf(json + written, sizeof(json) - written,
            ",\"dissolvedOxygen\":%.2f,\"doSaturation\":%.1f",
            data.dissolvedOxygen,
            data.doSaturation
        );
    }
    #endif

    // Optional: INA219 Power Monitor (v2.1)
    #if ENABLE_INA219
    if (powerMonitor.isInitialized()) {
        written += snprintf(json + written, sizeof(json) - written,
            ",\"voltage\":%.2f,\"current\":%.1f,\"power\":%.1f",
            powerMonitor.getVoltage(),
            powerMonitor.getCurrent(),
            powerMonitor.getPower()
        );
    }
    #endif

    // JSON abschließen
    snprintf(json + written, sizeof(json) - written, "}");

    server.send(200, "application/json", json);
}

void handleAPIStats() {
    // SECURITY v2.1: Rate Limiting
    #if ENABLE_RATE_LIMITING
    if (!rateLimiter.allowRequest(server.client().remoteIP())) {
        server.send(429, "application/json", "{\"error\":\"Too many requests\"}");
        return;
    }
    #endif

    SensorStats waterTemp = sensorManager.getWaterTempStats();
    SensorStats airTemp = sensorManager.getAirTempStats();
    SensorStats pH = sensorManager.getPHStats();
    SensorStats tds = sensorManager.getTDSStats();

    // OPTIMIERT v2.1: snprintf statt String-Konkatenation
    char json[384];
    snprintf(json, sizeof(json),
        "{\"waterTemp\":{\"min\":%.1f,\"max\":%.1f,\"avg\":%.1f},"
        "\"airTemp\":{\"min\":%.1f,\"max\":%.1f,\"avg\":%.1f},"
        "\"pH\":{\"min\":%.2f,\"max\":%.2f,\"avg\":%.2f},"
        "\"tds\":{\"min\":%.0f,\"max\":%.0f,\"avg\":%.0f}}",
        waterTemp.minValue, waterTemp.maxValue, waterTemp.avgValue,
        airTemp.minValue, airTemp.maxValue, airTemp.avgValue,
        pH.minValue, pH.maxValue, pH.avgValue,
        tds.minValue, tds.maxValue, tds.avgValue
    );

    server.send(200, "application/json", json);
}

// SECURITY v2.1: Download Handler mit Path Traversal Protection
void handleAPIDownload() {
    #if ENABLE_RATE_LIMITING
    if (!rateLimiter.allowRequest(server.client().remoteIP())) {
        server.send(429, "text/plain", "Too many requests");
        return;
    }
    #endif

    #if LOG_TO_SD
    extern bool sdCardAvailable;
    extern String currentLogFile;

    if (!sdCardAvailable) {
        server.send(404, "text/plain", "SD-Karte nicht verfügbar");
        return;
    }

    // SECURITY: Path Traversal Protection
    String filename = currentLogFile;
    if (filename.indexOf("..") >= 0 || !filename.startsWith("/logs/")) {
        Serial.println("⚠ SECURITY: Path Traversal Versuch blockiert!");
        server.send(403, "text/plain", "Forbidden");
        return;
    }

    File file = SD.open(filename.c_str(), FILE_READ);
    if (!file) {
        server.send(404, "text/plain", "Datei nicht gefunden");
        return;
    }

    server.sendHeader("Content-Disposition", "attachment; filename=forellen_data.csv");
    server.streamFile(file, "text/csv");
    file.close();
    #else
    server.send(501, "text/plain", "SD-Logging deaktiviert");
    #endif
}

// Optional: INA219 API (v2.1)
#if ENABLE_INA219
void handleAPIPower() {
    #if ENABLE_RATE_LIMITING
    if (!rateLimiter.allowRequest(server.client().remoteIP())) {
        server.send(429, "application/json", "{\"error\":\"Too many requests\"}");
        return;
    }
    #endif

    if (!powerMonitor.isInitialized()) {
        server.send(200, "application/json", "{}");
        return;
    }

    server.send(200, "application/json", powerMonitor.getJSON());
}
#endif

void setupWebServer() {
    // SECURITY v2.1: Rate Limiting für alle Endpunkte
    server.on("/", HTTP_GET, []() {
        #if ENABLE_RATE_LIMITING
        if (!rateLimiter.allowRequest(server.client().remoteIP())) {
            server.send(429, "text/html", "<h1>429 Too Many Requests</h1>");
            return;
        }
        #endif
        handleRoot();
    });

    server.on("/manifest.json", HTTP_GET, handleManifest);
    server.on("/api/data", HTTP_GET, handleAPIData);
    server.on("/api/stats", HTTP_GET, handleAPIStats);
    server.on("/api/download", HTTP_GET, handleAPIDownload);

    #if ENABLE_INA219
    server.on("/api/power", HTTP_GET, handleAPIPower);
    #endif

    // 404 Handler
    server.onNotFound([]() {
        server.send(404, "text/plain", "404 Not Found");
    });

    server.begin();
    Serial.print("✓ Web Server gestartet auf Port ");
    Serial.println(WEB_SERVER_PORT);

    webSocket.begin();
    Serial.print("✓ WebSocket gestartet auf Port ");
    Serial.println(WEBSOCKET_PORT);
}

#endif // WEBSERVER_H
