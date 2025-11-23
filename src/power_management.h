/*
 * ForellenWächter v2.0 - Energie-Management
 * Optimiert für Off-Grid / Batterie / Solar Betrieb
 */

#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include <esp_sleep.h>
#include "config.h"

class PowerManager {
private:
    unsigned long lastActivity;
    bool powerSaveMode;

public:
    PowerManager() : lastActivity(0), powerSaveMode(false) {}

    // Initialisierung
    void begin() {
        Serial.println("\n=== Energie-Management ===");

        // CPU Frequenz reduzieren für Energiesparen
        setCpuFrequencyMhz(CPU_FREQUENCY);
        Serial.printf("CPU Frequenz: %d MHz\n", getCpuFrequencyMhz());

        // Bluetooth komplett deaktivieren (nicht benötigt)
        btStop();
        esp_bt_controller_disable();
        Serial.println("✓ Bluetooth deaktiviert");

        // WiFi Power Save konfigurieren
        if (WIFI_POWER_SAVE) {
            esp_wifi_set_ps(WIFI_PS_MIN_MODEM);  // Modem Sleep aktivieren
            Serial.println("✓ WiFi Power Save aktiviert (Modem Sleep)");
        }

        // ADC Genauigkeit reduzieren für schnellere Messungen
        analogSetAttenuation(ADC_11db);  // 0-3.3V Range

        // Status LED dimmen
        #ifdef STATUS_LED
        ledcSetup(0, 5000, 8);  // Channel 0, 5kHz, 8-bit
        ledcAttachPin(STATUS_LED, 0);
        ledcWrite(0, DIM_LED_BRIGHTNESS);
        Serial.printf("✓ LED Helligkeit reduziert: %d/255\n", DIM_LED_BRIGHTNESS);
        #endif

        // Watchdog Timer konfigurieren
        esp_task_wdt_init(WATCHDOG_TIMEOUT, true);
        esp_task_wdt_add(NULL);
        Serial.printf("✓ Watchdog Timer: %ds\n", WATCHDOG_TIMEOUT);

        Serial.println("✓ Energie-Management initialisiert\n");
    }

    // Watchdog zurücksetzen
    void feedWatchdog() {
        esp_task_wdt_reset();
    }

    // WiFi Power Save aktivieren/deaktivieren
    void setWiFiPowerSave(bool enable) {
        if (enable) {
            WiFi.setSleep(true);
            esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
        } else {
            WiFi.setSleep(false);
            esp_wifi_set_ps(WIFI_PS_NONE);
        }
    }

    // Light Sleep (WiFi bleibt verbunden)
    void lightSleep(uint32_t milliseconds) {
        esp_sleep_enable_timer_wakeup(milliseconds * 1000);
        esp_light_sleep_start();
    }

    // Deep Sleep (WiFi trennt sich, voller Reset bei Wakeup)
    void deepSleep(uint32_t seconds) {
        Serial.printf("Gehe in Deep Sleep für %d Sekunden...\n", seconds);
        esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);
        esp_deep_sleep_start();
    }

    // Batteriespannung messen (wenn Spannungsteiler an ADC angeschlossen)
    float getBatteryVoltage() {
        // Beispiel: Spannungsteiler 100k/47k an GPIO36
        // Anpassen je nach Hardware
        #ifdef BATTERY_PIN
        int raw = analogRead(BATTERY_PIN);
        float voltage = (raw / 4095.0) * 3.3 * 3.13; // Faktor anpassen
        return voltage;
        #else
        return 0.0;
        #endif
    }

    // Batterie-Status prüfen
    bool isBatteryLow() {
        #ifdef BATTERY_PIN
        float voltage = getBatteryVoltage();
        return voltage < 11.0;  // Unter 11V = Batterie schwach
        #else
        return false;
        #endif
    }

    // Power Save Modus aktivieren bei niedriger Batterie
    void enablePowerSaveMode() {
        if (powerSaveMode) return;

        Serial.println("⚠ Aktiviere Energiespar-Modus!");
        powerSaveMode = true;

        // CPU Frequenz weiter reduzieren
        setCpuFrequencyMhz(80);

        // WiFi Power Save maximieren
        esp_wifi_set_ps(WIFI_PS_MAX_MODEM);

        // LED komplett aus
        #ifdef STATUS_LED
        ledcWrite(0, 0);
        #endif
    }

    // Normal Modus wiederherstellen
    void disablePowerSaveMode() {
        if (!powerSaveMode) return;

        Serial.println("✓ Deaktiviere Energiespar-Modus");
        powerSaveMode = false;

        setCpuFrequencyMhz(CPU_FREQUENCY);
        esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

        #ifdef STATUS_LED
        ledcWrite(0, DIM_LED_BRIGHTNESS);
        #endif
    }

    // Ist Power Save Modus aktiv?
    bool isPowerSaveMode() {
        return powerSaveMode;
    }

    // Energie-Statistiken ausgeben
    void printPowerStats() {
        Serial.println("\n=== Energie-Statistiken ===");
        Serial.printf("CPU Frequenz: %d MHz\n", getCpuFrequencyMhz());
        Serial.printf("WiFi Power Save: %s\n", WiFi.getSleep() ? "AN" : "AUS");
        Serial.printf("Power Save Modus: %s\n", powerSaveMode ? "AKTIV" : "NORMAL");

        #ifdef BATTERY_PIN
        Serial.printf("Batteriespannung: %.2fV\n", getBatteryVoltage());
        #endif

        Serial.println();
    }

    // Optimierte Delays (mit Watchdog Feed)
    void efficientDelay(unsigned long ms) {
        unsigned long start = millis();
        while (millis() - start < ms) {
            feedWatchdog();
            delay(10);
        }
    }
};

#endif // POWER_MANAGEMENT_H
