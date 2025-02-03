#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "config.h"

// Global variables
unsigned long lastVolumeCheck = 0;
bool volumeReduced = false;

WiFiClient espClient;
PubSubClient client(espClient);

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
  
  // MQTT Configuration
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(mqttCallback);
  
  reconnectMQTT();
  
  // HA Sensor Auto-Discovery
  String sensorConfig = R"(
  {
    "name":"Ambient Sound Level",
    "state_topic":"home/autovolume/sound_level",
    "unit_of_meas":"dB",
    "device_class":"signal_strength",
    "expire_after":300
  }
  )";
  client.publish("homeassistant/sensor/autovolume/config", sensorConfig.c_str());
  
  String discoveryPayload = R"({\n    \"name\": \"AutoVolume Sound Level\",\n    \"state_topic\": \"home/autovolume/sound_level\",\n    \"unit_of_measurement\": \"dB\",\n    \"device_class\": \"signal_strength\",\n    \"expire_after\": 300\n  })";
  client.publish(HA_AUTO_DISCOVERY_TOPIC, discoveryPayload.c_str());
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

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i=0;i<length;i++) {
    message += (char)payload[i];
  }
  
  if (String(topic) == MQTT_TOPIC) {
    int targetVolume = message.toInt();
    // Implement volume adjustment logic here
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    if (client.connect("AutoVolumeClient", MQTT_USER, MQTT_PASSWORD)) {
      client.subscribe(MQTT_TOPIC);
    } else {
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  
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
  
  static unsigned long lastSoundReport = 0;
  if (currentMillis - lastSoundReport >= SOUND_REPORT_INTERVAL) {
    int soundLevel = getSoundLevel();
    client.publish("home/autovolume/sound_level", String(soundLevel).c_str());
    lastSoundReport = currentMillis;
  }
}
