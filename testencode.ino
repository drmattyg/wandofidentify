#include <WiFi.h>
#include <HTTPClient.h>
#include <base64.h>
#include <Preferences.h>

const char* imageUrl = ""https://picsum.photos/128/128";"

#include <WiFi.h>
#include <HTTPClient.h>
#include "Base64.h" // Use a Base64 library or implement your own








void initWiFi() {
  Preferences preferences;
  preferences.begin("wandofidentify", true);
  
  password = preferences.getString("password", "notfound");
  if (password == "notfound") {
    Serial.println("password not found");
  }

  ssid = preferences.getString("ssid", "notfound");
  if (ssid == "notfound") {
    Serial.println("ssid not found");
  }
  preferences.end();

    // Begin WiFi connection
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Wait for connection with timeout
    int timeout = 0;
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED && timeout < 30) {
        delay(1000);
        Serial.print(".");
        timeout++;
    }
    
    // Check if connected successfully
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi network!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi!");
        // You might want to handle the failure case here
        // For example, restart the ESP32 or retry connection
    }
}

void setup() {
  Serial.begin(115200);
   while (!Serial) {
    ; // wait for serial port to connect
  }
  Serial.setDebugOutput(true);


 

  HTTPClient http;
  http.begin(imageUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    int len = http.getSize();
    WiFiClient* stream = http.getStreamPtr();

    // Allocate buffer for image (use with caution, may crash on large images)
    uint8_t* imgData = (uint8_t*)malloc(len);
    if (!imgData) {
      Serial.println("Memory allocation failed!");
      return;
    }

    int index = 0;
    while (http.connected() && len > 0) {
      size_t available = stream->available();
      if (available) {
        int readLen = stream->readBytes(imgData + index, available);
        index += readLen;
        len -= readLen;
      }
      delay(1);
    }

    // Base64 encode the image
    String encodedImage = base64::encode(imgData, index);
    Serial.println("Base64 Encoded Image:");
    Serial.println(encodedImage);

    free(imgData);
  } else {
    Serial.printf("Failed to fetch image. HTTP Code: %d\n", httpCode);
  }

  http.end();
}

void loop() {
  // nothing here
}