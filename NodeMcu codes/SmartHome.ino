#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "Solo Xr";
const char* password = "SoloCodd";

ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "NodeMCU is running!");
}

void handleCommand() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  DynamicJsonDocument json(1024);

  DeserializationError error = deserializeJson(json, server.arg("plain"));

  if (error) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  bool relay1State = json["relay1"];
  bool relay2State = json["relay2"];
  bool relay3State = json["relay3"];

  // Send the command to the Arduino
  // Adjust the code here to send the command via serial communication to the Arduino

  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  server.on("/", handleRoot);
  server.on("/command", handleCommand);
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();

  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    Serial.println("Received data from Arduino: " + data);

    DynamicJsonDocument json(1024);
    json["data"] = data;

    String jsonString;
    serializeJson(json, jsonString);

    // Send the data to the server as JSON
    // Adjust the server endpoint and headers as per your requirements
   // HTTPClient http;
    // http.begin("http://your-server-endpoint");
    // http.addHeader("Content-Type", "application/json");
    // int httpResponseCode = http.POST(jsonString);
    // http.end();

    // Example output for testing
    Serial.println("Sending data to server: " + jsonString);
  }
}
