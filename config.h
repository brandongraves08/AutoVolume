#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Home Assistant Configuration
#define HA_URL "http://your_home_assistant_ip:8123"
#define HA_TOKEN "your_long_lived_access_token"

// MQTT Configuration
#define MQTT_SERVER "your_mqtt_broker_ip"
#define MQTT_PORT 1883
#define MQTT_USER "mqtt_username"
#define MQTT_PASSWORD "mqtt_password"
#define MQTT_TOPIC "home/autovolume/control"

// Sound Detection Configuration
#define SOUND_SENSOR_PIN 36  // ADC1_CH0
#define SOUND_THRESHOLD 2000 // Adjust this value based on your environment
#define SAMPLE_WINDOW 50     // Sample window in milliseconds
#define VOLUME_CHECK_INTERVAL 1000 // Check volume every 1 second

// Reporting Configuration
#define SOUND_REPORT_INTERVAL 2000
#define HA_AUTO_DISCOVERY_TOPIC "homeassistant/sensor/autovolume/config"

// TV Entity Configuration
#define TV_ENTITY_ID "media_player.your_tv_entity_id"

#endif
