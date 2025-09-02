#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// HTML and CSS header
#include "webpage_content.h"

#ifndef STASSID
  #define STASSID "SSID"
  #define STAPSK "WIFI_PWD"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;
const char *hostname = "HOSTNAME";

const int port = 80;
ESP8266WebServer server(port);

const int MAX_HISTORY = 60;  // Max number of entries
String history[MAX_HISTORY];
int historyIndex = 0;
int historyCount = 0;

// Stores a new string
void addToHistory(String data) {
  history[historyIndex] = data;
  historyIndex = (historyIndex + 1) % MAX_HISTORY;
  if (historyCount < MAX_HISTORY) historyCount++;
}

// Converts history into a json format
String getHistoryJson() {
  String out = "[";
  for (int i = 0; i < historyCount; i++) {
    int idx = (historyIndex - historyCount + MAX_HISTORY + i) % MAX_HISTORY;
    String entry = history[idx];

    int firstComma = entry.indexOf(',');
    int secondComma = entry.indexOf(',', firstComma + 1);
    String t = entry.substring(0, firstComma);
    String h = entry.substring(firstComma + 1, secondComma);
    String s = entry.substring(secondComma + 1);
    out += "{\"temperature\":" + t + ",\"humidity\":" + h + ",\"soil\":" + s + "}";
    if (i < historyCount - 1) out += ",";
  }
  out += "]";
  return out;
}

// Last entry json format
String getLastJson() {
  if (historyCount == 0) return "{}";
  int idx = (historyIndex + MAX_HISTORY - 1) % MAX_HISTORY;
  String entry = history[idx];

  int firstComma = entry.indexOf(',');
  int secondComma = entry.indexOf(',', firstComma + 1);
  String t = entry.substring(0, firstComma);
  String h = entry.substring(firstComma + 1, secondComma);
  String s = entry.substring(secondComma + 1);
  return "{\"temperature\":" + t + ",\"humidity\":" + h + ",\"soil\":" + s + "}";
}

// Reads serial data in csv format
void readSerialData() {
  while (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    data.trim();
    int firstComma = data.indexOf(',');
    int secondComma = data.indexOf(',', firstComma + 1);
    if (firstComma != -1 && secondComma != -1) {
      addToHistory(data);
    }
  }
}

// Root page
void handleRoot() {
  readSerialData();
  Serial.println("/");
  
  // Creates the data table
  String historyTable = "<table><tr><th>#</th><th>Temperatura (°C)</th><th>Humedad (%)</th><th>Suelo</th></tr>";
  for (int i = 0; i < historyCount; i++) {
    int idx = (historyIndex - historyCount + MAX_HISTORY + i) % MAX_HISTORY;
    String entry = history[idx];
    int firstComma = entry.indexOf(',');
    int secondComma = entry.indexOf(',', firstComma + 1);
    if (firstComma != -1 && secondComma != -1) {
      String t = entry.substring(0, firstComma);
      String h = entry.substring(firstComma + 1, secondComma);
      String s = entry.substring(secondComma + 1);
      historyTable += "<tr><td>" + String(i + 1) + "</td><td>" + t + "</td><td>" + h + "</td><td>" + s + "</td></tr>";
    }
  }
  historyTable += "</table>";
  
  // Merge header content and footer
  String html = String(PAGE_HTML_HEAD) + historyTable + String(PAGE_HTML_FOOT_START_SCRIPT) + getHistoryJson() + String(PAGE_HTML_FOOT_END_SCRIPT);
  server.send(200, "text/html", html);
}

// All history api call
void handleApi() {
  readSerialData();
  Serial.println("/api");
  server.send(200, "application/json", getHistoryJson());
}

// Last entry api call
void handleApiLast() {
  readSerialData();
  Serial.println("/api/last");
  server.send(200, "application/json", getLastJson());
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

  server.on("/", handleRoot);
  server.on("/api", handleApi);
  server.on("/api/last", handleApiLast);

  server.begin();
  Serial.println("HTTP listening on port " + String(port));
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
