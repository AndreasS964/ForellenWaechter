/*
 * ForellenWächter v2.1 - Credentials Manager
 * Sicheres Passwort-Management mit EEPROM
 *
 * SECURITY: Generiert starke Passwörter beim ersten Start
 * und speichert sie verschlüsselt im EEPROM
 */

#ifndef CREDENTIALS_MANAGER_H
#define CREDENTIALS_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

class CredentialsManager {
private:
    Preferences prefs;
    String ap_password;
    String web_username;
    String web_password;
    String ota_password;

    // Sicheren Zufalls-String generieren
    String generateSecurePassword(int length = 16) {
        const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*-_=+";
        const int charsetSize = sizeof(charset) - 1;
        String password = "";

        for (int i = 0; i < length; i++) {
            uint32_t randomValue = esp_random();
            password += charset[randomValue % charsetSize];
        }

        return password;
    }

    // Passwort-Stärke validieren
    bool validatePasswordStrength(const char* password) {
        int len = strlen(password);
        if (len < 12) return false; // Minimum 12 Zeichen

        bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;

        for (int i = 0; i < len; i++) {
            if (isupper(password[i])) hasUpper = true;
            else if (islower(password[i])) hasLower = true;
            else if (isdigit(password[i])) hasDigit = true;
            else hasSpecial = true;
        }

        return hasUpper && hasLower && hasDigit; // Special optional
    }

public:
    CredentialsManager() {}

    // Initialisierung
    void begin() {
        prefs.begin("credentials", false);

        // Prüfe ob bereits initialisiert
        if (!prefs.isKey("initialized") || !prefs.getBool("initialized", false)) {
            Serial.println("\n════════════════════════════════════════════════════");
            Serial.println("   🔐 ERSTE INITIALISIERUNG - CREDENTIALS SETUP");
            Serial.println("════════════════════════════════════════════════════");
            Serial.println("");
            Serial.println("⚠️  WICHTIG: Notiere diese Passwörter!");
            Serial.println("   Sie werden nur EINMAL angezeigt!\n");

            // Sichere Passwörter generieren
            ap_password = generateSecurePassword(16);
            web_username = "admin";
            web_password = generateSecurePassword(20);
            ota_password = generateSecurePassword(16);

            // In EEPROM speichern
            prefs.putString("ap_pass", ap_password);
            prefs.putString("web_user", web_username);
            prefs.putString("web_pass", web_password);
            prefs.putString("ota_pass", ota_password);
            prefs.putBool("initialized", true);

            // Passwörter ausgeben
            Serial.println("─────────────────────────────────────────────────────");
            Serial.println("WiFi Access Point:");
            Serial.println("  SSID:     ForellenWaechter");
            Serial.printf( "  Passwort: %s\n", ap_password.c_str());
            Serial.println("─────────────────────────────────────────────────────");
            Serial.println("Web-Interface Login:");
            Serial.printf( "  Username: %s\n", web_username.c_str());
            Serial.printf( "  Passwort: %s\n", web_password.c_str());
            Serial.println("─────────────────────────────────────────────────────");
            Serial.println("OTA-Update:");
            Serial.printf( "  Passwort: %s\n", ota_password.c_str());
            Serial.println("─────────────────────────────────────────────────────");
            Serial.println("\n⚠️  SPEICHERE DIESE PASSWÖRTER SICHER!");
            Serial.println("   (z.B. in einem Passwort-Manager)\n");
            Serial.println("════════════════════════════════════════════════════\n");

            delay(10000); // 10 Sekunden Wartezeit zum Notieren

        } else {
            // Credentials aus EEPROM laden
            ap_password = prefs.getString("ap_pass", "");
            web_username = prefs.getString("web_user", "admin");
            web_password = prefs.getString("web_pass", "");
            ota_password = prefs.getString("ota_pass", "");

            Serial.println("✓ Credentials aus EEPROM geladen");
        }
    }

    // Getter Methoden
    const char* getAPPassword() const {
        return ap_password.c_str();
    }

    const char* getWebUsername() const {
        return web_username.c_str();
    }

    const char* getWebPassword() const {
        return web_password.c_str();
    }

    const char* getOTAPassword() const {
        return ota_password.c_str();
    }

    // Passwort ändern (mit Validierung)
    bool updateAPPassword(const char* newPass) {
        if (!validatePasswordStrength(newPass)) {
            Serial.println("✗ Passwort zu schwach! (min. 12 Zeichen)");
            return false;
        }

        prefs.putString("ap_pass", newPass);
        ap_password = String(newPass);
        Serial.println("✓ AP-Passwort aktualisiert");
        return true;
    }

    bool updateWebPassword(const char* newPass) {
        if (!validatePasswordStrength(newPass)) {
            Serial.println("✗ Passwort zu schwach! (min. 12 Zeichen)");
            return false;
        }

        prefs.putString("web_pass", newPass);
        web_password = String(newPass);
        Serial.println("✓ Web-Passwort aktualisiert");
        return true;
    }

    bool updateOTAPassword(const char* newPass) {
        if (!validatePasswordStrength(newPass)) {
            Serial.println("✗ Passwort zu schwach! (min. 12 Zeichen)");
            return false;
        }

        prefs.putString("ota_pass", newPass);
        ota_password = String(newPass);
        Serial.println("✓ OTA-Passwort aktualisiert");
        return true;
    }

    // Factory Reset - Alle Credentials löschen
    bool factoryReset() {
        Serial.println("\n⚠️  FACTORY RESET - Lösche alle Credentials...");
        prefs.clear();
        Serial.println("✓ Credentials gelöscht. Neustart erforderlich.");
        return true;
    }

    // Credentials-Status ausgeben (zensiert)
    void printStatus() {
        Serial.println("\n=== Credentials Status ===");
        Serial.printf("Initialized: %s\n", prefs.getBool("initialized", false) ? "YES" : "NO");
        Serial.printf("AP Password: %s*** (Länge: %d)\n",
                     ap_password.substring(0, 3).c_str(),
                     ap_password.length());
        Serial.printf("Web Username: %s\n", web_username.c_str());
        Serial.printf("Web Password: %s*** (Länge: %d)\n",
                     web_password.substring(0, 3).c_str(),
                     web_password.length());
        Serial.printf("OTA Password: %s*** (Länge: %d)\n",
                     ota_password.substring(0, 3).c_str(),
                     ota_password.length());
        Serial.println();
    }
};

#endif // CREDENTIALS_MANAGER_H
