#include <WiFi.h>

#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <ESP32Servo.h> // Use ESP32Servo library instead of the default Servo library



// Constants

// SSID and Password of the Access Point (same as AP)
const char* ssid = "ESP32-AP";
const char* password = "123456789";

// Server IP and port (IP of ESP32 AP)
const char* host = "192.168.4.1"; // IP address of the ESP32 AP
const uint16_t port = 80;




// ToF
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

int maxDistance = 0; // Maximum detected distance
int maxAngle = 0;    // Angle with maximum distance

int measuredDistance = 0;
int forwardDistance = 0;

// SG90
Servo myServo;

int servoPin = 25;   // Servo signal connected to D25
int servoAngle = 0;  // Initial servo angle

const int minPulseWidth = 500; // 0.5 ms
const int maxPulseWidth = 2500; // 2.5 ms
const int pwmFreq = 50;

// Setup
void setup() {
  Serial.begin(115200);

  // Connect to the Wi-Fi Access Point
  WiFi.begin(ssid, password);

  Serial.print("Connecting to AP");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to AP");

  // Display the IP address assigned to the station ESP32
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());



  // Initialize ToF sensor
  if (!lox.begin()) {
    Serial.println("Failed to boot VL53L0X");
    while(1);
  }
  Serial.println("VL53L0X Ready!");

  // Servo
  myServo.attach(servoPin, minPulseWidth, maxPulseWidth);
  myServo.setPeriodHertz(pwmFreq);
 
  delay(500);
}

// Loop
void loop() {

  // Connect to the server (ESP32 AP)
  WiFiClient client;
  
  servoAngle = 0;
  maxAngle = 0;

  measuredDistance = 0;
  maxDistance = 0;
  forwardDistance = 0;

  // Sweep the servo from 0 to 180 degreess
  for (servoAngle = 0; servoAngle <= 180; servoAngle += 10) {
    myServo.write(servoAngle);  // Move the servo
    delay(300);

    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false); // Perform ToF distance measurement

    if (measure.RangeStatus != 4) {   // If the measurement is valid
      Serial.print("Angle_deg:");
      Serial.print(servoAngle);
      Serial.print(", Distance_mm:");
      measuredDistance = measure.RangeMilliMeter;
      Serial.println(measuredDistance); //ln here to be able to use the plotter
      //Serial.println(" mm");

      // Check if the current distance is the highest
      if (measuredDistance > maxDistance) {
        maxDistance = measuredDistance;
        maxAngle = servoAngle;
      }

      if (servoAngle == 90) {
        forwardDistance = measuredDistance;
      }

    } else {
      Serial.println("Measurement error!");
    }
  }

  // Print the angle with the max distance
  Serial.print("Max distance: ");
  Serial.print(maxDistance);
  Serial.print(" mm at angle: ");
  Serial.println(maxAngle-90);

  // Rotate the servo back to the max distance angle
  myServo.write(0);
  delay(300);

  if (client.connect(host, port)) {
    Serial.println("-----------------------------------------------------------------");
    Serial.println("Server Status: Connected");

    // Send a message to the server
    char buffer1[100];
    sprintf(buffer1, "Max distance: %d, Angle: %d", maxDistance, maxAngle);
    Serial.print("Sending message: ");  // Debug: Check what is being sent
    Serial.println(buffer1);
    client.print(buffer1);  // Send the message
    client.flush();  // Ensure it's sent immediately
    Serial.println("-");

    // Wait for the server response
    while (client.available() == 0) {
      delay(500);
    }
    String response1 = client.readStringUntil('\r');
    Serial.println("Server Response: " + response1);

    char buffer2[100];
    sprintf(buffer2, "Forward distance: %d, Angle: %d", forwardDistance, 0);
    Serial.print("Sending message: ");  // Debug: Check what is being sent
    Serial.println(buffer2);
    client.print(buffer2);
    client.flush();

    // Wait for the server response
    while (client.available() == 0) {
      delay(500);
    }
    
    // Read the response
    String response2 = client.readStringUntil('\r');
    Serial.println("Server Response: " + response2);

    client.stop();
    Serial.println("-----------------------------------------------------------------");
  }
  else {
    Serial.println("Connection to server failed.");
  }

  delay(1000);  // Wait for --  ms before starting the next sweep
}
