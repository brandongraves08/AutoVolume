# AutoVolume

An ESP32-based sound monitoring system that automatically adjusts your TV volume through Home Assistant when ambient noise levels change. Features sophisticated noise analysis and smart volume control.

## Features

- Real-time sound level monitoring with advanced metrics
- Smart volume control based on multiple sound parameters
- Adaptive thresholds based on time of day
- MQTT integration with Home Assistant auto-discovery
- Configurable thresholds and timing intervals
- Web-based configuration and monitoring
- Automatic pause during known events

## Hardware Requirements

- ESP32 development board
- Sound sensor module (analog output)
- Power supply for ESP32
- Jumper wires

## Software Requirements

- Arduino IDE or PlatformIO
- Home Assistant instance
- MQTT broker (can be the one integrated with Home Assistant)
- Required Arduino libraries:
  - PubSubClient
  - ArduinoJson

## Installation

1. Clone this repository or download the source code
2. Install required libraries through Arduino IDE Library Manager or PlatformIO
3. Copy `example-config.h` to `config.h` and update the configuration
4. Flash the code to your ESP32

## Configuration

Copy `example-config.h` to `config.h` and update the following settings:

```cpp
// WiFi settings
WIFI_SSID - Your WiFi network name
WIFI_PASSWORD - Your WiFi password

// Home Assistant settings
HA_URL - Your Home Assistant URL (e.g., "http://homeassistant.local:8123")
HA_TOKEN - Your long-lived access token from Home Assistant

// MQTT settings
MQTT_SERVER - IP address of your MQTT broker
MQTT_PORT - MQTT broker port (default: 1883)
MQTT_USER - MQTT username
MQTT_PASSWORD - MQTT password

// Device settings
SOUND_PIN - GPIO pin connected to sound sensor (default: 34)
SOUND_THRESHOLD - Sound level threshold for volume adjustment
TV_ENTITY_ID - Your TV's entity ID in Home Assistant
```

## Sound Metrics

The system tracks multiple sound metrics for intelligent volume control:

### Real-time Metrics
- **Current Level**: Instantaneous sound reading
- **Peak Level**: Highest sound level in a 5-second window
- **Running Average**: Smooth average over 100 samples
- **Variance**: Measure of sound level stability

### Derived Metrics
- **Sound Stability**: Percentage indicating how stable the sound environment is
- **Change Rate**: How quickly sound levels are changing
- **24-hour Statistics**: Long-term averages for baseline calculation

## Home Assistant Integration

### Required Configuration

Add these sections to your Home Assistant configuration:

1. **Configuration.yaml**:
```yaml
# Sound Level Statistics
sensor:
  - platform: statistics
    name: "Sound Level 1h Stats"
    entity_id: sensor.ambient_sound_level
    sampling_size: 720
    max_age:
      hours: 1

  - platform: statistics
    name: "Sound Level 24h Stats"
    entity_id: sensor.ambient_sound_level
    sampling_size: 17280
    max_age:
      hours: 24

# Helper entities
input_boolean:
  volume_control_paused:
    name: Volume Control Pause
    icon: mdi:volume-off

input_number:
  volume_reduction_level:
    name: Volume Reduction Level
    min: 0
    max: 5
    step: 1
```

2. **Automations.yaml**:
The system includes several smart automations:

- **Gradual Response**: Slowly adjusts volume for sustained noise
- **Sudden Noise Response**: Quick response to sudden loud sounds
- **Volume Recovery**: Gradually restores volume when noise normalizes
- **Adaptive Threshold**: Adjusts sensitivity based on time of day
- **Auto-Pause**: Temporarily disables during specific events

See the full automation examples in `automations.yaml`.

### Dashboard

A comprehensive Lovelace dashboard is provided showing:
- Current sound levels and thresholds
- Historical graphs and statistics
- System status and controls
- Sound environment stability

## Smart Volume Control Features

### 1. Adaptive Response
- Distinguishes between sudden and gradual noise changes
- Different response patterns for different noise types
- Considers sound stability in decisions

### 2. Time-Based Adaptation
- More sensitive during quiet hours (22:00 - 06:00)
- Less sensitive during typical active hours
- Customizable threshold adjustments

### 3. Intelligent Recovery
- Gradual volume restoration
- Considers environment stability
- Prevents oscillating adjustments

### 4. Auto-Pause Triggers
- Media changes
- Door openings (if sensors available)
- Other configurable events

## Troubleshooting

1. **No WiFi Connection**
   - Check SSID and password in config.h
   - Ensure ESP32 is within range of WiFi

2. **MQTT Connection Issues**
   - Verify MQTT broker IP and credentials
   - Check if MQTT broker is running
   - Ensure broker is accessible from ESP32's network

3. **Sound Detection Issues**
   - Check sensor wiring
   - Adjust SOUND_THRESHOLD in config.h
   - Monitor serial output for actual readings

4. **Volume Control Not Working**
   - Verify TV_ENTITY_ID matches your Home Assistant entity
   - Check Home Assistant API token permissions
   - Ensure TV is available and controllable in Home Assistant

5. **Metrics Issues**
   - Check MQTT topics in Home Assistant
   - Verify sensor statistics configuration
   - Monitor system stability percentage

## Contributing

Feel free to submit issues and pull requests.

## License

MIT License - feel free to use and modify as you wish.
