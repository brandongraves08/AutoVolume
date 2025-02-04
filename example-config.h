#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Home Assistant Configuration
#define HA_URL "http://homeassistant.local:8123"
#define HA_TOKEN "your_long_lived_access_token"

// MQTT Configuration
#define MQTT_SERVER "192.168.1.100"  // Your MQTT broker IP
#define MQTT_PORT 1883
#define MQTT_USER "your_mqtt_username"
#define MQTT_PASSWORD "your_mqtt_password"
#define MQTT_TOPIC "home/autovolume/control"

// Sound Detection Configuration
#define SOUND_PIN 34  // ADC pin for sound sensor
#define SOUND_THRESHOLD 2000  // Threshold for loud sound detection (adjust as needed)
#define VOLUME_CHECK_INTERVAL 1000  // Check volume every 1 second
#define SOUND_REPORT_INTERVAL 5000  // Report sound level every 5 seconds

// Home Assistant Auto-Discovery Configuration
#define HA_AUTO_DISCOVERY_TOPIC "homeassistant/sensor/autovolume/config"

// TV Entity Configuration
#define TV_ENTITY_ID "media_player.your_tv_entity_id"

#endif
