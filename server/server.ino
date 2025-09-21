// python -m esptool --chip esp8266 --port COM5 --baud 115200 --after hard_reset erase_flash
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <uri/UriBraces.h>
#include <time.h>

#define WAIT_FOR_RESPONSE 200
#define BAUD_RATE 115200

const char *ssid = "SSID";             // Wi-Fi Authentication
const char *password = "PASSWORD";    // Wi-Fi Authentication
const char *www_username = "USERNAME";    // API Authentication
const char *www_password = "PASSWORD";  // API Authentication
const char *hostname = "law";

const int port = 80;
ESP8266WebServer server(port);

// NTP configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -5 * 3600;     // adjust for your timezone
const int daylightOffset_sec = 0; // adjust for DST if needed

void sendError(int code, const char *msg) {
  char buf[64];
  snprintf(buf, sizeof(buf), "{\"message\":\"%s\"}", msg);
  server.send(code, F("application/json"), buf);
}

String addTimestampToJson(const String& data) {
  int firstComma = data.indexOf(',');
  int secondComma = data.indexOf(',', firstComma + 1);

  if (firstComma < 0 || secondComma < 0) {
    return F("{\"error\":\"invalid sensor data\"}");
  }

  String t = data.substring(0, firstComma);
  String h = data.substring(firstComma + 1, secondComma);
  String s = data.substring(secondComma + 1);

  // Get current timestamp
  time_t now = time(nullptr);
  struct tm *tm = gmtime(&now);

  char ts[25];
  strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm); // ISO 8601 UTC

  char jsonBuffer[128];
  snprintf(jsonBuffer, sizeof(jsonBuffer),
           "{\"timestamp\":\"%s\",\"temperature\":%s,\"humidity\":%s,\"soil\":%s}",
           ts, t.c_str(), h.c_str(), s.c_str());

  return String(jsonBuffer);
}

String sendSerialCommand(const String &command, const char *defaultValue = "null,null,null") {
  String data = defaultValue;
  Serial.println(command);

  unsigned long start = millis();
  while (millis() - start < WAIT_FOR_RESPONSE) {
    if (Serial.available()) {
      data = Serial.readStringUntil('\n');
      data.trim();
      //server.handleClient();
      //yield():
      break;
    }
    delay(1);
  }
  return data;
}

void authenticate() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
}

void handleStatus() {
  authenticate();
  server.send(200, F("application/json"), F("{\"status\":\"OK\",\"message\":\"server is running\"}"));
}

void handleRead() {
  authenticate();
  String sensorData = sendSerialCommand("/read");
  server.send(200, "application/json", addTimestampToJson(sensorData));
}

void setup(void) {
  Serial.begin(BAUD_RATE);
  delay(200);

  WiFi.hostname(hostname);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(3000);
    ESP.restart();
  }

  // Init mDNS
  if (MDNS.begin(hostname)) {
    Serial.println(F("mDNS initialized"));
  }

  // Init NTP time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  Serial.println(F("NTP time configured"));
  Serial.print(F("SSID: ")); Serial.println(ssid);
  Serial.print(F("IP: "));   Serial.println(WiFi.localIP());
  Serial.print(F("MAC: "));  Serial.println(WiFi.macAddress());

  // Web server endpoints
  server.on("/status", handleStatus);
  server.on("/read", handleRead);

  server.begin();
  Serial.println(F("HTTP listening on port 80"));
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
