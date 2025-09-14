#include <DHT.h>
#include <SoftwareSerial.h>

#define BAUD_RATE 9600

DHT dht(12, DHT11);
SoftwareSerial espSerial(2, 3);  // RX, TX

float temperature;  // Temperatura
float humidity;     // Humidity
float soil;         // Soil humidity

void setup() {
  Serial.begin(BAUD_RATE);
  espSerial.begin(BAUD_RATE);
  dht.begin();
  delay(5000);
}

void loop() {
  if(!espSerial.available()){
    return -1;
  }

  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  soil = analogRead(A0);

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("There is a problem reading sensor data.");
    return -1;
  }

  if (espSerial.available()) {
    String espData = espSerial.readStringUntil('\n');
    espData.trim();
    Serial.println("COMMAND: " + espData);
    if (espData == "/read") {
      String data = String(temperature, 1) + "," + String(humidity, 1) + "," + String(soil);
      espSerial.println(data);
      Serial.println("RESPONSE: " + data);
    } else {
      Serial.println(espData + " is not a known instruction.");
    }
  }
}
