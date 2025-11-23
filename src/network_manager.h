/*
 * ForellenWächter v2.1 - Network Manager
 * Abstraktions-Layer für WiFi UND LTE-Module
 *
 * Unterstützt:
 * - WiFi (ESP32 onboard)
 * - SIM7600 (4G LTE)
 * - SIM800L (2G/3G)
 */

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

// LTE-Modul Config (Optional)
#define ENABLE_LTE false          // LTE-Modul aktivieren
#define LTE_MODULE_TYPE "SIM7600" // "SIM7600" oder "SIM800L"

// SIM7600 Pins (Standard)
#define LTE_SERIAL_RX 16
#define LTE_SERIAL_TX 17
#define LTE_POWER_PIN 4
#define LTE_BAUD 115200

// APN Einstellungen
#define APN_NAME "internet"           // Provider-abhängig
#define APN_USER ""                   // Meist leer
#define APN_PASS ""                   // Meist leer

enum NetworkType {
    NETWORK_NONE,
    NETWORK_WIFI,
    NETWORK_LTE
};

enum ConnectionStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR_STATE
};

class NetworkManager {
private:
    NetworkType currentNetwork;
    ConnectionStatus status;

    #if ENABLE_LTE
    HardwareSerial* lteSerial;
    bool lteModulePresent;
    String lteIMEI;
    int lteSignalQuality;
    #endif

    // WiFi Helpers
    bool connectWiFi(const char* ssid, const char* password, int timeout = 20) {
        Serial.printf("Verbinde mit WiFi: %s\n", ssid);

        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < timeout) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("✓ WiFi verbunden: %s\n", WiFi.localIP().toString().c_str());
            Serial.printf("  Signal: %d dBm\n", WiFi.RSSI());
            return true;
        }

        Serial.println("✗ WiFi Verbindung fehlgeschlagen");
        return false;
    }

    #if ENABLE_LTE
    // LTE Helpers
    bool initLTEModule() {
        Serial.println("\n=== LTE-Modul Initialisierung ===");

        lteSerial = new HardwareSerial(2);
        lteSerial->begin(LTE_BAUD, SERIAL_8N1, LTE_SERIAL_RX, LTE_SERIAL_TX);

        // Power-On Sequenz
        pinMode(LTE_POWER_PIN, OUTPUT);
        digitalWrite(LTE_POWER_PIN, LOW);
        delay(500);
        digitalWrite(LTE_POWER_PIN, HIGH);
        delay(2000);
        digitalWrite(LTE_POWER_PIN, LOW);
        delay(5000);

        // Modul-Test
        if (!sendATCommand("AT", "OK", 5000)) {
            Serial.println("✗ LTE-Modul antwortet nicht");
            lteModulePresent = false;
            return false;
        }

        Serial.println("✓ LTE-Modul erkannt");

        // Modul-Info
        lteIMEI = sendATCommandResponse("AT+GSN", 2000);
        Serial.printf("  IMEI: %s\n", lteIMEI.c_str());

        // Firmware-Version
        String firmware = sendATCommandResponse("AT+CGMR", 2000);
        Serial.printf("  Firmware: %s\n", firmware.c_str());

        // SIM-Karte prüfen
        if (!sendATCommand("AT+CPIN?", "READY", 5000)) {
            Serial.println("⚠ SIM-Karte nicht bereit");
            return false;
        }
        Serial.println("✓ SIM-Karte OK");

        lteModulePresent = true;
        return true;
    }

    bool connectLTE() {
        if (!lteModulePresent) {
            return false;
        }

        Serial.println("\n=== LTE-Verbindung herstellen ===");

        // APN konfigurieren
        String apnCmd = "AT+CGDCONT=1,\"IP\",\"" + String(APN_NAME) + "\"";
        if (!sendATCommand(apnCmd.c_str(), "OK", 5000)) {
            Serial.println("✗ APN-Konfiguration fehlgeschlagen");
            return false;
        }

        // Netzwerk-Registrierung warten
        Serial.print("Warte auf Netzwerk-Registrierung");
        int attempts = 0;
        while (attempts < 30) {
            String response = sendATCommandResponse("AT+CREG?", 2000);
            if (response.indexOf("+CREG: 0,1") >= 0 || response.indexOf("+CREG: 0,5") >= 0) {
                Serial.println(" OK");
                break;
            }
            Serial.print(".");
            delay(1000);
            attempts++;
        }

        if (attempts >= 30) {
            Serial.println(" Timeout");
            return false;
        }

        // Signal-Qualität prüfen
        String csqResponse = sendATCommandResponse("AT+CSQ", 2000);
        int csqStart = csqResponse.indexOf("+CSQ: ") + 6;
        if (csqStart > 6) {
            lteSignalQuality = csqResponse.substring(csqStart, csqStart + 2).toInt();
            Serial.printf("✓ Signal-Qualität: %d/31\n", lteSignalQuality);
        }

        // Daten-Verbindung aktivieren
        if (!sendATCommand("AT+CGACT=1,1", "OK", 10000)) {
            Serial.println("✗ Daten-Verbindung fehlgeschlagen");
            return false;
        }

        // IP-Adresse abrufen
        String ipResponse = sendATCommandResponse("AT+CGPADDR=1", 2000);
        Serial.printf("✓ LTE verbunden: %s\n", ipResponse.c_str());

        return true;
    }

    bool sendATCommand(const char* command, const char* expectedResponse, unsigned long timeout) {
        lteSerial->println(command);

        unsigned long start = millis();
        String response = "";

        while (millis() - start < timeout) {
            if (lteSerial->available()) {
                char c = lteSerial->read();
                response += c;

                if (response.indexOf(expectedResponse) >= 0) {
                    return true;
                }
            }
            delay(10);
        }

        return false;
    }

    String sendATCommandResponse(const char* command, unsigned long timeout) {
        lteSerial->println(command);

        unsigned long start = millis();
        String response = "";

        while (millis() - start < timeout) {
            if (lteSerial->available()) {
                response += (char)lteSerial->read();
            }
            delay(10);
        }

        return response;
    }
    #endif

public:
    NetworkManager() :
        currentNetwork(NETWORK_NONE),
        status(DISCONNECTED)
        #if ENABLE_LTE
        , lteSerial(nullptr),
        lteModulePresent(false),
        lteSignalQuality(0)
        #endif
    {}

    // Verbindung herstellen (Auto-Select)
    bool connect(const char* wifiSSID = "", const char* wifiPass = "") {
        status = CONNECTING;

        // Versuche WiFi zuerst
        if (strlen(wifiSSID) > 0) {
            if (connectWiFi(wifiSSID, wifiPass)) {
                currentNetwork = NETWORK_WIFI;
                status = CONNECTED;
                return true;
            }
        }

        #if ENABLE_LTE
        // Fallback auf LTE
        if (initLTEModule()) {
            if (connectLTE()) {
                currentNetwork = NETWORK_LTE;
                status = CONNECTED;
                return true;
            }
        }
        #endif

        status = ERROR_STATE;
        return false;
    }

    // Manuelle Netzwerk-Auswahl
    bool connectWiFiOnly(const char* ssid, const char* password) {
        status = CONNECTING;
        if (connectWiFi(ssid, password)) {
            currentNetwork = NETWORK_WIFI;
            status = CONNECTED;
            return true;
        }
        status = ERROR_STATE;
        return false;
    }

    #if ENABLE_LTE
    bool connectLTEOnly() {
        status = CONNECTING;
        if (initLTEModule() && connectLTE()) {
            currentNetwork = NETWORK_LTE;
            status = CONNECTED;
            return true;
        }
        status = ERROR_STATE;
        return false;
    }
    #endif

    // Verbindung trennen
    void disconnect() {
        if (currentNetwork == NETWORK_WIFI) {
            WiFi.disconnect();
        }
        #if ENABLE_LTE
        else if (currentNetwork == NETWORK_LTE && lteModulePresent) {
            sendATCommand("AT+CGACT=0,1", "OK", 5000);
        }
        #endif

        currentNetwork = NETWORK_NONE;
        status = DISCONNECTED;
    }

    // Status-Abfragen
    bool isConnected() const {
        return status == CONNECTED;
    }

    NetworkType getNetworkType() const {
        return currentNetwork;
    }

    String getNetworkTypeName() const {
        switch (currentNetwork) {
            case NETWORK_WIFI: return "WiFi";
            case NETWORK_LTE: return "LTE";
            default: return "None";
        }
    }

    int getSignalStrength() {
        if (currentNetwork == NETWORK_WIFI) {
            return WiFi.RSSI();
        }
        #if ENABLE_LTE
        else if (currentNetwork == NETWORK_LTE) {
            return lteSignalQuality;
        }
        #endif
        return 0;
    }

    String getIPAddress() {
        if (currentNetwork == NETWORK_WIFI) {
            return WiFi.localIP().toString();
        }
        #if ENABLE_LTE
        else if (currentNetwork == NETWORK_LTE && lteModulePresent) {
            return sendATCommandResponse("AT+CGPADDR=1", 2000);
        }
        #endif
        return "0.0.0.0";
    }

    // Statistiken ausgeben
    void printStatus() {
        Serial.println("\n=== Network Status ===");
        Serial.printf("Type: %s\n", getNetworkTypeName().c_str());
        Serial.printf("Status: %s\n", status == CONNECTED ? "Connected" : "Disconnected");

        if (status == CONNECTED) {
            Serial.printf("IP: %s\n", getIPAddress().c_str());
            Serial.printf("Signal: %d\n", getSignalStrength());
        }

        Serial.println();
    }
};

#endif // NETWORK_MANAGER_H
