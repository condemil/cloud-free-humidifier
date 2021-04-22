# ESP8266 firmware for zhimi.humidifier.ca1 humidifier

This is a replacement for mt7697 module with esp8266 to make cloud-free humidifier.

## MQTT protocol

The MQTT protocol has json format and works both ways to send data to humidifier and receive current status.

To humidifier topic `esp8266/XXXXXX/set`:

- state - controls power, can be `ON` or `OFF`
- preset_mode - controls preset mode, one of `silent`, `medium`, `high`, `auto`
- command - `restart` to restart esp8266 and `reset` to remove the configuration and restart

```json
{
  "state": "ON",
  "preset_mode": "high"
}
```

From humidifier topic `esp8266/XXXX`:

```json
{
  "state": "ON",
  "temperature": 24.3,
  "humidity": 48,
  "preset_mode": "silent",
  "depth": 21
}
```

## Example home-assistant configuration

```yaml
fan:
  - platform: mqtt
    name: Humidifier
    state_topic: esp8266/XXXXXX
    state_value_template: "{{ value_json.state }}"
    json_attributes_topic: esp8266/XXXXXX
    availability_topic: esp8266/XXXXXX/availability
    command_topic: esp8267/XXXXXX/set
    command_template: "{ state: '{{ value }}'}"
    preset_mode_command_topic: esp8266/XXXXXX/set
    preset_mode_command_template: "{ preset_mode: '{{ value }}'}"
    preset_mode_value_template: "{{ value_json.preset_mode }}"
    preset_modes:
      - "auto"
      - "high"
      - "medium"
      - "silent"
```

## File list

- src/ - esp8266 firmware
- tools/get-tz.py - generate timezone list for esp8266 firmware
- protocol.txt - uart protocol between mt7697 and stm32
