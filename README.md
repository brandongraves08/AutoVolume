# Auto Volume Control with ESP32 and Home Assistant

This project uses an ESP32 microcontroller with a microphone module to monitor ambient sound levels and automatically adjust TV volume through Home Assistant integration.

## Dependencies

### Python Packages
```bash
pip install -r requirements.txt
```

### Arduino Libraries
1. PubSubClient (for MQTT communication)
   - Open Arduino IDE
   - Go to Tools > Manage Libraries
   - Search for "PubSubClient"
   - Click Install

   OR using PlatformIO:
   ```ini
   # platformio.ini
   lib_deps =
     knolleary/PubSubClient @ ^2.8
   ```

## Hardware Requirements

- ESP32 Development Board
- Sound Detection Module (e.g., MAX9814 or similar)
- Jumper wires
- USB cable for programming

## Pin Connections

1. Microphone Module to ESP32:
   - VCC → 3.3V
   - GND → GND
   - OUT → GPIO36 (ADC1_CH0)

## Setup Instructions

1. Install required dependencies:
   ```
   pip install -r requirements.txt
   ```

2. Configure your Home Assistant credentials in `config.h`
3. Flash the ESP32 with the provided code
4. Add the automation to your Home Assistant configuration

## Features

- Real-time sound level monitoring
- Configurable threshold for volume reduction
- Integration with Home Assistant for TV control
- Automatic volume adjustment based on ambient noise

## Home Assistant Integration

### Requirements
- Home Assistant instance with Mosquitto MQTT broker add-on
- Long-lived access token created in Home Assistant

### Configuration Steps
1. Update MQTT credentials in `config.h`:
```cpp
#define MQTT_SERVER "your_mqtt_broker_ip"
#define MQTT_USER "your_username"
#define MQTT_PASSWORD "your_password"
```

2. Flash the ESP32:
```bash
platformio run --target upload
```

3. Add MQTT sensor to your Home Assistant `configuration.yaml`:
```yaml
mqtt:
  sensor:
    - name: "Ambient Sound Level"
      state_topic: "home/autovolume/sound_level"
      unit_of_measurement: "dB"
      expire_after: 300
```

4. Create dashboard in Lovelace UI:
```yaml
type: vertical-stack
cards:
  - type: gauge
    entity: sensor.ambient_sound_level
    name: Live Sound Level
    min: 0
    max: 4095
    severity:
      green: 0
      yellow: 2000
      red: 3500
  - type: history-graph
    entities:
      - entity: sensor.ambient_sound_level
    hours_to_show: 24
    refresh_interval: 60
```

### Troubleshooting
- **No Data**: Check MQTT connection status in ESP32 serial monitor
- **Stale Data**: Verify `expire_after` matches in code and config
- **Incorrect Values**: Calibrate sound sensor thresholds in `config.h`
