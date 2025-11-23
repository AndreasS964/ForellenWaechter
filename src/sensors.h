/*
 * ForellenWächter v2.0 - Sensor Management
 * Optimierte Sensor-Auslesungen mit Fehlerbehandlung
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

// Sensor-Datenstruktur
struct SensorData {
    float waterTemp;
    float airTemp;
    float pH;
    float tds;
    bool waterLevel;
    unsigned long timestamp;
    bool valid;
};

// Sensor-Statistiken
struct SensorStats {
    float minValue;
    float maxValue;
    float avgValue;
    int sampleCount;
};

class SensorManager {
private:
    OneWire* oneWire;
    DallasTemperature* tempSensors;
    DeviceAddress waterThermometer, airThermometer;

    SensorData currentData;
    bool sensorsInitialized;

    // Statistiken (24h)
    SensorStats waterTempStats;
    SensorStats airTempStats;
    SensorStats pHStats;
    SensorStats tdsStats;

    // Fehler-Tracking
    int tempSensorErrors;
    int phSensorErrors;
    int tdsSensorErrors;

    // Kalibrierungswerte (können zur Laufzeit geändert werden)
    float phNeutralVoltage;
    float phAcidVoltage;
    float tdsCalibrationFactor;

public:
    SensorManager() :
        oneWire(nullptr),
        tempSensors(nullptr),
        sensorsInitialized(false),
        tempSensorErrors(0),
        phSensorErrors(0),
        tdsSensorErrors(0),
        phNeutralVoltage(PH_NEUTRAL_VOLTAGE),
        phAcidVoltage(PH_ACID_VOLTAGE),
        tdsCalibrationFactor(1.0) {

        currentData.valid = false;
        resetStats();
    }

    ~SensorManager() {
        if (tempSensors) delete tempSensors;
        if (oneWire) delete oneWire;
    }

    // Initialisierung
    bool begin() {
        Serial.println("\n=== Sensor-Initialisierung ===");

        // Pin Modes
        pinMode(WATER_LEVEL_PIN, INPUT_PULLUP);

        // OneWire initialisieren
        oneWire = new OneWire(ONE_WIRE_BUS);
        tempSensors = new DallasTemperature(oneWire);

        tempSensors->begin();
        int deviceCount = tempSensors->getDeviceCount();

        Serial.printf("DS18B20 Sensoren gefunden: %d\n", deviceCount);

        if (deviceCount >= 2) {
            tempSensors->getAddress(waterThermometer, 0);
            tempSensors->getAddress(airThermometer, 1);

            // Auflösung setzen (9-12 bit, höher = genauer aber langsamer)
            tempSensors->setResolution(waterThermometer, 11);  // 11 bit für Balance
            tempSensors->setResolution(airThermometer, 11);

            Serial.println("✓ Temperatursensoren konfiguriert (11-bit)");
            sensorsInitialized = true;
        } else {
            Serial.println("⚠ WARNUNG: Nicht genug Temperatursensoren gefunden!");
            sensorsInitialized = false;
        }

        // ADC Konfiguration
        analogReadResolution(12);  // 12-bit ADC (0-4095)
        analogSetAttenuation(ADC_11db);  // 0-3.3V

        Serial.println("✓ ADC konfiguriert (12-bit, 0-3.3V)");
        Serial.println("✓ Sensor-Manager initialisiert\n");

        return sensorsInitialized;
    }

    // Alle Sensoren auslesen
    bool readAll() {
        if (!sensorsInitialized) {
            currentData.valid = false;
            return false;
        }

        // Temperatur
        if (!readTemperatures()) {
            tempSensorErrors++;
        }

        // pH
        if (!readPH()) {
            phSensorErrors++;
        }

        // TDS
        if (!readTDS()) {
            tdsSensorErrors++;
        }

        // Wasserstand
        currentData.waterLevel = readWaterLevel();

        // Timestamp
        currentData.timestamp = millis();
        currentData.valid = true;

        // Statistiken aktualisieren
        updateStats();

        return true;
    }

    // Temperatur-Sensoren auslesen
    bool readTemperatures() {
        tempSensors->requestTemperatures();

        currentData.waterTemp = tempSensors->getTempC(waterThermometer);
        currentData.airTemp = tempSensors->getTempC(airThermometer);

        // Validierung
        if (currentData.waterTemp == DEVICE_DISCONNECTED_C ||
            currentData.waterTemp < -50 || currentData.waterTemp > 50) {
            Serial.println("⚠ Wassertemperatur-Sensor Fehler");
            currentData.waterTemp = 0.0;
            return false;
        }

        if (currentData.airTemp == DEVICE_DISCONNECTED_C ||
            currentData.airTemp < -50 || currentData.airTemp > 60) {
            Serial.println("⚠ Lufttemperatur-Sensor Fehler");
            currentData.airTemp = 0.0;
            return false;
        }

        return true;
    }

    // pH-Sensor auslesen (mit Median-Filter)
    bool readPH() {
        const int samples = 10;
        int readings[samples];

        // Mehrere Messungen für Median
        for (int i = 0; i < samples; i++) {
            readings[i] = analogRead(PH_PIN);
            delay(10);
        }

        // Median berechnen (robuster gegen Ausreißer)
        for (int i = 0; i < samples - 1; i++) {
            for (int j = i + 1; j < samples; j++) {
                if (readings[i] > readings[j]) {
                    int temp = readings[i];
                    readings[i] = readings[j];
                    readings[j] = temp;
                }
            }
        }

        int medianRaw = readings[samples / 2];
        float voltage = (medianRaw / 4095.0) * 3.3 * 1000.0;  // in mV

        // pH berechnen
        currentData.pH = 7.0 - ((voltage - phNeutralVoltage) /
                        (phAcidVoltage - phNeutralVoltage)) * 3.0;

        // Validierung
        if (currentData.pH < 0 || currentData.pH > 14) {
            Serial.println("⚠ pH-Sensor Fehler (außerhalb 0-14)");
            currentData.pH = 7.0;
            return false;
        }

        return true;
    }

    // TDS-Sensor auslesen (temperaturkompensiert)
    bool readTDS() {
        int tdsBuffer[TDS_SCOUNT];

        // Samples sammeln
        for (int i = 0; i < TDS_SCOUNT; i++) {
            tdsBuffer[i] = analogRead(TDS_PIN);
            delay(10);
        }

        // Median berechnen
        for (int i = 0; i < TDS_SCOUNT - 1; i++) {
            for (int j = i + 1; j < TDS_SCOUNT; j++) {
                if (tdsBuffer[i] > tdsBuffer[j]) {
                    int temp = tdsBuffer[i];
                    tdsBuffer[i] = tdsBuffer[j];
                    tdsBuffer[j] = temp;
                }
            }
        }

        int medianValue = tdsBuffer[TDS_SCOUNT / 2];
        float voltage = (medianValue / 4095.0) * TDS_VREF;

        // Temperaturkompensation (nur wenn Wassertemperatur gültig)
        float compensationCoefficient = 1.0;
        if (currentData.waterTemp > 0 && currentData.waterTemp < 40) {
            compensationCoefficient = 1.0 + 0.02 * (currentData.waterTemp - 25.0);
        }

        float compensatedVoltage = voltage / compensationCoefficient;

        // TDS Berechnung
        currentData.tds = (133.42 * compensatedVoltage * compensatedVoltage * compensatedVoltage
                         - 255.86 * compensatedVoltage * compensatedVoltage
                         + 857.39 * compensatedVoltage) * 0.5 * tdsCalibrationFactor;

        // Validierung
        if (currentData.tds < 0 || currentData.tds > 2000) {
            Serial.println("⚠ TDS-Sensor Fehler (außerhalb 0-2000)");
            currentData.tds = 0.0;
            return false;
        }

        return true;
    }

    // Wasserstand auslesen
    bool readWaterLevel() {
        // LOW = Wasser vorhanden bei Pull-Up Konfiguration
        return (digitalRead(WATER_LEVEL_PIN) == LOW);
    }

    // Aktuelle Daten abrufen
    SensorData getData() {
        return currentData;
    }

    // Statistiken aktualisieren
    void updateStats() {
        updateSingleStat(waterTempStats, currentData.waterTemp);
        updateSingleStat(airTempStats, currentData.airTemp);
        updateSingleStat(pHStats, currentData.pH);
        updateSingleStat(tdsStats, currentData.tds);
    }

    void updateSingleStat(SensorStats& stat, float value) {
        if (stat.sampleCount == 0) {
            stat.minValue = value;
            stat.maxValue = value;
            stat.avgValue = value;
            stat.sampleCount = 1;
        } else {
            if (value < stat.minValue) stat.minValue = value;
            if (value > stat.maxValue) stat.maxValue = value;
            stat.avgValue = (stat.avgValue * stat.sampleCount + value) / (stat.sampleCount + 1);
            stat.sampleCount++;
        }
    }

    // Statistiken zurücksetzen
    void resetStats() {
        waterTempStats = {0, 0, 0, 0};
        airTempStats = {0, 0, 0, 0};
        pHStats = {0, 0, 0, 0};
        tdsStats = {0, 0, 0, 0};
    }

    // Statistiken abrufen
    SensorStats getWaterTempStats() { return waterTempStats; }
    SensorStats getAirTempStats() { return airTempStats; }
    SensorStats getPHStats() { return pHStats; }
    SensorStats getTDSStats() { return tdsStats; }

    // Kalibrierung
    void calibratePH(float neutralVoltage, float acidVoltage) {
        phNeutralVoltage = neutralVoltage;
        phAcidVoltage = acidVoltage;
        Serial.printf("pH kalibriert: Neutral=%.1fmV, Sauer=%.1fmV\n", neutralVoltage, acidVoltage);
    }

    void calibrateTDS(float factor) {
        tdsCalibrationFactor = factor;
        Serial.printf("TDS Kalibrierfaktor: %.2f\n", factor);
    }

    // Fehler-Statistiken
    void printErrorStats() {
        Serial.println("\n=== Sensor-Fehlerstatistik ===");
        Serial.printf("Temperatur-Fehler: %d\n", tempSensorErrors);
        Serial.printf("pH-Fehler: %d\n", phSensorErrors);
        Serial.printf("TDS-Fehler: %d\n", tdsSensorErrors);
        Serial.println();
    }

    // Alle Sensorwerte ausgeben
    void printSensorData() {
        Serial.println("\n=== Aktuelle Sensorwerte ===");
        Serial.printf("Wassertemperatur: %.2f°C\n", currentData.waterTemp);
        Serial.printf("Lufttemperatur: %.2f°C\n", currentData.airTemp);
        Serial.printf("pH-Wert: %.2f\n", currentData.pH);
        Serial.printf("TDS: %.0f ppm\n", currentData.tds);
        Serial.printf("Wasserstand: %s\n", currentData.waterLevel ? "OK" : "NIEDRIG");
        Serial.println();
    }
};

#endif // SENSORS_H
