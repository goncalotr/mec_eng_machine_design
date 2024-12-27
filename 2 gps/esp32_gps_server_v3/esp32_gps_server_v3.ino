#include <TinyGPS.h>
#include <WiFi.h>

const char* ssid     = "DESKTOP_13212";  // Replace with your Wi-Fi SSID
const char* password = "7x9Q888123XFfg"; // Replace with your Wi-Fi password

WiFiServer server(80);

TinyGPS gps;

String htmlResponse;

void setup() {
    Serial.begin(115200);           // For debugging via USB
    Serial2.begin(115200, SERIAL_8N1, 16, 17);  // Use Serial2 for GPS (RX = 16, TX = 17)

    Serial.print("Connecting to ");
    Serial.println(ssid);

    // Static IP Configuration (Optional but recommended)
    IPAddress local_IP(192, 168, 137, 101);
    IPAddress gateway(192, 168, 137, 1);
    IPAddress subnet(255, 255, 255, 0);
    // Optional: Set DNS servers if needed
    // IPAddress primaryDNS(8, 8, 8, 8);
    // IPAddress secondaryDNS(8, 8, 4, 4);

    if (!WiFi.config(local_IP, gateway, subnet)) {
        Serial.println("Static IP Configuration failed!");
    }

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
}

void loop() {
    smartdelay(1000); // Get GPS data for 1 second

    WiFiClient client = server.available();
    float latitude, longitude;
    unsigned long age, date, time_val;

    gps.f_get_position(&latitude, &longitude, &age);

    int year, month, day, hour, minute, second;

    if (age == TinyGPS::GPS_INVALID_AGE || age > 5000) {
        // Default values if GPS fix is not available or too old
        latitude = 37.235;
        longitude = -115.811;
        year = 2024;
        month = 11;
        day = 6;
        hour = 0;
        minute = 0;
        second = 0;
    } else {
        // Get date and time from GPS
        gps.get_datetime(&date, &time_val, &age);

        // Extract date and time components correctly
        year = (date / 10000) + 2000;
        month = (date % 10000) / 100;
        day = date % 100;

        hour = time_val / 1000000;
        minute = (time_val % 1000000) / 10000;
        second = (time_val % 10000) / 100;
    }

    if (client) {
        Serial.println("New client connected");
        String requestLine = "";

        while (client.connected()) {
            if (client.available()) {
                char c = client.read();

                if (c == '\n') {
                    if (requestLine.length() == 0) {
                        // Construct the HTML response using String objects correctly:
                        htmlResponse = "<!DOCTYPE html><html lang='en'>";
                        htmlResponse += "<head><meta charset='UTF-8'>";
                        htmlResponse += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
                        htmlResponse += "<title>GPS Location</title>";
                        htmlResponse += "<link rel='stylesheet' href='https://unpkg.com/leaflet@1.7.1/dist/leaflet.css' />";
                        htmlResponse += "<style>#map { height: 400px; width: 100%; }</style></head>";
                        htmlResponse += "<body><h1>GPS Location</h1>";

                        // Display Coordinates, Date, and Time (Corrected String concatenation):
                        htmlResponse += "<p>Latitude: " + String(latitude, 6) + "</p>";
                        htmlResponse += "<p>Longitude: " + String(longitude, 6) + "</p>";
                        htmlResponse += "<p>Date: " + String(year) + "-" + (month < 10 ? "0" : "") + String(month) + "-" + (day < 10 ? "0" : "") + String(day) + "</p>"; // Correct date format
                        htmlResponse += "<p>Time: " + (hour < 10 ? "0" : "") + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute) + ":" + (second < 10 ? "0" : "") + String(second) + "</p>"; // Correct time format

                        // Leaflet Map Setup (Corrected String concatenation):
                        htmlResponse += "<div id='map'></div>";
                        htmlResponse += "<script src='https://unpkg.com/leaflet@1.7.1/dist/leaflet.js'></script>";
                        htmlResponse += "<script>";
                        htmlResponse += "var map = L.map('map').setView([" + String(latitude, 6) + ", " + String(longitude, 6) + "], 13);";
                        htmlResponse += "L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', { maxZoom: 19, attribution: '© OpenStreetMap' }).addTo(map);";
                        htmlResponse += "L.marker([" + String(latitude, 6) + ", " + String(longitude, 6) + "]).addTo(map).bindPopup('Current Location').openPopup();";
                        htmlResponse += "</script>";
                        htmlResponse += "</body></html>";

                        // Send the HTML response (no changes here)
                        client.print("HTTP/1.1 200 OK\r\n");
                        client.print("Content-Type: text/html\r\n");
                        client.print("Connection: close\r\n");
                        client.print("\r\n");
                        client.print(htmlResponse);
                        break;
                    } else {
                        requestLine = "";
                    }
                } else if (c != '\r') {
                    requestLine += c;
                }
            }
        }
        delay(1);
        client.stop();
        Serial.println("Client disconnected");
    }
}

// Function to continuously get GPS data
static void smartdelay(unsigned long ms) {
    unsigned long start = millis();
    do {
        while (Serial2.available()) {
            gps.encode(Serial2.read());
        }
    } while (millis() - start < ms);
}