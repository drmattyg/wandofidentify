#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include <Preferences.h>


bool initWiFi(char *ssid, char *password) {
  Serial.println("Connecting to WiFi...");
  
  WiFi.begin(ssid, password);
  
  // Wait up to 10 seconds for connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected to WiFi. IP: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("Failed to connect to WiFi");
    return false;
  }
}
/**
 * Captures an image from the Xiao ESP32S3 camera and sends it to AWS Lambda
 * 
 * @param lambda_url The URL of your AWS Lambda function (with API Gateway)
 * @param api_key Optional API key for the Lambda function (pass empty string if not needed)
 * @return true if successful, false otherwise
 */
bool captureAndSendImageToLambda(const char* lambda_url) {
  // Get camera frame buffer
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return false;
  }
  
  Serial.printf("Image captured, size: %d bytes\n", fb->len);
  
  Preferences preferences;
  preferences.begin("wandofidentify", true);
  String apiKey = preferences.getString("api_key", "notfound");
  if apiKey == "notfound" {
    Serial.println("api key not found")
  }

  String password = preferences.getString("password", "notfound");
  if password == "notfound" {
    Serial.println("password not found")
  }

  String ssid = preferences.getString("ssid", "notfound");
  if ssid == "notfound" {
    Serial.println("ssid not found")
  }

  initWiFi(ssid, password);
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    esp_camera_fb_return(fb);
    return false;
  }
  
  bool success = false;
  HTTPClient http;
  
  Serial.printf("Sending image to Lambda: %s\n", lambda_url);
  
  // Configure HTTP request
  http.begin(lambda_url);
  http.addHeader("Content-Type", "image/jpeg");
  
  // Add API key if provided
  if (strlen(api_key) > 0) {
    http.addHeader("x-api-key", api_key);
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

/**
 * Example of how to use the function in your code
 * This is just for reference - you would call this from your main program
 */
void exampleUsage() {
  // AWS Lambda endpoint (from API Gateway)
  const char* lambdaUrl = "https://your-api-id.execute-api.region.amazonaws.com/stage/resource";
  const char* apiKey = "your-api-key"; // Leave empty if not using API key
  
  // Take picture and send to Lambda
  bool result = captureAndSendImageToLambda(lambdaUrl, apiKey);
  
  if (result) {
    Serial.println("Successfully sent image to Lambda");
  } else {
    Serial.println("Failed to send image to Lambda");
  }
}
