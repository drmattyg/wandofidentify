#include <Preferences.h>

Preferences preferences;

void setup() {
    Serial.begin(115200);

    // Open Preferences with namespace "my-app"
    preferences.begin("my-app", false); // false = read/write mode

    // Uncomment this line ONCE to store the API key permanently
    preferences.putString("api_key", "your-secret-api-key");

    // Retrieve the API key
    String apiKey = preferences.getString("api_key", "no-key-found");

    Serial.println("Stored API Key: " + apiKey);

    // Close Preferences
    preferences.end();
}

void loop() {
    // Do nothing
}