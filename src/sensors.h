/*
 * ForellenWächter v2.1 - Sensor Management
 * Optimierte Sensor-Auslesungen mit Fehlerbehandlung
 *
 * OPTIMIERUNGEN v2.1:
 * - DS18B20 auf 9-bit (75% schneller: 375ms→94ms)
 * - TDS Samples 30→15 (50% schneller)
 * - Optional: DO-Sensor Support
 * - Besseres Error-Handling
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

// Optionale Sensoren
#if ENABLE_DO_SENSOR
#include "do_sensor.h"
#endif

// Sensor-Datenstruktur
struct SensorData {
    float waterTemp;
    float airTemp;
    float pH;
    float tds;
    bool waterLevel;
    unsigned long timestamp;
    bool valid;

    #if ENABLE_DO_SENSOR
    float dissolvedOxygen;    // mg/L
    float doSaturation;       // %
    #endif
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

    #if ENABLE_DO_SENSOR
    DOSensor* doSensor;
    #endif

    // Statistiken (24h)
    SensorStats waterTempStats;
    SensorStats airTempStats;
    SensorStats pHStats;
    SensorStats tdsStats;

    // Fehler-Tracking
    int tempSensorErrors;
    int phSensorErrors;
    int tdsSensorErrors;
    #if ENABLE_DO_SENSOR
    int doSensorErrors;
    #endif

    // Kalibrierungswerte (können zur Laufzeit geändert werden)
    float phNeutralVoltage;
    float phAcidVoltage;
    float tdsCalibrationFactor;

    // Performance-Tracking
    unsigned long lastReadDuration;
    unsigned long totalReadTime;
    int readCount;

public:
    SensorManager() :
        oneWire(nullptr),
        tempSensors(nullptr),
        sensorsInitialized(false),
        #if ENABLE_DO_SENSOR
        doSensor(nullptr),
        doSensorErrors(0),
        #endif
        tempSensorErrors(0),
        phSensorErrors(0),
        tdsSensorErrors(0),
        phNeutralVoltage(PH_NEUTRAL_VOLTAGE),
        phAcidVoltage(PH_ACID_VOLTAGE),
        tdsCalibrationFactor(1.0),
        lastReadDuration(0),
        totalReadTime(0),
        readCount(0) {

        currentData.valid = false;
        #if ENABLE_DO_SENSOR
        currentData.dissolvedOxygen = 0.0;
        currentData.doSaturation = 0.0;
        #endif
        resetStats();
    }

    ~SensorManager() {
        if (tempSensors) delete tempSensors;
        if (oneWire) delete oneWire;
        #if ENABLE_DO_SENSOR
        if (doSensor) delete doSensor;
        #endif
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

            // OPTIMIERT: 9-bit Resolution (94ms statt 375ms bei 11-bit)
            // Genauigkeit: ±0.5°C (ausreichend für Forellenzucht!)
            tempSensors->setResolution(waterThermometer, DS18B20_RESOLUTION);
            tempSensors->setResolution(airThermometer, DS18B20_RESOLUTION);

            Serial.printf("✓ Temperatursensoren konfiguriert (%d-bit)\n", DS18B20_RESOLUTION);
            Serial.printf("  Conversion-Zeit: %d ms\n",
                         DS18B20_RESOLUTION == 9 ? 94 :
                         DS18B20_RESOLUTION == 10 ? 188 :
                         DS18B20_RESOLUTION == 11 ? 375 : 750);
            sensorsInitialized = true;
        } else {
            Serial.println("⚠ WARNUNG: Nicht genug Temperatursensoren gefunden!");
            sensorsInitialized = false;
        }

        // ADC Konfiguration
        analogReadResolution(12);  // 12-bit ADC (0-4095)
        analogSetAttenuation(ADC_11db);  // 0-3.3V

        Serial.println("✓ ADC konfiguriert (12-bit, 0-3.3V)");

        // Optional: DO-Sensor
        #if ENABLE_DO_SENSOR
        doSensor = new DOSensor();
        if (doSensor->begin()) {
            Serial.println("✓ DO-Sensor aktiviert");
        } else {
            Serial.println("ℹ DO-Sensor nicht verfügbar (optional)");
        }
        #endif

        Serial.println("✓ Sensor-Manager initialisiert\n");

        return sensorsInitialized;
    }

    // Alle Sensoren auslesen
    bool readAll() {
        if (!sensorsInitialized) {
            currentData.valid = false;
            return false;
        }

        unsigned long startTime = millis();

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

        // Optional: DO-Sensor (mit Temperaturkompensation!)
        #if ENABLE_DO_SENSOR
        if (doSensor && doSensor->isInitialized()) {
            if (doSensor->read(currentData.waterTemp)) {
                currentData.dissolvedOxygen = doSensor->getDO();
                currentData.doSaturation = doSensor->getSaturation();
            } else {
                doSensorErrors++;
            }
        }
        #endif

        // Timestamp
        currentData.timestamp = millis();
        currentData.valid = true;

        // Performance-Tracking
        lastReadDuration = millis() - startTime;
        totalReadTime += lastReadDuration;
        readCount++;

        #if DEBUG_PERFORMANCE
        if (readCount % 100 == 0) {
            Serial.printf("Sensor Read Performance: %lu ms (Avg: %lu ms)\n",
                         lastReadDuration,
                         totalReadTime / readCount);
        }
        #endif

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
        #if ENABLE_DO_SENSOR
        Serial.printf("DO-Fehler: %d\n", doSensorErrors);
        #endif
        Serial.println();
    }

    // Performance-Statistiken
    void printPerformanceStats() {
        Serial.println("\n=== Sensor Performance ===");
        Serial.printf("Letzter Read: %lu ms\n", lastReadDuration);
        if (readCount > 0) {
            Serial.printf("Durchschnitt: %lu ms\n", totalReadTime / readCount);
            Serial.printf("Reads Total: %d\n", readCount);
        }
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

        #if ENABLE_DO_SENSOR
        if (doSensor && doSensor->isInitialized()) {
            Serial.printf("DO: %.2f mg/L (%.1f%%)\n",
                         currentData.dissolvedOxygen,
                         currentData.doSaturation);
        }
        #endif

        Serial.println();
    }

    // DO-Sensor Zugriff (wenn aktiviert)
    #if ENABLE_DO_SENSOR
    DOSensor* getDOSensor() { return doSensor; }
    #endif
};

#endif // SENSORS_H
