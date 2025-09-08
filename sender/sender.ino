#include <DHT.h>
#include <SoftwareSerial.h>

DHT dht(12, DHT11);
SoftwareSerial espSerial(2, 3); // RX, TX

float temperature;  // Temperatura
float humidity;     // Humidity
float soil;         // Soil humidity

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  dht.begin();
}

void loop() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  soil = analogRead(A0);

  if (isnan(temperature) || isnan(humidity)) {
    // Does not send something if sensor fails
  } else {
    String data = String(temperature, 1) + "," + String(humidity, 1) + "," + String(soil);
    espSerial.println(data);
    // Serial.println(data);
  }

  if (espSerial.available()) {
    String espData = espSerial.readStringUntil('\n');
    if(!espData.equals("")){
      Serial.println("ESP-01: " + espData);
    }
  }

  delay(1000);
}
