#include <TimeLib.h>

#include <DHT.h>
#include <SoftwareSerial.h>


#define BAUD_RATE 9600
#define DHT11_FRECUENCY 2000

DHT dht(12, DHT11);
SoftwareSerial espSerial(2, 3);  // RX, TX

float temperature;  // Temperatura
float humidity;     // Humidity
float soil;         // Soil humidity
static unsigned long lastRead = 0;

const char* getDate() {
  static char buffer[15];
  snprintf(
    buffer,
    sizeof(buffer),
    "%02d%02d%02d%02d%02d%04d",
    hour(), minute(), second(), day(), month(), year());
  return buffer;
}

void printDate() {
  Serial.print("DATE: ");
  Serial.println(getDate());
}

void setup() {
  Serial.begin(BAUD_RATE);
  espSerial.begin(BAUD_RATE);
  dht.begin();

  unsigned long start = millis();
  while (millis() - start < 5000) {}
  printDate();
}

void loop() {
  if (millis() - lastRead > DHT11_FRECUENCY) {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    soil = analogRead(A0);
    lastRead = millis();
  }

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("There is a problem reading sensor data.");
    return;
  }

  if (espSerial.available()) {

    String espData = espSerial.readStringUntil('\n');
    espData.trim();
    Serial.println("COMMAND: " + espData);

    if (espData == "/read") {
      char tbuf[6], hbuf[6], data[33];

      dtostrf(temperature, 3, 2, tbuf);
      dtostrf(humidity, 3, 2, hbuf);
      snprintf(data, sizeof(data), "%s,%s,%s,%d", getDate(), tbuf, hbuf, (int)soil);

      espSerial.println(data);
      Serial.println(data);
    } else if (espData.startsWith("/set")) {
      String subcommand = espData.substring(5, 9);
      String value = espData.substring(10, 26);
      if (subcommand == "date") {
        setTime(
          value.substring(0, 2).toInt(),   // Hour
          value.substring(2, 4).toInt(),   // Minute
          value.substring(4, 6).toInt(),   // Second
          value.substring(6, 8).toInt(),   // Day
          value.substring(8, 10).toInt(),  // Month
          value.substring(10, 14).toInt()  // Year
        );
        espSerial.println("OK");
        printDate();
      } else {
        Serial.println(subcommand + "is not a valid subcommand for the value " + value + ".");
      }

    } else {

      Serial.println(espData + " is not a valid instruction.");
    }
  }
}
