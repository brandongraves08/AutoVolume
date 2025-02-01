# Auto Volume Control with ESP32 and Home Assistant

This project uses an ESP32 microcontroller with a microphone module to monitor ambient sound levels and automatically adjust TV volume through Home Assistant integration.

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
