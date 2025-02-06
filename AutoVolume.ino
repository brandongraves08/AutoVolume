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
void updateSoundMetrics(int newReading);
void publishMetrics();

// Global variables
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastSoundCheck = 0;
int currentSoundLevel = 0;
int peakSoundLevel = 0;
int averageSoundLevel = -1;

void setup() {
  Serial.begin(115200);
  
  // Configure ADC
  analogReadResolution(12);  // Set ADC resolution to 12 bits (0-4095)
  analogSetAttenuation(ADC_11db);  // Set ADC attenuation for higher voltage range
  
  // Configure microphone pin
  pinMode(SOUND_SENSOR_PIN, INPUT);
  
  // Initialize WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  
  // MQTT Configuration
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(mqttCallback);
  
  reconnectMQTT();
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - lastSoundCheck >= VOLUME_CHECK_INTERVAL) {
    lastSoundCheck = currentMillis;
    
    // Get sound readings and update metrics
    int soundLevel = getSoundLevel();
    updateSoundMetrics(soundLevel);

    // Prepare and send MQTT message with scaled values
    StaticJsonDocument<200> doc;
    doc["level"] = currentSoundLevel;    // Already in dB scale (25-100)
    doc["peak"] = peakSoundLevel;        // Already in dB scale (25-100)
    doc["average"] = averageSoundLevel;  // Now a whole number
    
    char buffer[200];
    serializeJson(doc, buffer);
    client.publish("home/sound/level", buffer);
    
    // Debug output with whole numbers
    Serial.printf("Sound Metrics - Current: %d, Peak: %d, Average: %d\n", 
                 currentSoundLevel, peakSoundLevel, averageSoundLevel);
  }
}

int getSoundLevel() {
  unsigned long startMillis = millis();
  unsigned long soundSum = 0;
  unsigned int sampleCount = 0;
  unsigned int soundMax = 0;
  const int NOISE_THRESHOLD = 50;  // Lowered threshold for quieter sounds
  
  // Sample sound levels over the defined window
  while (millis() - startMillis < SAMPLE_WINDOW) {
    unsigned int sample = analogRead(SOUND_SENSOR_PIN);
    
    // Include all readings above noise floor
    if (sample > NOISE_THRESHOLD) {
      soundSum += sample;
      sampleCount++;
      if (sample > soundMax) {
        soundMax = sample;
      }
    }
    
    // Small delay to stabilize ADC
    delayMicroseconds(100);
  }
  
  // Calculate average
  unsigned int soundAvg = (sampleCount > 0) ? (soundSum / sampleCount) : 0;
  
  // Print debug info
  Serial.print("Raw MAX value: ");
  Serial.println(soundMax);
  Serial.print("Raw AVG value: ");
  Serial.println(soundAvg);
  Serial.print("Samples counted: ");
  Serial.println(sampleCount);
  
  // Use average instead of max for more stable readings
  unsigned int soundValue = soundAvg;
  
  // Handle very quiet environments
  if (soundValue < NOISE_THRESHOLD) {
    soundValue = NOISE_THRESHOLD;
  }
  
  // Convert to dB scale (adjusted for better range)
  // 25 dB is very quiet room, 100 dB is very loud
  float dB = 25 + (20 * log10((float)soundValue / NOISE_THRESHOLD));
  
  // Print calculated values
  Serial.print("Calculated dB: ");
  Serial.println(dB);
  
  // Constrain to reasonable room values
  int finalDB = constrain((int)dB, 25, 100);
  
  Serial.print("Final dB: ");
  Serial.println(finalDB);
  Serial.println();
  
  return finalDB;
}

void updateSoundMetrics(int newReading) {
  // Update current level (already in dB scale)
  currentSoundLevel = newReading;
  
  // Update peak level
  if (newReading > peakSoundLevel) {
    peakSoundLevel = newReading;
  }
  
  // Reset peak if it's been a while (5 minutes)
  static unsigned long lastPeakReset = 0;
  if (millis() - lastPeakReset > 300000) {  // 5 minutes
    peakSoundLevel = newReading;
    lastPeakReset = millis();
  }
  
  // Update running average with smoothing (now as whole number)
  const float alpha = 0.1; // Smoothing factor (0.1 = 10% weight to new readings)
  if (averageSoundLevel < 0) {
    averageSoundLevel = newReading;
  } else {
    // Calculate new average and round to nearest integer
    float newAverage = (alpha * newReading) + ((1 - alpha) * averageSoundLevel);
    averageSoundLevel = round(newAverage);
  }
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
