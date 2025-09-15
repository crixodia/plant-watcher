#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <uri/UriBraces.h>

#define STASSID   "SSID"            // Wi-Fi Authentication
#define STAPSK    "PASSWORD"        // Wi-Fi Authentication
#define USERNAME  "YOUR_USERNAME"   // Api Authentication
#define PASSWORD  "YOUR_PASSWORD"   // Api Aithentication
#define WAIT_FOR_RESPONSE 200

const char *ssid = STASSID;
const char *password = STAPSK;
const char *www_username = USERNAME;
const char *www_password = PASSWORD;
const char *hostname = "law";

const int port = 80;
ESP8266WebServer server(port);

void serialFlush() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}

String read2json(String data) {
  int firstComma = data.indexOf(',');
  int secondComma = data.indexOf(',', firstComma + 1);
  int thirdComma = data.indexOf(',', secondComma + 1);
  String d = data.substring(0, firstComma);
  String t = data.substring(firstComma + 1, secondComma);
  String h = data.substring(secondComma + 1, thirdComma);
  String s = data.substring(thirdComma + 1);
  return "{\"timestamp\":\"" + d + "\",\"temperature\":" + t + ",\"humidity\":" + h + ",\"soil\":" + s + "}";
}

String readSerialData(String command) {
  serialFlush();
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

String setSerialData(String command) {
  serialFlush();
  String data = "ERROR";
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

void authenticate(){
  if (!server.authenticate(www_username, www_password)) { return server.requestAuthentication(); }
}

void handleStatus() {
  authenticate();
  server.send(200, "application/json", "{\"status\":\"OK\", \"message\":\"server is running\"}");
}

void handleRead() {
  authenticate();
  server.send(200, "application/json", read2json(readSerialData("/read")));
}

void handleSetDate() {
  authenticate();
  if (server.method() != HTTP_PUT) { server.send(405, "application/json", "{\"message\":\"method not allowed.\"}"); }
  
  String response = setSerialData("/set date " + server.pathArg(0));
  if (response == "ERROR") {
    server.send(501, "application/json", "{\"message\":\"there was an error.\"}");
    return;
  }

  server.send(200, "application/json", "{\"message\":\"" + response + "\"}");
}

void setup(void) {
  Serial.begin(9600);

  WiFi.hostname(hostname);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(1000);
    ESP.restart();
  }

  ArduinoOTA.begin();

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
  server.on(UriBraces("/set/date/{}"), handleSetDate);

  server.begin();
  Serial.println("HTTP listening on port " + String(port));
}

void loop(void) {
  ArduinoOTA.handle();
  server.handleClient();
  MDNS.update();
}
