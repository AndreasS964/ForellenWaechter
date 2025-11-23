/*
 * ForellenWächter v2.1 - Rate Limiter
 * Schutz gegen DoS-Angriffe und Request-Flooding
 */

#ifndef RATE_LIMITER_H
#define RATE_LIMITER_H

#include <Arduino.h>
#include <WiFi.h>

class RateLimiter {
private:
    struct ClientInfo {
        IPAddress ip;
        unsigned long windowStart;
        int requestCount;
        bool blocked;
        unsigned long blockUntil;
    };

    static const int MAX_CLIENTS = 10;
    static const int MAX_REQUESTS_PER_MINUTE = 60;
    static const unsigned long WINDOW_MS = 60000; // 1 Minute
    static const unsigned long BLOCK_DURATION_MS = 300000; // 5 Minuten Sperre
    static const int BLOCK_THRESHOLD = 100; // Bei 100 Requests/Min blockieren

    ClientInfo clients[MAX_CLIENTS];
    int blockedCount = 0;

    // Finde Client-Slot
    int findClientSlot(IPAddress ip) {
        unsigned long now = millis();

        // Suche existierenden Client
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].ip == ip && clients[i].requestCount > 0) {
                return i;
            }
        }

        // Suche freien Slot
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].requestCount == 0) {
                clients[i].ip = ip;
                clients[i].windowStart = now;
                clients[i].requestCount = 0;
                clients[i].blocked = false;
                return i;
            }
        }

        // Ältesten Slot überschreiben
        int oldest = 0;
        unsigned long oldestTime = clients[0].windowStart;

        for (int i = 1; i < MAX_CLIENTS; i++) {
            if (clients[i].windowStart < oldestTime) {
                oldest = i;
                oldestTime = clients[i].windowStart;
            }
        }

        clients[oldest].ip = ip;
        clients[oldest].windowStart = now;
        clients[oldest].requestCount = 0;
        clients[oldest].blocked = false;

        return oldest;
    }

public:
    RateLimiter() {
        reset();
    }

    // Alle Einträge zurücksetzen
    void reset() {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            clients[i].requestCount = 0;
            clients[i].blocked = false;
        }
        blockedCount = 0;
    }

    // Prüfe ob Request erlaubt ist
    bool allowRequest(IPAddress clientIP) {
        unsigned long now = millis();
        int slot = findClientSlot(clientIP);

        if (slot < 0) {
            // Fallback: Erlaube wenn kein Slot verfügbar
            return true;
        }

        ClientInfo& client = clients[slot];

        // Prüfe ob Client blockiert ist
        if (client.blocked) {
            if (now < client.blockUntil) {
                // Noch blockiert
                return false;
            } else {
                // Block abgelaufen
                client.blocked = false;
                client.requestCount = 0;
                client.windowStart = now;
                blockedCount--;
            }
        }

        // Prüfe ob neues Zeitfenster
        if (now - client.windowStart > WINDOW_MS) {
            client.windowStart = now;
            client.requestCount = 0;
        }

        // Erhöhe Counter
        client.requestCount++;

        // Prüfe Limit
        if (client.requestCount > BLOCK_THRESHOLD) {
            // Client blockieren
            client.blocked = true;
            client.blockUntil = now + BLOCK_DURATION_MS;
            blockedCount++;

            Serial.printf("⚠ RATE LIMIT: Client %s blockiert für 5 Min (%d Requests)\n",
                         clientIP.toString().c_str(),
                         client.requestCount);

            return false;
        }

        if (client.requestCount > MAX_REQUESTS_PER_MINUTE) {
            // Soft Limit - noch nicht blockiert
            return false;
        }

        return true;
    }

    // Prüfe ob Client blockiert ist
    bool isBlocked(IPAddress clientIP) {
        unsigned long now = millis();

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].ip == clientIP && clients[i].blocked) {
                if (now < clients[i].blockUntil) {
                    return true;
                }
            }
        }

        return false;
    }

    // Client manuell blockieren
    void blockClient(IPAddress clientIP, unsigned long durationMs = BLOCK_DURATION_MS) {
        int slot = findClientSlot(clientIP);
        if (slot >= 0) {
            clients[slot].blocked = true;
            clients[slot].blockUntil = millis() + durationMs;
            blockedCount++;

            Serial.printf("⚠ Client %s manuell blockiert\n", clientIP.toString().c_str());
        }
    }

    // Client manuell entsperren
    void unblockClient(IPAddress clientIP) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].ip == clientIP && clients[i].blocked) {
                clients[i].blocked = false;
                clients[i].requestCount = 0;
                blockedCount--;

                Serial.printf("✓ Client %s entsperrt\n", clientIP.toString().c_str());
                return;
            }
        }
    }

    // Statistiken ausgeben
    void printStats() {
        Serial.println("\n=== Rate Limiter Statistiken ===");
        Serial.printf("Aktive Clients: %d/%d\n", getActiveClientCount(), MAX_CLIENTS);
        Serial.printf("Blockierte Clients: %d\n", blockedCount);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].requestCount > 0) {
                Serial.printf("  [%d] %s: %d Requests",
                             i,
                             clients[i].ip.toString().c_str(),
                             clients[i].requestCount);

                if (clients[i].blocked) {
                    unsigned long remaining = (clients[i].blockUntil - millis()) / 1000;
                    Serial.printf(" (BLOCKIERT - %lus verbleibend)", remaining);
                }

                Serial.println();
            }
        }
        Serial.println();
    }

    // Anzahl aktiver Clients
    int getActiveClientCount() {
        int count = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].requestCount > 0) count++;
        }
        return count;
    }

    // Anzahl blockierter Clients
    int getBlockedCount() {
        return blockedCount;
    }
};

#endif // RATE_LIMITER_H
