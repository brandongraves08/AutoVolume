# AutoVolume

An ESP32-based ambient sound monitoring system that integrates with Home Assistant to automatically adjust TV volume based on room noise levels.

## Features

- Real-time sound level monitoring (25-100 dB range)
- MQTT integration with Home Assistant
- Automatic TV volume adjustment based on ambient noise
- Historical data tracking and visualization
- Customizable sound thresholds and responses
- Noise filtering and smoothing algorithms

## Sound Level Scale

The system measures sound levels in decibels (dB) with the following reference scale:
- **25-30 dB**: Very quiet room 🤫
- **30-40 dB**: Quiet whisper 📝
- **40-50 dB**: Quiet office 💻
- **50-60 dB**: Normal conversation 💬
- **60-70 dB**: Loud conversation 🗣️
- **70-80 dB**: Vacuum cleaner 🧹
- **80-90 dB**: Heavy traffic 🚗
- **90-100 dB**: Power tools ⚡

## Hardware Requirements

- ESP32 development board
- Microphone module (e.g., MAX9814 or similar)
- Power supply for ESP32

## Software Requirements

- Arduino IDE
- Home Assistant
- MQTT broker (can be the one built into Home Assistant)

## Libraries Required

- PubSubClient
- ArduinoJson
- WiFi

## Installation

1. **ESP32 Setup**
   - Connect the microphone to the ESP32:
     - VCC to 3.3V
     - GND to GND
     - OUT to GPIO36 (or your chosen analog input pin)
   - Install required libraries in Arduino IDE
   - Copy `example-config.h` to `config.h` and update with your settings
   - Flash the ESP32 with the code

2. **Home Assistant Configuration**
   Add to your `configuration.yaml`:
   ```yaml
   # MQTT Configuration
   mqtt:
     sensor:
       - name: "Room Sound Level"
         state_topic: "home/sound/level"
         unit_of_measurement: "dB"
         value_template: "{{ value_json.level }}"
         device_class: "sound_pressure"
         suggested_display_precision: 1
       
       - name: "Room Sound Peak"
         state_topic: "home/sound/level"
         unit_of_measurement: "dB"
         value_template: "{{ value_json.peak }}"
         device_class: "sound_pressure"
         suggested_display_precision: 1
       
       - name: "Room Sound Average"
         state_topic: "home/sound/level"
         unit_of_measurement: "dB"
         value_template: "{{ value_json.average }}"
         device_class: "sound_pressure"
         suggested_display_precision: 1

   # Enable history
   history:
     include:
       domains:
         - sensor
       entities:
         - sensor.room_sound_level
         - sensor.room_sound_peak
         - sensor.room_sound_average

   # Recording configuration
   recorder:
     purge_keep_days: 30
     commit_interval: 30
     include:
       entities:
         - sensor.room_sound_level
         - sensor.room_sound_peak
         - sensor.room_sound_average
   ```

3. **Dashboard Configuration**
   Create `/config/dashboards/dashboard.yaml`:
   ```yaml
   views:
     - title: Sound Monitor
       cards:
         - type: entities
           title: Sound Levels
           entities:
             - entity: sensor.room_sound_level
             - entity: sensor.room_sound_peak
             - entity: sensor.room_sound_average

         - type: gauge
           title: Current Sound Level
           entity: sensor.room_sound_level
           min: 25
           max: 100
           severity:
             green: 0
             yellow: 60
             red: 75
           name: Sound Level (dB)

         - type: history-graph
           title: Sound History
           hours_to_show: 24
           entities:
             - entity: sensor.room_sound_level
             - entity: sensor.room_sound_peak
             - entity: sensor.room_sound_average

         - type: markdown
           title: Sound Level Reference
           content: |
             ### Sound Level Scale (dB)
             - **25-30 dB**: Very quiet room 🤫
             - **30-40 dB**: Quiet whisper 📝
             - **40-50 dB**: Quiet office 💻
             - **50-60 dB**: Normal conversation 💬
             - **60-70 dB**: Loud conversation 🗣️
             - **70-80 dB**: Vacuum cleaner 🧹
             - **80-90 dB**: Heavy traffic 🚗
             - **90-100 dB**: Power tools ⚡
   ```

4. **Automations**
   Add to your `automations.yaml`:
   ```yaml
   - alias: "AutoVolume TV Control - High Noise Volume Reduction"
     description: "Decrease TV volume when ambient noise is too high"
     trigger:
       platform: numeric_state
       entity_id: sensor.room_sound_level
       above: 70
       for:
         seconds: 5
     condition:
       - condition: state
         entity_id: switch.autovolume_enable
         state: 'on'
       - condition: template
         value_template: >
           {{ states('media_player.living_room_tv').state == 'playing' }}
     action:
       - service: media_player.volume_down
         target:
           entity_id: media_player.living_room_tv
       - delay:
           seconds: 2
   ```

## Configuration

### ESP32 (`config.h`)
```cpp
// WiFi Settings
#define WIFI_SSID "your_ssid"
#define WIFI_PASSWORD "your_password"

// MQTT Settings
#define MQTT_SERVER "homeassistant.local"
#define MQTT_PORT 1883
#define MQTT_USER "your_mqtt_user"
#define MQTT_PASSWORD "your_mqtt_password"

// Sound Monitoring Settings
#define SOUND_SENSOR_PIN 36  // ADC pin connected to microphone
#define SAMPLE_WINDOW 50     // Sample window in milliseconds
```

## Features in Detail

### Sound Monitoring
- Continuous sampling with noise filtering
- Running average calculation with smoothing
- Peak detection with 5-minute auto-reset
- Automatic ADC calibration

### Volume Control Logic
- Triggers volume reduction above 70 dB
- Responds to sudden noise spikes (15 dB above average)
- 2-second delay between adjustments
- Automatic re-enabling when noise levels normalize

### Data Storage
- 30-day history retention
- 30-second database commit interval
- Full history graphs and statistics
- Daily, weekly, and monthly trends

## Troubleshooting

1. **No MQTT Data**
   - Check WiFi connection
   - Verify MQTT broker settings
   - Check MQTT topics in Home Assistant

2. **Inaccurate Sound Levels**
   - Verify microphone connections
   - Check SOUND_SENSOR_PIN configuration
   - Ensure proper power supply to microphone
   - Consider environmental noise factors

3. **Home Assistant Integration Issues**
   - Verify MQTT integration is enabled
   - Check entity names in automations
   - Restart Home Assistant after configuration changes
   - Verify dashboard configuration

## Contributing

Feel free to submit issues and pull requests.

## License

MIT License - see LICENSE file for details