#include <DHT.h>
#include <SoftwareSerial.h>

DHT dht(12, DHT11);
SoftwareSerial espSerial(2, 3); // RX, TX

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  dht.begin();
}

void loop() {
  float tc = dht.readTemperature();
  float hu = dht.readHumidity();
  float sh = analogRead(A0);

  if (isnan(tc) || isnan(hu)) {
    // Does not send something if sensor fails
  } else {
    String data = String(tc, 1) + "," + String(hu, 1) + "," + String(sh);
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
