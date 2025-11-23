/*
 * ForellenWächter v2.2 - INA219 Power Monitor
 * Stromverbrauch & Leistungs-Tracking für Off-Grid Systeme
 *
 * FEATURES:
 * - Stromverbrauch messen (mA)
 * - Spannung messen (V)
 * - Leistung berechnen (W)
 * - Energie-Tracking (Wh)
 * - Effizienz-Analyse
 * - Wasserrad/Solar-Leistung überwachen
 *
 * Hardware: INA219 I2C Modul (~3€ bei AliExpress)
 */

#ifndef INA219_MONITOR_H
#define INA219_MONITOR_H

#include <Arduino.h>
#include <Wire.h>

// INA219 Konfiguration
#define ENABLE_INA219 false        // Aktivieren wenn Hardware vorhanden
#define INA219_I2C_ADDRESS 0x40    // Standard I2C Adresse
#define INA219_I2C_SDA 21          // ESP32 Standard SDA
#define INA219_I2C_SCL 22          // ESP32 Standard SCL

// INA219 Register
#define INA219_REG_CONFIG 0x00
#define INA219_REG_SHUNTVOLTAGE 0x01
#define INA219_REG_BUSVOLTAGE 0x02
#define INA219_REG_POWER 0x03
#define INA219_REG_CURRENT 0x04
#define INA219_REG_CALIBRATION 0x05

// Kalibrierung (abhängig vom Shunt-Widerstand)
#define SHUNT_RESISTANCE 0.1       // 0.1 Ohm (Standard)
#define MAX_EXPECTED_CURRENT 3.2   // 3.2A max
#define CURRENT_LSB 0.0001         // 0.1mA pro Bit

class INA219Monitor {
private:
    bool initialized;
    float shuntVoltage;      // mV
    float busVoltage;        // V
    float current;           // mA
    float power;             // mW

    // Langzeit-Tracking
    float totalEnergy;       // Wh (akkumuliert)
    unsigned long lastUpdate;
    float peakCurrent;
    float minVoltage;
    float maxVoltage;

    // Effizienz-Daten
    float avgCurrent;
    int sampleCount;

    // I2C Helper
    void writeRegister(uint8_t reg, uint16_t value) {
        Wire.beginTransmission(INA219_I2C_ADDRESS);
        Wire.write(reg);
        Wire.write((value >> 8) & 0xFF);
        Wire.write(value & 0xFF);
        Wire.endTransmission();
    }

    uint16_t readRegister(uint8_t reg) {
        Wire.beginTransmission(INA219_I2C_ADDRESS);
        Wire.write(reg);
        Wire.endTransmission();

        Wire.requestFrom((uint8_t)INA219_I2C_ADDRESS, (uint8_t)2);

        if (Wire.available() == 2) {
            uint16_t value = Wire.read() << 8;
            value |= Wire.read();
            return value;
        }

        return 0;
    }

    // Kalibrierung berechnen
    void calibrate() {
        // Formel aus INA219 Datasheet
        uint16_t cal = (uint16_t)(0.04096 / (CURRENT_LSB * SHUNT_RESISTANCE));
        writeRegister(INA219_REG_CALIBRATION, cal);

        // Config: 32V Bus, ±320mV Shunt, 12-bit, Continuous
        uint16_t config = 0x399F;
        writeRegister(INA219_REG_CONFIG, config);
    }

public:
    INA219Monitor() :
        initialized(false),
        shuntVoltage(0),
        busVoltage(0),
        current(0),
        power(0),
        totalEnergy(0),
        lastUpdate(0),
        peakCurrent(0),
        minVoltage(99),
        maxVoltage(0),
        avgCurrent(0),
        sampleCount(0)
    {}

    // Initialisierung
    bool begin() {
        #if !ENABLE_INA219
        return false;
        #endif

        Serial.println("\n=== INA219 Power Monitor ===");

        // I2C initialisieren
        Wire.begin(INA219_I2C_SDA, INA219_I2C_SCL);

        // Teste Verbindung
        Wire.beginTransmission(INA219_I2C_ADDRESS);
        if (Wire.endTransmission() != 0) {
            Serial.println("✗ INA219 nicht gefunden auf 0x40");
            Serial.println("  (Feature ist optional - System läuft weiter)");
            initialized = false;
            return false;
        }

        Serial.println("✓ INA219 erkannt");

        // Kalibrieren
        calibrate();

        Serial.printf("  Shunt: %.2f Ω\n", SHUNT_RESISTANCE);
        Serial.printf("  Max Current: %.1f A\n", MAX_EXPECTED_CURRENT);
        Serial.println("✓ INA219 kalibriert\n");

        initialized = true;
        lastUpdate = millis();
        return true;
    }

    // Messung durchführen
    bool read() {
        if (!initialized) return false;

        unsigned long now = millis();

        // Shunt Voltage lesen (±320mV)
        int16_t shuntRaw = (int16_t)readRegister(INA219_REG_SHUNTVOLTAGE);
        shuntVoltage = shuntRaw * 0.01; // 10µV pro Bit

        // Bus Voltage lesen (0-32V)
        uint16_t busRaw = readRegister(INA219_REG_BUSVOLTAGE);
        busVoltage = ((busRaw >> 3) * 4) / 1000.0; // 4mV pro Bit, konvertiere zu V

        // Current lesen
        int16_t currentRaw = (int16_t)readRegister(INA219_REG_CURRENT);
        current = currentRaw * CURRENT_LSB * 1000.0; // in mA

        // Power lesen
        uint16_t powerRaw = readRegister(INA219_REG_POWER);
        power = powerRaw * 20.0 * CURRENT_LSB; // in mW

        // Statistiken updaten
        if (current > peakCurrent) peakCurrent = current;
        if (busVoltage < minVoltage && busVoltage > 0) minVoltage = busVoltage;
        if (busVoltage > maxVoltage) maxVoltage = busVoltage;

        // Rolling Average für Strom
        avgCurrent = (avgCurrent * sampleCount + current) / (sampleCount + 1);
        sampleCount++;

        // Energie akkumulieren (Wh)
        if (lastUpdate > 0) {
            float hours = (now - lastUpdate) / 3600000.0;
            totalEnergy += (power / 1000.0) * hours; // mW → W, dann × h
        }

        lastUpdate = now;
        return true;
    }

    // Getter
    float getCurrent() const { return current; }           // mA
    float getVoltage() const { return busVoltage; }       // V
    float getPower() const { return power; }              // mW
    float getEnergy() const { return totalEnergy; }       // Wh
    float getPeakCurrent() const { return peakCurrent; }  // mA
    float getAvgCurrent() const { return avgCurrent; }    // mA

    // Effizienz-Berechnung
    float getEfficiency() const {
        // Vergleich aktueller vs durchschnittlicher Verbrauch
        if (avgCurrent == 0) return 100.0;
        return (avgCurrent / (avgCurrent + 10)) * 100.0; // Vereinfacht
    }

    // Batterie-Laufzeit schätzen (bei gegebener Kapazität)
    float estimateBatteryRuntime(float batteryAh) const {
        if (current == 0) return 0;
        return (batteryAh * 1000.0) / current; // Stunden
    }

    // Solar-Leistung berechnen (wenn INA219 am Solar-Panel)
    bool isSolarCharging() const {
        return current < 0; // Negativer Strom = Laden
    }

    // Statistiken zurücksetzen
    void resetStats() {
        totalEnergy = 0;
        peakCurrent = 0;
        minVoltage = 99;
        maxVoltage = 0;
        avgCurrent = 0;
        sampleCount = 0;
        Serial.println("✓ Power-Statistiken zurückgesetzt");
    }

    // Status ausgeben
    void printStatus() {
        if (!initialized) {
            Serial.println("INA219: NICHT AKTIVIERT");
            return;
        }

        Serial.println("\n=== INA219 Power Monitor ===");
        Serial.printf("Spannung: %.3f V\n", busVoltage);
        Serial.printf("Strom: %.1f mA\n", current);
        Serial.printf("Leistung: %.1f mW\n", power);
        Serial.printf("Energie (Total): %.2f Wh\n", totalEnergy);
        Serial.println("─────────────────────────");
        Serial.printf("Peak Current: %.1f mA\n", peakCurrent);
        Serial.printf("Avg Current: %.1f mA\n", avgCurrent);
        Serial.printf("Voltage Range: %.2f - %.2f V\n", minVoltage, maxVoltage);
        Serial.printf("Effizienz: %.1f%%\n", getEfficiency());
        Serial.println();
    }

    // JSON für API
    String getJSON() const {
        if (!initialized) return "{}";

        char json[256];
        snprintf(json, sizeof(json),
                "{\"voltage\":%.3f,\"current\":%.1f,\"power\":%.1f,"
                "\"energy\":%.2f,\"peak\":%.1f,\"avg\":%.1f}",
                busVoltage, current, power, totalEnergy, peakCurrent, avgCurrent);

        return String(json);
    }

    bool isInitialized() const { return initialized; }
};

#endif // INA219_MONITOR_H
