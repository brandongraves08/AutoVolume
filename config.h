#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "Mine!"
#define WIFI_PASSWORD "welcomehome"

// Home Assistant Configuration
#define HA_URL "http://homeassistant.local:8123"
#define HA_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJjNDcyMmQwZTRjYzI0MzI0ODI1ZmNiMDVmMGRhMGE2NCIsImlhdCI6MTczODYyMTk2OCwiZXhwIjoyMDUzOTgxOTY4fQ.Qw9eMxyGhJrGFilOim9G_Z27wuUAi23g8wyrdV1sJzU"

// MQTT Configuration
#define MQTT_SERVER "192.168.0.188"
#define MQTT_PORT 1883
#define MQTT_USER "autosound"
#define MQTT_PASSWORD "1Workhard!"
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
#define TV_ENTITY_ID "media_player.living_room_tv"

#endif
