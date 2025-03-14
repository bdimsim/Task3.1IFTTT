#include <Arduino.h>
#include <SPI.h>      // Required for the WiFiNINA library
#include <WiFiNINA.h> // Enables WiFi connectivity
#include <BH1750.h>   // Library for light sensor
#include "secrets.h"  // Contains SSID and password

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const char *const myAPIKey = SECRET_IFTTT_APIKEY;

WiFiClient client;
BH1750 lightSensor;

bool sunlightDetected = false;

void sendToIFTTT(float lux, const char *event)
{
  if (client.connect("maker.ifttt.com", 80)) // Connect to IFTTT if not connected
  {
    Serial.println("Connected to IFTTT.");
    client.println("GET /trigger/" + String(event) + "/with/key/" + myAPIKey + "?value1=" + String(lux) + " HTTP/1.1");
    client.println("Host: maker.ifttt.com");
    client.println("Connection: close");
    client.println(); // end HTTP header

    while (client.connected()) // Loop while connected to the server
    {
      if (client.available()) // If there is data available
      {
        // Read data from the server and print it to serial monitor
        Serial.print((char)client.read());
      }
    }

    client.stop(); // closes the connection
    Serial.println("Data sent to IFTTT. The connection is now closed.");
  }
  else
  {
    Serial.println("connection to IFTTT failed.");
  }
}

void connectWifi()
{
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED)
  {
    Serial.print(".");
    delay(5000);
  }
  Serial.println("Connected to WiFi.\n");
}

void setup()
{
  connectWifi();
  Serial.begin(115200); // Initialize the serial port
  while (!Serial);  // Wait for the serial port to be ready
  Wire.begin(); // Enables I2C communication (SDA and SCL pins)
  lightSensor.begin();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Wifi lost. Reconnecting...");
    connectWifi();
  }

  float lux = lightSensor.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lux");

  if (lux < 0)
  {
    Serial.println("Error: Could not read data from sensor.");
  }
  else if (lux <= 400 && sunlightDetected)
  {
    sunlightDetected = false;
    sendToIFTTT(lux, "sunlight_stopped");
  }
  else if (lux >= 400 && !sunlightDetected)
  {
    sunlightDetected = true;
    sendToIFTTT(lux, "sunlight_detected");
  }
  delay(1000); // 1 second delay between readings
}
