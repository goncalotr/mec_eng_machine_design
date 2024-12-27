#include <WiFi.h>

// SSID and Password for the Access Point
const char *ssid = "ESP32-AP";
const char *password = "123456789";

// Setup the server on port 80
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  
  // Start the WiFi as an Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");

  // Display the IP address of the ESP32 AP
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP Address: ");
  Serial.println(IP);

  // Start the server
  server.begin();
}

void loop() {
  // Check for incoming clients
  WiFiClient client = server.available(); 

  if (client) {
    Serial.println("New Client Connected");
    while (client.connected()) {
      if (client.available()) {
        String request = client.readStringUntil('\r');
        Serial.println("Received: " + request);
        client.flush();

        // Respond back to client
        client.print("Message received by server");
      }
    }
    client.stop();
    Serial.println("Client Disconnected");
  }
}