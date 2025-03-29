#include "esp_camera.h"
#include <WiFi.h>
#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
#include "camera_pins.h"
#include <HTTPClient.h>
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
  
  Serial.printf("Sending image to Lambda: %s\n", lambdaUrl);
  
  // Configure HTTP request
  http.begin(lambdaUrl);
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

void configInitCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
  }
  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }
    sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // flip it back
    s->set_brightness(s, 1);   // up the brightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

  Serial.println("Camera initialized successfully");
} 

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  Serial.setDebugOutput(true);

  Serial.println("Serial initialized");
  
  // Initialize camera before other operations
  configInitCamera();
  
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
