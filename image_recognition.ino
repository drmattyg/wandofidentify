#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <SHA256.h>
#include <time.h>

// AWS credentials - replace with your values
const char* AWS_ACCESS_KEY = "YOUR_ACCESS_KEY";
const char* AWS_SECRET_KEY = "YOUR_SECRET_KEY";
const char* AWS_REGION = "us-east-2";
const char* LAMBDA_FUNCTION = "recognizeImage";

WiFiClientSecure client;
SHA256 sha256;

// Function declarations
String sign_request(const String& key, const String& msg) {
  uint8_t hmacResult[32];
  sha256.resetHMAC(key.c_str(), key.length());
  sha256.update(msg.c_str(), msg.length());
  sha256.finalizeHMAC(key.c_str(), key.length(), hmacResult, sizeof(hmacResult));
  
  String signature;
  for (int i = 0; i < 32; i++) {
    char hex[3];
    sprintf(hex, "%02x", hmacResult[i]);
    signature += hex;
  }
  return signature;
}

String get_signature(const String& date_stamp, const String& region_name, const String& service_name) {
  String k_date = sign_request("AWS4" + String(AWS_SECRET_KEY), date_stamp);
  String k_region = sign_request(k_date, region_name);
  String k_service = sign_request(k_region, service_name);
  String k_signing = sign_request(k_service, "aws4_request");
  return k_signing;
}

String recognize_image(uint8_t* image_data, size_t image_size) {
  // Get current time
  configTime(0, 0, "pool.ntp.org");
  time_t now;
  time(&now);
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  
  char amz_date[17];
  char date_stamp[9];
  strftime(amz_date, sizeof(amz_date), "%Y%m%dT%H%M%SZ", &timeinfo);
  strftime(date_stamp, sizeof(date_stamp), "%Y%m%d", &timeinfo);

  // Convert image to base64
  String image_b64 = base64::encode(image_data, image_size);

  // Prepare JSON payload
  StaticJsonDocument<2048> doc;
  doc["image"] = image_b64;
  String payload;
  serializeJson(doc, payload);

  // Calculate payload hash
  sha256.reset();
  sha256.update(payload.c_str(), payload.length());
  uint8_t payload_hash_result[32];
  sha256.finalize(payload_hash_result, sizeof(payload_hash_result));
  String payload_hash = "";
  for (int i = 0; i < 32; i++) {
    char hex[3];
    sprintf(hex, "%02x", payload_hash_result[i]);
    payload_hash += hex;
  }

  // Prepare host and endpoint
  String host = "lambda." + String(AWS_REGION) + ".amazonaws.com";
  String endpoint = "https://" + host + "/2015-03-31/functions/" + String(LAMBDA_FUNCTION) + "/invocations";

  // Get signature
  String signature = get_signature(date_stamp, AWS_REGION, "lambda");

  // Prepare headers
  String credential_scope = String(date_stamp) + "/" + AWS_REGION + "/lambda/aws4_request";
  String authorization = "AWS4-HMAC-SHA256 Credential=" + String(AWS_ACCESS_KEY) + "/" + credential_scope + 
                        ", SignedHeaders=content-type;host;x-amz-date;x-amz-target, Signature=" + signature;

  // Make HTTPS request
  if (client.connect(host.c_str(), 443)) {
    client.println("POST " + endpoint + " HTTP/1.1");
    client.println("Host: " + host);
    client.println("Content-Type: application/json");
    client.println("X-Amz-Date: " + String(amz_date));
    client.println("X-Amz-Target: AWS_Lambda." + String(LAMBDA_FUNCTION));
    client.println("Authorization: " + authorization);
    client.println("Content-Length: " + String(payload.length()));
    client.println();
    client.println(payload);

    // Read response
    String response = client.readString();
    client.stop();

    // Parse response
    int bodyStart = response.indexOf("\r\n\r\n") + 4;
    String body = response.substring(bodyStart);
    
    StaticJsonDocument<512> responseDoc;
    DeserializationError error = deserializeJson(responseDoc, body);
    
    if (!error) {
      return responseDoc["label"].as<String>();
    }
    return "Error parsing response";
  }
  
  return "Connection failed";
}

void setup() {
  Serial.begin(115200);
  
  // Initialize WiFi connection here
  // WiFi.begin(ssid, password);
  
  // Configure secure client
  client.setInsecure(); // Note: In production, you should use proper certificate validation
}

void loop() {
  // Example usage with camera (you'll need to add your camera initialization and capture code)
  /*
  if (camera_capture_successful) {
    uint8_t* image_data = // ... your camera capture code ...
    size_t image_size = // ... size of captured image ...
    
    String label = recognize_image(image_data, image_size);
    Serial.println("Detected: " + label);
  }
  */
  
  delay(5000); // Wait 5 seconds between captures
} 