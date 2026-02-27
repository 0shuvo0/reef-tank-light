#include "ota.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>   // ← add this include

// ====== CONFIG ======
static const char* VERSION_URL =
  "https://raw.githubusercontent.com/0shuvo0/reef-tank-light/refs/heads/main/ota_publish/version.json";

#ifndef FW_VERSION
  #error "FW_VERSION is not defined. Set it in platformio.ini build_flags, e.g. -D FW_VERSION=\"1.0.0\""
#endif
// ====================

// Stores the bin URL discovered during checkFirmwareUpdate()
static String g_binUrl;

// "1.2.10" compare helper: -1 if a<b, 0 if equal, 1 if a>b
static int compareSemver(const String& a, const String& b) {
  int a1=0,a2=0,a3=0, b1=0,b2=0,b3=0;
  sscanf(a.c_str(), "%d.%d.%d", &a1, &a2, &a3);
  sscanf(b.c_str(), "%d.%d.%d", &b1, &b2, &b3);

  if (a1 != b1) return (a1 > b1) ? 1 : -1;
  if (a2 != b2) return (a2 > b2) ? 1 : -1;
  if (a3 != b3) return (a3 > b3) ? 1 : -1;
  return 0;
}

static String httpGetString(const char* url) {
  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  if (!http.begin(url)) {
    Serial.println("HTTP begin() failed");
    return "";
  }

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("GET failed, code=%d\n", code);
    http.end();
    return "";
  }

  String payload = http.getString();
  http.end();
  return payload;
}

bool checkFirmwareUpdate() {
  g_binUrl = "";

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("OTA check: WiFi not connected");
    return false;
  }

  Serial.println("OTA: Checking version.json...");
  String json = httpGetString(VERSION_URL);
  if (json.isEmpty()) {
    Serial.println("OTA: version.json download failed");
    return false;
  }
  Serial.println(json);
  JsonDocument doc;
  auto err = deserializeJson(doc, json);
  if (err) {
    Serial.print("OTA: JSON parse error: ");
    Serial.println(err.c_str());
    return false;
  }

  const char* latest = doc["version"] | "";
  const char* binUrl  = doc["bin"] | "";

  if (strlen(latest) == 0 || strlen(binUrl) == 0) {
    Serial.println("OTA: version.json missing 'version' or 'bin'");
    return false;
  }

  Serial.printf("OTA: Current=%s Latest=%s\n", FW_VERSION, latest);

  if (compareSemver(String(FW_VERSION), String(latest)) >= 0) {
    Serial.println("OTA: No update available");
    return false;
  }

  g_binUrl = String(binUrl);
  Serial.print("OTA: Update available, bin=");
  Serial.println(g_binUrl);
  return true;
}

bool updateFirmware() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("OTA update: WiFi not connected");
    return false;
  }
  if (g_binUrl.isEmpty()) {
    Serial.println("OTA update: No bin URL set (run checkFirmwareUpdate() first)");
    return false;
  }

  Serial.println("OTA: Starting firmware update...");

  httpUpdate.onProgress([](int cur, int total) {
    if (total > 0) {
      Serial.printf("OTA: %d%%\r", (cur * 100) / total);
    }
  });

  httpUpdate.rebootOnUpdate(true);

  WiFiClientSecure client;          // ← was WiFiClient
  client.setInsecure();             // ← skip cert verification (GitHub has valid certs, 
                                    //   but you'd need to bundle the root CA otherwise)

  t_httpUpdate_return ret = httpUpdate.update(client, g_binUrl);

  Serial.println();

  if (ret == HTTP_UPDATE_OK) {
    Serial.println("OTA: Update OK (rebooting...)");
    return true;
  }

  if (ret == HTTP_UPDATE_NO_UPDATES) {
    Serial.println("OTA: No updates (unexpected)");
  } else {
    Serial.printf("OTA: Update failed. Error(%d): %s\n",
                  httpUpdate.getLastError(),
                  httpUpdate.getLastErrorString().c_str());
  }

  return false;
}