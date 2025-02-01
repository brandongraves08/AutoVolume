#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

// Global variables
unsigned long lastVolumeCheck = 0;
bool volumeReduced = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  
  // Configure ADC
  analogReadResolution(12);  // Set ADC resolution to 12 bits
  analogSetAttenuation(ADC_11db);  // Set ADC attenuation for 3.3V full-scale range
}

int getSoundLevel() {
  unsigned long startMillis = millis();
  unsigned int soundMax = 0;
  
  // Sample sound levels over the defined window
  while (millis() - startMillis < SAMPLE_WINDOW) {
    unsigned int sample = analogRead(SOUND_SENSOR_PIN);
    if (sample > soundMax) {
      soundMax = sample;
    }
  }
  
  return soundMax;
}

void adjustTVVolume(bool reduce) {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  HTTPClient http;
  
  // Create JSON payload
  StaticJsonDocument<200> doc;
  doc["entity_id"] = TV_ENTITY_ID;
  
  if (reduce) {
    doc["service"] = "volume_down";
  } else {
    doc["service"] = "volume_up";
  }
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send request to Home Assistant
  http.begin(String(HA_URL) + "/api/services/media_player/" + (reduce ? "volume_down" : "volume_up"));
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(HA_TOKEN));
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    Serial.printf("Volume %s: Success\n", reduce ? "decreased" : "increased");
  } else {
    Serial.printf("Error adjusting volume: %d\n", httpResponseCode);
  }
  
  http.end();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check sound level periodically
  if (currentMillis - lastVolumeCheck >= VOLUME_CHECK_INTERVAL) {
    lastVolumeCheck = currentMillis;
    
    int soundLevel = getSoundLevel();
    Serial.printf("Sound level: %d\n", soundLevel);
    
    // If sound is above threshold and volume hasn't been reduced
    if (soundLevel > SOUND_THRESHOLD && !volumeReduced) {
      adjustTVVolume(true);  // Reduce volume
      volumeReduced = true;
    }
    // If sound is below threshold and volume was previously reduced
    else if (soundLevel <= SOUND_THRESHOLD && volumeReduced) {
      adjustTVVolume(false);  // Increase volume
      volumeReduced = false;
    }
  }
}
