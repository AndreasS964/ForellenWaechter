/*
 * ForellenWächter - Sensor Test
 * Einfacher Test für alle Sensoren
 * 
 * Nutze diesen Sketch um einzelne Sensoren zu testen
 * bevor du das volle System installierst.
 */

#include <OneWire.h>
#include <DallasTemperature.h>

// Pin-Definitionen (wie im Hauptcode)
#define ONE_WIRE_BUS 4
#define PH_PIN 34
#define TDS_PIN 35
#define WATER_LEVEL_PIN 33
#define STATUS_LED 2

// Objekte
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensors(&oneWire);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n");
  Serial.println("════════════════════════════════════════");
  Serial.println("  ForellenWächter - Sensor Test");
  Serial.println("════════════════════════════════════════\n");
  
  // LED
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);
  delay(500);
  digitalWrite(STATUS_LED, LOW);
  Serial.println("✓ LED funktioniert");
  
  // Float Switch
  pinMode(WATER_LEVEL_PIN, INPUT_PULLUP);
  Serial.println("✓ Float Switch Pin konfiguriert");
  
  // Temperatursensoren
  tempSensors.begin();
  int count = tempSensors.getDeviceCount();
  Serial.printf("✓ DS18B20 Sensoren gefunden: %d\n", count);
  
  if (count > 0) {
    for (int i = 0; i < count; i++) {
      DeviceAddress addr;
      tempSensors.getAddress(addr, i);
      Serial.printf("  Sensor %d: ", i);
      for (int j = 0; j < 8; j++) {
        Serial.printf("%02X", addr[j]);
      }
      Serial.println();
    }
  } else {
    Serial.println("  ⚠️ Keine Sensoren gefunden!");
    Serial.println("  → Verkabelung prüfen");
    Serial.println("  → 4.7kΩ Pull-up vorhanden?");
  }
  
  Serial.println("\n════════════════════════════════════════");
  Serial.println("  Starte Messungen (alle 2 Sekunden)");
  Serial.println("════════════════════════════════════════\n");
}

void loop() {
  // Temperaturen
  tempSensors.requestTemperatures();
  
  float temp0 = tempSensors.getTempCByIndex(0);
  float temp1 = tempSensors.getTempCByIndex(1);
  
  Serial.println("────────────────────────────────────────");
  Serial.printf("🌡️ Temp Sensor 0: %.2f°C", temp0);
  if (temp0 == -127.0) Serial.print(" ⚠️ NICHT VERBUNDEN!");
  Serial.println();
  
  Serial.printf("🌡️ Temp Sensor 1: %.2f°C", temp1);
  if (temp1 == -127.0) Serial.print(" ⚠️ NICHT VERBUNDEN!");
  Serial.println();
  
  // pH (raw)
  int phRaw = analogRead(PH_PIN);
  float phVoltage = phRaw * 3.3 / 4095.0;
  float phValue = 7.0 + (2.5 - phVoltage) * 3.5;
  Serial.printf("🧪 pH Raw: %d, Spannung: %.2fV, pH: %.2f\n", phRaw, phVoltage, phValue);
  
  // TDS (raw)
  int tdsRaw = analogRead(TDS_PIN);
  float tdsVoltage = tdsRaw * 3.3 / 4095.0;
  Serial.printf("📊 TDS Raw: %d, Spannung: %.2fV\n", tdsRaw, tdsVoltage);
  
  // Float Switch
  bool waterLevel = digitalRead(WATER_LEVEL_PIN) == LOW;
  Serial.printf("🌊 Wasserlevel: %s (GPIO: %d)\n", 
                waterLevel ? "OK" : "NIEDRIG", 
                digitalRead(WATER_LEVEL_PIN));
  
  // LED blinken lassen
  digitalWrite(STATUS_LED, HIGH);
  delay(100);
  digitalWrite(STATUS_LED, LOW);
  
  delay(1900);
}

/*
 * ERWARTETE WERTE:
 * 
 * Temperatur:
 * - Normal: 10-25°C
 * - -127.0 = Sensor nicht verbunden
 * - 85.0 = Sensor-Fehler
 * 
 * pH:
 * - Raw: ~1000-3000 (je nach pH)
 * - Spannung: 2.0-3.0V
 * - pH: 4-10 (muss kalibriert werden!)
 * 
 * TDS:
 * - Raw: 0-4095
 * - Spannung: 0-3.3V
 * - Luft: ~0
 * - Leitungswasser: 200-500
 * - Destilliert: <50
 * 
 * Float Switch:
 * - OK = Schwimmer oben, Schalter geschlossen
 * - NIEDRIG = Schwimmer unten, Schalter offen
 */
