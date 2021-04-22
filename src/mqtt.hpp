#pragma once

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#include "config.hpp"
#include "helpers.hpp"
#include "logger.hpp"
#include "state.hpp"

namespace mqtt {
const uint8_t COMMAND_RESET = 0;
const uint8_t COMMAND_RESTART = 1;

const bool WILL_RETAIN = true;
const uint8_t WILL_QOS = 1;
static const char *WILL_PAYLOAD_AVAILABLE = "online";
static const char *WILL_PAYLOAD_NOT_AVAILABLE = "offline";

char MQTT_TOPIC_STATE[15];
char MQTT_TOPIC_COMMAND[19];
char MQTT_TOPIC_AVAILABILITY[28];
char publishBuffer[128];
bool connected = false;

std::function<void(uint8_t)> _onCommand;
std::function<void(bool)> _onPower;
std::function<void(PresetMode)> _onPresetMode;

WiFiClient _esp_client;
PubSubClient _client(_esp_client);
helpers::elapsedMillis _reconnect_time_elapsed;
const unsigned int RECONNECT_DELAY = 5000;

void publish() {
    StaticJsonDocument<128> doc;

    doc["state"] = state::power ? "ON" : "OFF";

    switch (state::preset_mode) {
    case PresetMode::auto_:
        doc["preset_mode"] = "auto";
        break;
    case PresetMode::high:
        doc["preset_mode"] = "high";
        break;
    case PresetMode::medium:
        doc["preset_mode"] = "medium";
        break;
    case PresetMode::silent:
        doc["preset_mode"] = "silent";
        break;
    }

    doc["temperature"] = state::temperature;
    doc["humidity"] = state::humidity;
    doc["depth"] = state::depth;

    serializeJson(doc, publishBuffer);

    _client.publish(MQTT_TOPIC_STATE, publishBuffer, true);
}

void _callback(char *topic, byte *payload, unsigned int length) {
    if (length >= MQTT_MAX_PACKET_SIZE) {
        logger::errorln(F("mqtt: payload is too long"));
        return;
    }

    logger::debugf("mqtt: message arrived [%s]\n", topic);

    StaticJsonDocument<128> doc;

    // the payload buffer is reused in PubSubClient for publish() and it breaks data in doc
    // cast to pointer-to-const to disable ArduinoJson zero-copy mode
    DeserializationError error = deserializeJson(doc, (const byte*)payload);

    if (error) {
        logger::errorln(F("mqtt: failed to deserialize payload"));
        return;
    }

    if (doc["mode"].is<char *>()) {
        if (doc["mode"] == "silent") {
            _onPresetMode(PresetMode::silent);
        } else if (doc["mode"] == "medium") {
            _onPresetMode(PresetMode::medium);
        } else if (doc["mode"] == "high") {
            _onPresetMode(PresetMode::high);
        } else if (doc["mode"] == "auto") {
            _onPresetMode(PresetMode::auto_);
        }
    }

    if (doc["state"].is<char *>()) {
        const char* state = doc["state"];

        if (doc["state"] == "ON") {
            _onPower(true);
        } else if (doc["state"] == "OFF") {
            _onPower(false);
        }
    }

    if (doc["command"].is<char *>()) {
        if (doc["command"] == "reset") {
            _onCommand(COMMAND_RESET);
        } else if (doc["command"] == "restart") {
            _onCommand(COMMAND_RESTART);
        }
    }
}

void _reconnect() {
    logger::debugln(F("mqtt: attempting to connect"));

    connected = _client.connect(config::HOSTNAME, config::conf.mqtt_login, config::conf.mqtt_pass,
        MQTT_TOPIC_AVAILABILITY, WILL_QOS, WILL_RETAIN, WILL_PAYLOAD_NOT_AVAILABLE);

    if (connected) {
        logger::debugln(F("mqtt: connected"));
        _client.subscribe(MQTT_TOPIC_COMMAND);
        logger::debugf("mqtt: subscribed to %s\n", MQTT_TOPIC_COMMAND);
        _client.publish(MQTT_TOPIC_AVAILABILITY, WILL_PAYLOAD_AVAILABLE);
    } else {
        logger::debugf(
            "mqtt: connect failed, rc=%d try again in %u seconds\n", _client.state(), RECONNECT_DELAY / 1000);
    }
}

void setup(std::function<void(uint8_t)> onCommand, std::function<void(bool)> onPower, std::function<void(PresetMode)> onPresetMode) {
    _onCommand = onCommand;
    _onPower = onPower;
    _onPresetMode = onPresetMode;

    sprintf(MQTT_TOPIC_STATE, config::MQTT_TOPIC_STATE_FORMAT, ESP.getChipId());
    sprintf(MQTT_TOPIC_COMMAND, config::MQTT_TOPIC_COMMAND_FORMAT, ESP.getChipId());
    sprintf(MQTT_TOPIC_AVAILABILITY, config::MQTT_TOPIC_AVAILABILITY_FORMAT, ESP.getChipId());
    _client.setServer(config::conf.mqtt_host, config::conf.mqtt_port);
    _client.setCallback(_callback);
}

void handle() {
    if (_client.connected()) {
        _client.loop();
        return;
    }

    if (_reconnect_time_elapsed >= RECONNECT_DELAY) {
        _reconnect_time_elapsed = 0; // reset timer
        _reconnect();
    }
}
} // namespace mqtt
