#include <WiFi.h>
#include "ota.h"

const char* ssid = "TO BE OR NOT TO BE";
const char* password = "22744623";

WiFiServer server(80);
bool WIFI_CONNECTED = false;

//built in led
const int ledPin = 2;

bool connectToWiFi(uint32_t timeoutMs) {
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  uint32_t start = millis();

  while (WiFi.status() != WL_CONNECTED &&
         (millis() - start) < timeoutMs) {

    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println("\nWiFi connection timed out.");
  WiFi.disconnect(true);
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(ledPin, OUTPUT);

  WIFI_CONNECTED = connectToWiFi(30000);  // 30 seconds max

  if (WIFI_CONNECTED) {
    server.begin();

    bool hasFirmwareUpdate = checkFirmwareUpdate();
    if (hasFirmwareUpdate) {
      updateFirmware();  // will reboot on success
    }
  } else {
    Serial.println("Skipping OTA check (no WiFi).");
  }
}


void loop() {
  // if(!WIFI_CONNECTED) {
  //   // Don't attempt to serve if WiFi isn't connected
  //   delay(1000);
  //   Serial.println("WiFi not connected. Waiting..."); 
  //   return;
  // }

  // WiFiClient client = server.available();

  // if (client) {
  //   Serial.println("New Client Connected");

  //   String request = client.readStringUntil('\r');
  //   Serial.println(request);
  //   client.flush();

  //   // Send HTTP response
  //   client.println("HTTP/1.1 200 OK");
  //   client.println("Content-type:text/html");
  //   client.println("Connection: close");
  //   client.println();

  //   client.println("<!DOCTYPE html><html>");
  //   client.println("<head><title>ESP32 Web Server</title></head>");
  //   client.println("<body>");
  //   client.println("<h1>Hello from ESP32!</h1>");
  //   client.println("<p>WiFi Web Server is running.</p>");
  //   client.println("</body></html>");

  //   client.println();
  //   client.stop();
  //   Serial.println("Client Disconnected");
  // }
  

  // Blink the built-in LED to indicate the device is alive
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);
  delay(100);
}