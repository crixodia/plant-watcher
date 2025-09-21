#include <DHT.h>
#include <SoftwareSerial.h>

#define BAUD_RATE 115200
#define DHT_FREQUENCY 2000

DHT dht(12, DHT11);
SoftwareSerial espSerial(2, 3);  // RX, TX

float temperature = 0.0;
float humidity = 0.0;
float soil = 0.0;
static unsigned long lastRead = 0;

void setup() {
  Serial.begin(BAUD_RATE);
  espSerial.begin(BAUD_RATE);
  dht.begin();

  Serial.println(F("Arduino sender ready"));
}

void loop() {
  // Read sensors periodically
  if (millis() - lastRead > DHT_FREQUENCY) {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    soil = analogRead(A0);
    lastRead = millis();
  }

  // Validate readings
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println(F("Error: Could not read sensor data."));
    return;
  }

  // Check for commands from ESP
  if (espSerial.available()) {
    String espData = espSerial.readStringUntil('\n');
    espData.trim();
    Serial.println("COMMAND: " + espData);

    if (espData == "/read") {
      char tbuf[6], hbuf[6], data[32];
      dtostrf(temperature, 3, 2, tbuf);
      dtostrf(humidity, 3, 2, hbuf);

      snprintf(data, sizeof(data), "%s,%s,%d", tbuf, hbuf, (int)soil);

      espSerial.println(data);
      Serial.println(data);
    } else {
      Serial.println(espData + " is not a valid instruction.");
    }
  }
}
