# WIP: ESP8266 firmware for zhimi.humidifier.ca1 humidifier

This is a replacement for mt7697 module with esp8266 to make cloud-free humidifier.

File list:

- custom_components/ - home-assistant component to interact with humidifier though mqtt (wip)
- lib/ src/ - esp8266 firmware (wip)
- tools/find-cloudkey.go - search for a cloud key from the miio udp packet in the binary file (not found in firmware dump)
- tools/dump-firmware.py - dump firmare to file though uart cli
- tools/get-tz.py - generate timezone list for esp8266 firmware
- protocol.txt - uart protocol between mt7697 and stm32
