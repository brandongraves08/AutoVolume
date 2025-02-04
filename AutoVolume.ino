#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "config.h"

// Sound metrics structure
struct SoundMetrics {
  int current;           // Current reading
  int peak;             // Peak level in the sampling window
  float average;        // Running average
  int samples;          // Number of samples taken
  float variance;       // Sound variance (for detecting sudden changes)
  unsigned long lastPeakTime;  // When the last peak occurred
};

// Function prototypes
void mqttCallback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT();
int getSoundLevel();
void adjustTVVolume(bool reduce);
void updateSoundMetrics(int newReading);
void publishMetrics();

// Global variables
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastVolumeCheck = 0;
bool volumeReduced = false;

// Metrics variables
SoundMetrics metrics = {0, 0, 0, 0, 0, 0};
const int SAMPLES_WINDOW = 100;  // Number of samples to consider for metrics
float samples[SAMPLES_WINDOW];   // Circular buffer for samples
int sampleIndex = 0;

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

void updateSoundMetrics(int newReading) {
  // Update current reading
  metrics.current = newReading;
  
  // Update peak
  if (newReading > metrics.peak) {
    metrics.peak = newReading;
    metrics.lastPeakTime = millis();
  }
  
  // Reset peak if it's older than 5 seconds
  if (millis() - metrics.lastPeakTime > 5000) {
    metrics.peak = newReading;
    metrics.lastPeakTime = millis();
  }
  
  // Update samples buffer
  samples[sampleIndex] = newReading;
  sampleIndex = (sampleIndex + 1) % SAMPLES_WINDOW;
  
  if (metrics.samples < SAMPLES_WINDOW) {
    metrics.samples++;
  }
  
  // Calculate running average
  float sum = 0;
  for (int i = 0; i < metrics.samples; i++) {
    sum += samples[i];
  }
  metrics.average = sum / metrics.samples;
  
  // Calculate variance
  float sumSquareDiff = 0;
  for (int i = 0; i < metrics.samples; i++) {
    float diff = samples[i] - metrics.average;
    sumSquareDiff += diff * diff;
  }
  metrics.variance = sumSquareDiff / metrics.samples;
}

void publishMetrics() {
  StaticJsonDocument<256> doc;
  doc["current"] = metrics.current;
  doc["peak"] = metrics.peak;
  doc["average"] = metrics.average;
  doc["variance"] = metrics.variance;
  
  char buffer[256];
  serializeJson(doc, buffer);
  client.publish("home/autovolume/metrics", buffer);
  
  // Also publish individual metrics for easier HA integration
  client.publish("home/autovolume/sound_level", String(metrics.current).c_str());
  client.publish("home/autovolume/sound_peak", String(metrics.peak).c_str());
  client.publish("home/autovolume/sound_average", String(metrics.average).c_str());
  client.publish("home/autovolume/sound_variance", String(metrics.variance).c_str());
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
    updateSoundMetrics(soundLevel);
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
    publishMetrics();
    lastSoundReport = currentMillis;
  }
}
