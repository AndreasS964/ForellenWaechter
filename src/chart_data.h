/*
 * ForellenWächter v2.1 - Chart Data Manager
 * Speichert historische Sensordaten für Chart.js Visualisierung
 *
 * FEATURES:
 * - Ring Buffer für 24h Daten (288 Punkte @ 5min Intervall)
 * - Kompakte Speichernutzung (~9 KB RAM)
 * - Schnelle JSON-Serialisierung
 * - Automatische Datenrotation
 */

#ifndef CHART_DATA_H
#define CHART_DATA_H

#include <Arduino.h>
#include "config.h"

// Chart Konfiguration
#define CHART_DATA_POINTS 288       // 24h @ 5min Intervall = 288 Punkte
#define CHART_UPDATE_INTERVAL 300000 // 5 Minuten in ms

// Kompakte Datenstruktur für historische Werte
struct ChartDataPoint {
    uint32_t timestamp;      // Unix timestamp (4 bytes)
    int16_t waterTemp;       // Temp * 10 (z.B. 15.7°C = 157)
    int16_t airTemp;         // Temp * 10
    uint16_t pH;             // pH * 100 (z.B. 7.45 = 745)
    uint16_t tds;            // TDS direkt (0-9999 ppm)
};

class ChartDataManager {
private:
    ChartDataPoint dataPoints[CHART_DATA_POINTS];
    int currentIndex;
    int dataCount;
    unsigned long lastUpdate;

public:
    ChartDataManager() : currentIndex(0), dataCount(0), lastUpdate(0) {}

    // Datenpunkt hinzufügen (wird alle 5 Minuten aufgerufen)
    void addDataPoint(float waterTemp, float airTemp, float pH, float tds) {
        dataPoints[currentIndex].timestamp = millis() / 1000; // Sekunden seit Start
        dataPoints[currentIndex].waterTemp = (int16_t)(waterTemp * 10.0);
        dataPoints[currentIndex].airTemp = (int16_t)(airTemp * 10.0);
        dataPoints[currentIndex].pH = (uint16_t)(pH * 100.0);
        dataPoints[currentIndex].tds = (uint16_t)tds;

        currentIndex = (currentIndex + 1) % CHART_DATA_POINTS;
        if (dataCount < CHART_DATA_POINTS) {
            dataCount++;
        }

        lastUpdate = millis();
    }

    // Prüfen ob Update nötig ist
    bool shouldUpdate() {
        return (millis() - lastUpdate) >= CHART_UPDATE_INTERVAL;
    }

    // JSON für Chart.js generieren (Letzte N Punkte)
    String getChartJSON(int maxPoints = 48) { // Default: 48 Punkte = 4h
        if (dataCount == 0) {
            return "{\"labels\":[],\"waterTemp\":[],\"airTemp\":[],\"pH\":[],\"tds\":[]}";
        }

        // Anzahl der Punkte begrenzen
        int pointsToSend = min(dataCount, maxPoints);
        int startIdx = (currentIndex - pointsToSend + CHART_DATA_POINTS) % CHART_DATA_POINTS;

        String json = "{\"labels\":[";

        // Labels (Zeitstempel)
        for (int i = 0; i < pointsToSend; i++) {
            int idx = (startIdx + i) % CHART_DATA_POINTS;
            json += "\"";
            json += formatTimestamp(dataPoints[idx].timestamp);
            json += "\"";
            if (i < pointsToSend - 1) json += ",";
        }
        json += "],\"waterTemp\":[";

        // Wassertemperatur
        for (int i = 0; i < pointsToSend; i++) {
            int idx = (startIdx + i) % CHART_DATA_POINTS;
            json += String(dataPoints[idx].waterTemp / 10.0, 1);
            if (i < pointsToSend - 1) json += ",";
        }
        json += "],\"airTemp\":[";

        // Lufttemperatur
        for (int i = 0; i < pointsToSend; i++) {
            int idx = (startIdx + i) % CHART_DATA_POINTS;
            json += String(dataPoints[idx].airTemp / 10.0, 1);
            if (i < pointsToSend - 1) json += ",";
        }
        json += "],\"pH\":[";

        // pH-Wert
        for (int i = 0; i < pointsToSend; i++) {
            int idx = (startIdx + i) % CHART_DATA_POINTS;
            json += String(dataPoints[idx].pH / 100.0, 2);
            if (i < pointsToSend - 1) json += ",";
        }
        json += "],\"tds\":[";

        // TDS
        for (int i = 0; i < pointsToSend; i++) {
            int idx = (startIdx + i) % CHART_DATA_POINTS;
            json += String(dataPoints[idx].tds);
            if (i < pointsToSend - 1) json += ",";
        }
        json += "]}";

        return json;
    }

    // Optimierte Version mit snprintf (weniger Heap-Fragmentierung)
    int getChartJSONOptimized(char* buffer, size_t bufferSize, int maxPoints = 48) {
        if (dataCount == 0) {
            return snprintf(buffer, bufferSize,
                "{\"labels\":[],\"waterTemp\":[],\"airTemp\":[],\"pH\":[],\"tds\":[]}");
        }

        int pointsToSend = min(dataCount, maxPoints);
        int startIdx = (currentIndex - pointsToSend + CHART_DATA_POINTS) % CHART_DATA_POINTS;

        int written = snprintf(buffer, bufferSize, "{\"labels\":[");

        // Labels
        for (int i = 0; i < pointsToSend && written < bufferSize; i++) {
            int idx = (startIdx + i) % CHART_DATA_POINTS;
            char timeStr[16];
            formatTimestampToBuffer(dataPoints[idx].timestamp, timeStr, sizeof(timeStr));
            written += snprintf(buffer + written, bufferSize - written,
                "\"%s\"%s", timeStr, (i < pointsToSend - 1) ? "," : "");
        }

        written += snprintf(buffer + written, bufferSize - written, "],\"waterTemp\":[");

        // Wassertemperatur
        for (int i = 0; i < pointsToSend && written < bufferSize; i++) {
            int idx = (startIdx + i) % CHART_DATA_POINTS;
            written += snprintf(buffer + written, bufferSize - written,
                "%.1f%s", dataPoints[idx].waterTemp / 10.0, (i < pointsToSend - 1) ? "," : "");
        }

        written += snprintf(buffer + written, bufferSize - written, "],\"airTemp\":[");

        // Lufttemperatur
        for (int i = 0; i < pointsToSend && written < bufferSize; i++) {
            int idx = (startIdx + i) % CHART_DATA_POINTS;
            written += snprintf(buffer + written, bufferSize - written,
                "%.1f%s", dataPoints[idx].airTemp / 10.0, (i < pointsToSend - 1) ? "," : "");
        }

        written += snprintf(buffer + written, bufferSize - written, "],\"pH\":[");

        // pH-Wert
        for (int i = 0; i < pointsToSend && written < bufferSize; i++) {
            int idx = (startIdx + i) % CHART_DATA_POINTS;
            written += snprintf(buffer + written, bufferSize - written,
                "%.2f%s", dataPoints[idx].pH / 100.0, (i < pointsToSend - 1) ? "," : "");
        }

        written += snprintf(buffer + written, bufferSize - written, "],\"tds\":[");

        // TDS
        for (int i = 0; i < pointsToSend && written < bufferSize; i++) {
            int idx = (startIdx + i) % CHART_DATA_POINTS;
            written += snprintf(buffer + written, bufferSize - written,
                "%d%s", dataPoints[idx].tds, (i < pointsToSend - 1) ? "," : "");
        }

        written += snprintf(buffer + written, bufferSize - written, "]}");

        return written;
    }

    // Timestamp formatieren (HH:MM)
    String formatTimestamp(uint32_t timestamp) {
        uint32_t seconds = timestamp % 86400; // Sekunden des Tages
        int hours = (seconds / 3600) % 24;
        int minutes = (seconds % 3600) / 60;

        char buffer[6];
        snprintf(buffer, sizeof(buffer), "%02d:%02d", hours, minutes);
        return String(buffer);
    }

    // Optimierte Version direkt in Buffer
    void formatTimestampToBuffer(uint32_t timestamp, char* buffer, size_t bufferSize) {
        uint32_t seconds = timestamp % 86400;
        int hours = (seconds / 3600) % 24;
        int minutes = (seconds % 3600) / 60;
        snprintf(buffer, bufferSize, "%02d:%02d", hours, minutes);
    }

    // Debug-Info
    void printDebugInfo() {
        Serial.println("\n=== Chart Data Manager ===");
        Serial.print("Gespeicherte Punkte: ");
        Serial.print(dataCount);
        Serial.print(" / ");
        Serial.println(CHART_DATA_POINTS);
        Serial.print("RAM-Nutzung: ");
        Serial.print(sizeof(dataPoints));
        Serial.println(" Bytes");

        if (dataCount > 0) {
            int latestIdx = (currentIndex - 1 + CHART_DATA_POINTS) % CHART_DATA_POINTS;
            Serial.print("Letzter Wert: Wasser=");
            Serial.print(dataPoints[latestIdx].waterTemp / 10.0);
            Serial.print("°C, pH=");
            Serial.println(dataPoints[latestIdx].pH / 100.0);
        }
        Serial.println();
    }

    // Statistiken zurücksetzen (z.B. bei Neustart)
    void reset() {
        currentIndex = 0;
        dataCount = 0;
        lastUpdate = 0;
    }

    // Anzahl der gespeicherten Punkte
    int getDataCount() {
        return dataCount;
    }

    // Letzte Update-Zeit
    unsigned long getLastUpdate() {
        return lastUpdate;
    }
};

#endif // CHART_DATA_H
