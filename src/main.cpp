#include <Arduino.h>
#include <SoftwareSerial.h>
#include <math.h>

#include "config.hpp"
#include "logger.hpp"
#include "mqtt.hpp"
#include "ntp.hpp"
#include "ota.hpp"
#include "state.hpp"
#include "uartapi.hpp"
#include "wifi.hpp"

void onMQTTCommand(uint8_t);
void onMQTTPower(bool);
void onUARTPower(bool);
void onMQTTPresetMode(PresetMode);
void onUARTPresetMode(PresetMode);
void onUARTTemperature(float);
void onUARTHumidity(uint8_t);
void onUARTDepth(uint8_t);

helpers::elapsedMillis _update_mqtt_temperature_elapsed;
helpers::elapsedMillis _update_mqtt_humidity_elapsed;
helpers::elapsedMillis _update_mqtt_depth_elapsed;
const unsigned int UPDATE_SENSORS_MQTT_DELAY = 60000; // milliseconds

void setup() {
    logger::setup();
    logger::debugln(F("\nmain: started"));

    uartapi::setup(onUARTPower, onUARTPresetMode, onUARTTemperature, onUARTHumidity, onUARTDepth);
    config::setup();
    wifi::setup();
    if (strlen(config::conf.syslog_host) != 0) {
        logger::setupSyslog(config::conf.syslog_host, config::conf.syslog_port, config::HOSTNAME, config::NAME);
    }
    mqtt::setup(onMQTTCommand, onMQTTPower, onMQTTPresetMode);
    ota::setup();
    ntp::setup();

    logger::debugln(F("main: setup is over"));
}

void loop() {
    uartapi::handle();
    wifi::handle();
    mqtt::handle();
    ota::handle();
}

void onMQTTCommand(uint8_t command) {
    switch (command) {
    case mqtt::COMMAND_RESET:
        config::truncate();
        ESP.restart();
        break;
    case mqtt::COMMAND_RESTART:
        ESP.restart();
        break;
    }
}

void onMQTTPower(bool power) {
    if (state::power == power) {
        // out of sync with remote
        // send back same value without setting it
        mqtt::publish();
        return;
    }

    state::power = power;
    state::power_updated = true;
}

void onUARTPower(bool power) {
    state::power = power;
    mqtt::publish();
}

void onMQTTPresetMode(PresetMode preset_mode) {
    if (state::preset_mode == preset_mode) {
        // out of sync with remote
        // send back same value without setting it
        mqtt::publish();
        return;
    }

    state::preset_mode = preset_mode;
    state::preset_mode_updated = true;
}

void onUARTPresetMode(PresetMode preset_mode) {
    state::preset_mode = preset_mode;
    mqtt::publish();
}

void onUARTTemperature(float temperature) {
    if (_update_mqtt_temperature_elapsed < UPDATE_SENSORS_MQTT_DELAY) {
        return;
    }

    if (state::temperature == temperature) {
        return;
    }

    state::temperature = temperature;
    mqtt::publish();
    _update_mqtt_temperature_elapsed = 0;
}

void onUARTHumidity(uint8_t humidity) {
    if (_update_mqtt_humidity_elapsed < UPDATE_SENSORS_MQTT_DELAY) {
        return;
    }

    if (state::humidity == humidity) {
        return;
    }

    state::humidity = humidity;
    mqtt::publish();
    _update_mqtt_humidity_elapsed = 0;
}

void onUARTDepth(uint8_t depth) {
    if (depth == 127) { // unable to read depth value, ignore it
        return;
    }

    if (_update_mqtt_depth_elapsed < UPDATE_SENSORS_MQTT_DELAY) {
        return;
    }

    if (state::depth == depth) {
        return;
    }

    state::depth = depth;
    mqtt::publish();
    _update_mqtt_depth_elapsed = 0;
}
