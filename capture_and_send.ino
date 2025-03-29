#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include <Preferences.h>

const int API_KEY_SIZE = 40;
char apiKeyBuffer[API_KEY_SIZE + 1];
const char* lambdaUrl = "https://your-api-id.execute-api.region.amazonaws.com/stage/resource";

String ssid;
String password;
String apiKey;

void initWiFi(const char* ssid, const char* password) {
    // Begin WiFi connection
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
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

/**
 * Captures an image from the Xiao ESP32S3 camera and sends it to AWS Lambda
 * 
 * @param lambda_url The URL of your AWS Lambda function (with API Gateway)
 * @param api_key Optional API key for the Lambda function (pass empty string if not needed)
 * @return true if successful, false otherwise
 */
bool captureAndSendImageToLambda() {
  // Get camera frame buffer
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return false;
  }
  
  Serial.printf("Image captured, size: %d bytes\n", fb->len);
  
  
  bool success = false;
  HTTPClient http;
  
  Serial.printf("Sending image to Lambda: %s\n", lambda_url);
  
  // Configure HTTP request
  http.begin(lambda_url);
  http.addHeader("Content-Type", "image/jpeg");
  
  // Add API key if provided
  if (strlen(apiKeyBuffer) > 0) {
    http.addHeader("x-api-key", apiKeyBuffer);
  }
  
  // Send HTTP POST request with image data
  int httpResponseCode = http.POST(fb->buf, fb->len);
  
  if (httpResponseCode > 0) {
    // Request was successful
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    String payload = http.getString();
    Serial.println("Response:");
    Serial.println(payload);
    success = true;
  } else {
    // Request failed
    Serial.printf("HTTP Error: %d\n", httpResponseCode);
    Serial.println(http.errorToString(httpResponseCode).c_str());
    success = false;
  }
  
  // Clean up
  http.end();
  esp_camera_fb_return(fb);
  
  return success;
}

// bool getApiKey(char* buffer) {
//   Preferences preferences;
//   preferences.begin("wandofidentify", true);
//   String apiKey = preferences.getString("api_key", "no-key-found");
//   preferences.end();
//   if (apiKey == "no-key-found") {
//     Serial.println("No API key found");
//     return false;
//   }
  
//   strncpy(buffer, apiKey.c_str(), API_KEY_SIZE - 1);
//   buffer[API_KEY_SIZE - 1] = '\0'; // Fixed: Changed from API_KEY_SIZE to API_KEY_SIZE - 1
//   return true;
// }

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  Serial.println("Serial initialized");
  Preferences preferences;
  preferences.begin("wandofidentify", true);
  apiKey = preferences.getString("api_key", "notfound");
  if (apiKey == "notfound") {
    Serial.println("api key not found");
  }

  password = preferences.getString("password", "notfound");
  if (password == "notfound") {
    Serial.println("password not found");
  }

  ssid = preferences.getString("ssid", "notfound");
  if (ssid == "notfound") {
    Serial.println("ssid not found");
  }
  preferences.end();
  strncpy(apiKeyBuffer, apiKey.c_str(), API_KEY_SIZE);
  apiKeyBuffer[API_KEY_SIZE] = '\0';


  initWiFi(ssid.c_str(), password.c_str());
}

void loop() {
    bool result = captureAndSendImageToLambda();
  
  if (result) {
    Serial.println("Successfully sent image to Lambda");
  } else {
    Serial.println("Failed to send image to Lambda");
  }
  while(true){}
}

bool checkTouch() {
  // Using GPIO pin 2 which supports touch sensing on ESP32
  const int touchPin = 2;  
  const int threshold = 40; // Adjust this threshold based on testing
  
  int touchValue = touchRead(touchPin);
  
  // Lower value means touched (capacitance increases)
  if (touchValue < threshold) {
    return true;
  }
  return false;
}
