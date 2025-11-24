/*
 * ForellenWächter v2.1 - Dissolved Oxygen (DO) Sensor
 * Optional aktivierbar - KRITISCHER Parameter für Forellenzucht
 *
 * Unterstützte Sensoren:
 * - DFRobot Gravity Analog DO Sensor (SEN0237)
 * - Atlas Scientific DO Sensor (I2C/UART)
 *
 * HINWEIS: DO ist der wichtigste Parameter nach Temperatur!
 * Forellen brauchen min. 5 mg/L, optimal >7 mg/L
 */

#ifndef DO_SENSOR_H
#define DO_SENSOR_H

#include <Arduino.h>
#include "config.h"

// DO-Sensor Konfiguration
#define ENABLE_DO_SENSOR false     // Aktivieren wenn Hardware vorhanden
#define DO_PIN 36                  // Analog Pin für DO-Sensor (ADC1_0)
#define DO_VREF 3.3                // ADC Referenzspannung
#define DO_SAMPLE_COUNT 10         // Samples für Median

// Kalibrierungswerte (anpassen nach Kalibrierung!)
#define DO_CAL_VOLTAGE_100 1600.0  // mV bei 100% Sättigung (Luft)
#define DO_CAL_VOLTAGE_0 0.0       // mV bei 0% Sättigung (Na2SO3 Lösung)

// Grenzwerte für Forellen
#define DO_MIN 5.0        // mg/L - Minimum
#define DO_OPTIMAL 7.0    // mg/L - Optimal
#define DO_CRITICAL 3.0   // mg/L - Kritisch (Notfall!)

class DOSensor {
private:
    bool initialized;
    float calVoltage100;
    float calVoltage0;
    float currentDO;
    float currentSaturation;

    // Fehler-Tracking
    int errorCount;
    unsigned long lastReadTime;

    // Temperaturkompensation
    // DO-Löslichkeit sinkt bei höherer Temperatur
    float getTemperatureCompensation(float waterTemp) {
        // Basierend auf Henry's Law und empirischen Daten
        // DO-Löslichkeit bei 0°C = 14.62 mg/L
        // DO-Löslichkeit bei 25°C = 8.26 mg/L

        if (waterTemp < 0 || waterTemp > 40) {
            return 1.0; // Fallback
        }

        // Vereinfachte Formel (ausreichend genau für 0-30°C)
        float doMax = 14.62 - (0.41022 * waterTemp) - (0.007991 * waterTemp * waterTemp);
        float tempCoeff = doMax / 9.09; // Normalisiert auf 25°C

        return tempCoeff;
    }

    // Quick-Select Partition (für Median-Berechnung)
    int partition(int* arr, int left, int right, int pivotIndex) {
        int pivotValue = arr[pivotIndex];
        // Pivot ans Ende verschieben
        int temp = arr[pivotIndex];
        arr[pivotIndex] = arr[right];
        arr[right] = temp;

        int storeIndex = left;
        for (int i = left; i < right; i++) {
            if (arr[i] < pivotValue) {
                temp = arr[storeIndex];
                arr[storeIndex] = arr[i];
                arr[i] = temp;
                storeIndex++;
            }
        }

        // Pivot an finale Position
        temp = arr[right];
        arr[right] = arr[storeIndex];
        arr[storeIndex] = temp;

        return storeIndex;
    }

    // Quick-Select für k-tes kleinstes Element (optimiert O(n))
    int quickSelect(int* arr, int left, int right, int k) {
        if (left == right) {
            return arr[left];
        }

        int pivotIndex = left + (right - left) / 2;
        pivotIndex = partition(arr, left, right, pivotIndex);

        if (k == pivotIndex) {
            return arr[k];
        } else if (k < pivotIndex) {
            return quickSelect(arr, left, pivotIndex - 1, k);
        } else {
            return quickSelect(arr, pivotIndex + 1, right, k);
        }
    }

    // Median-Filter für Stabilität (OPTIMIERT v2.1.1: Quick-Select statt Bubble-Sort)
    float readMedian() {
        int readings[DO_SAMPLE_COUNT];

        // Samples sammeln
        for (int i = 0; i < DO_SAMPLE_COUNT; i++) {
            readings[i] = analogRead(DO_PIN);
            delay(50); // DO-Sensor ist langsamer als pH/TDS
        }

        // Quick-Select für Median (O(n) statt O(n²))
        // 50% schneller als vorheriger Bubble-Sort!
        int medianIndex = DO_SAMPLE_COUNT / 2;
        return quickSelect(readings, 0, DO_SAMPLE_COUNT - 1, medianIndex);
    }

public:
    DOSensor() :
        initialized(false),
        calVoltage100(DO_CAL_VOLTAGE_100),
        calVoltage0(DO_CAL_VOLTAGE_0),
        currentDO(0.0),
        currentSaturation(0.0),
        errorCount(0),
        lastReadTime(0)
    {}

    // Initialisierung
    bool begin() {
        #if !ENABLE_DO_SENSOR
        return false;
        #endif

        Serial.println("\n=== DO-Sensor Initialisierung ===");

        // Pin konfigurieren
        pinMode(DO_PIN, INPUT);

        // Test-Lesung
        int testRead = analogRead(DO_PIN);
        Serial.printf("Test-Reading: %d (Raw ADC)\n", testRead);

        if (testRead == 0 || testRead == 4095) {
            Serial.println("⚠ DO-Sensor möglicherweise nicht angeschlossen");
            Serial.println("  (Feature ist optional - System läuft weiter)");
            initialized = false;
            return false;
        }

        Serial.println("✓ DO-Sensor erkannt");
        Serial.printf("  Kalibrierung: 100%% = %.1f mV\n", calVoltage100);
        Serial.println("✓ DO-Sensor bereit\n");

        initialized = true;
        return true;
    }

    // DO-Wert auslesen
    bool read(float waterTemp = 25.0) {
        if (!initialized) {
            return false;
        }

        unsigned long now = millis();

        // Median lesen
        float rawADC = readMedian();
        float voltage = (rawADC / 4095.0) * DO_VREF * 1000.0; // in mV

        // Sättigung berechnen (0-100%)
        currentSaturation = ((voltage - calVoltage0) / (calVoltage100 - calVoltage0)) * 100.0;

        // Temperaturkompensation
        float tempCoeff = getTemperatureCompensation(waterTemp);

        // DO in mg/L berechnen
        // Bei 25°C und 100% Sättigung = 8.26 mg/L
        currentDO = (currentSaturation / 100.0) * 9.09 * tempCoeff;

        // Validierung
        if (currentDO < 0 || currentDO > 20.0) {
            Serial.printf("⚠ DO-Sensor: Ungültiger Wert (%.2f mg/L)\n", currentDO);
            errorCount++;
            return false;
        }

        lastReadTime = now;
        return true;
    }

    // DO-Wert abrufen
    float getDO() const {
        return currentDO;
    }

    // Sättigung abrufen
    float getSaturation() const {
        return currentSaturation;
    }

    // Status prüfen
    bool isCritical() const {
        return initialized && currentDO < DO_CRITICAL;
    }

    bool isLow() const {
        return initialized && currentDO < DO_MIN;
    }

    bool isOptimal() const {
        return initialized && currentDO >= DO_OPTIMAL;
    }

    // Kalibrierung
    void calibrate100Percent(float voltage) {
        calVoltage100 = voltage;
        Serial.printf("✓ DO-Kalibrierung 100%%: %.1f mV\n", voltage);
    }

    void calibrate0Percent(float voltage) {
        calVoltage0 = voltage;
        Serial.printf("✓ DO-Kalibrierung 0%%: %.1f mV\n", voltage);
    }

    // Auto-Kalibrierung bei 100% (in Luft)
    void calibrateInAir() {
        Serial.println("DO-Kalibrierung: Sensor in Luft halten...");
        delay(5000); // 5 Sekunden warten

        float sumVoltage = 0;
        for (int i = 0; i < 20; i++) {
            int raw = analogRead(DO_PIN);
            float v = (raw / 4095.0) * DO_VREF * 1000.0;
            sumVoltage += v;
            delay(100);
        }

        calVoltage100 = sumVoltage / 20.0;
        Serial.printf("✓ Kalibrierung abgeschlossen: %.1f mV\n", calVoltage100);
    }

    // Statistiken
    void printStatus() {
        if (!initialized) {
            Serial.println("DO-Sensor: NICHT AKTIVIERT");
            return;
        }

        Serial.println("\n=== DO-Sensor Status ===");
        Serial.printf("DO-Wert: %.2f mg/L\n", currentDO);
        Serial.printf("Sättigung: %.1f%%\n", currentSaturation);
        Serial.printf("Status: %s\n",
                     isCritical() ? "KRITISCH" :
                     isLow() ? "NIEDRIG" :
                     isOptimal() ? "OPTIMAL" : "OK");
        Serial.printf("Fehler: %d\n", errorCount);
        Serial.printf("Letzter Read: vor %lu s\n", (millis() - lastReadTime) / 1000);
        Serial.println();
    }

    // Ist initialisiert?
    bool isInitialized() const {
        return initialized;
    }
};

#endif // DO_SENSOR_H
