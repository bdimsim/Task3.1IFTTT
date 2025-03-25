#include <WiFiNINA.h> // Enables WiFi connectivity
#include <BH1750.h>   // Library for light sensor
#include "Secrets.h"  // Contains SSID, password, and API

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const char *const myAPIKey = SECRET_IFTTT_APIKEY;

WiFiClient client;
BH1750 lightSensor;

constexpr unsigned long sunlightTimeThreshold = 1000 * 60 * 60 * 2; // 2 hours in milliseconds
unsigned long sunlightStartTime = 0;
bool sunlightDetected = false;

void connectWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi lost. Reconnecting...");
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    while (WiFi.begin(ssid, pass) != WL_CONNECTED) delay(5000);
    Serial.println("Connected to WiFi.\n");
  }
}

void sendToIFTTT(const char *const event) {
  if (!client.connect("maker.ifttt.com", 80)) {
    Serial.println("Connection to IFTTT failed.");
    return;
  }

  Serial.println("Connected to IFTTT.");

  // Send HTTP GET request to IFTTT
  client.println("GET /trigger/" + String(event) + "/with/key/" + myAPIKey + " HTTP/1.1");
  client.println("Host: maker.ifttt.com");
  client.println("Connection: close");
  client.println(); // end HTTP header

  while (client.connected()) { 
    if (client.available()) { // If there is data available
      Serial.print((char)client.read()); // Reads server response
    }
  }
  client.stop(); // closes the connection
}

void setup() {
  connectWifi();
  Serial.begin(115200); // Initialise the serial port
  while (!Serial); // Wait for the serial port to be ready
  Wire.begin(); // Enables I2C communication (SDA and SCL pins)
  lightSensor.begin(); // Begin measurement
}

void loop() {
  connectWifi();

  // Read and print light level to serial monitor
  float lux = lightSensor.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lux");

  if (lux < 0) {
    Serial.println("Error: Could not read data from sensor.");
  }
  else if (lux < 400 && sunlightDetected) {
    sunlightDetected = false;
  }
  else if (lux >= 400 && !sunlightDetected) {
    sunlightDetected = true; // Toggle sunlight detection when threshold is reached
    sunlightStartTime = millis(); // Timer value when sunlight is detected
  }

  if (sunlightDetected && (millis() - sunlightStartTime >= sunlightTimeThreshold)) {
    sendToIFTTT("2_hours_of_sunlight");
    sunlightDetected = false;
  }
  delay(1000); // 1 second delay between readings
}
