#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK "PASSWORD"
#define WAIT_FOR_RESPONSE 100
#endif

const char *ssid = STASSID;
const char *password = STAPSK;
const char *hostname = "law";

const int port = 80;
ESP8266WebServer server(port);

String read2json(String data) {
  int firstComma = data.indexOf(',');
  int secondComma = data.indexOf(',', firstComma + 1);
  String t = data.substring(0, firstComma);
  String h = data.substring(firstComma + 1, secondComma);
  String s = data.substring(secondComma + 1);
  return "{\"temperature\":" + t + ",\"humidity\":" + h + ",\"soil\":" + s + "}";
}

String readSerialData(String command) {
  String data = "null,null,null";
  Serial.println(command);

  unsigned long start = millis(); 
  while (millis() - start < WAIT_FOR_RESPONSE) {
    if (Serial.available()) {
      data = Serial.readStringUntil('\n');
      data.trim();
      break;
    }
  }
  return data;
}

void handleStatus() {
  server.send(200, "application/json", "{\"status\":\"OK\", \"message\":\"server is running\"}");
}

void handleRead() {
  server.send(200, "application/json", read2json(readSerialData("/read")));
}

void setup(void) {
  Serial.begin(9600);

  WiFi.hostname(hostname);
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");

  Serial.print("SSID: ");
  Serial.println(ssid);

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  if (MDNS.begin("esp8266")) {
    Serial.println("mDNS initialized");
  }

  server.on("/status", handleStatus);
  server.on("/read", handleRead);

  server.begin();
  Serial.println("HTTP listening on port " + String(port));
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
