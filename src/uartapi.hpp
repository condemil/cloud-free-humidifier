#pragma once

#include <Arduino.h>

#include "logger.hpp"
#include "ntp.hpp"
#include "state.hpp"

namespace uartapi {
char curr_char;
char buf[256];
int buf_idx;
static HardwareSerial &out = Serial;

std::function<void(uint8_t)> _onPower;
std::function<void(PresetMode)> _onPresetMode;
std::function<void(float)> _onTemperature;
std::function<void(uint8_t)> _onHumidity;
std::function<void(uint8_t)> _onDepth;

void setup(std::function<void(bool)> onPower, std::function<void(PresetMode)> onPresetMode, std::function<void(float)> onTemperature,
    std::function<void(uint8_t)> onHumidity, std::function<void(uint8_t)> onDepth) {
    _onPower = onPower;
    _onPresetMode = onPresetMode;
    _onTemperature = onTemperature;
    _onHumidity = onHumidity;
    _onDepth = onDepth;

    out.begin(115200);
    out.swap();
}

void respond_ok() {
    out.print("ok\r");
}

void send_down_response() {
    if (state::preset_mode_updated) {
        switch (state::preset_mode) {
        case PresetMode::auto_:
            out.print("down set_mode \"auto\"\r");
            break;
        case PresetMode::high:
            out.print("down set_mode \"high\"\r");
            break;
        case PresetMode::medium:
            out.print("down set_mode \"medium\"\r");
            break;
        case PresetMode::silent:
            out.print("down set_mode \"silent\"\r");
            break;
        }

        state::preset_mode_updated = false;
        return;
    }

    if (state::power_updated) {
        state::power ? out.print("down set_power \"on\"\r") : out.print("down set_power \"off\"\r");
        state::power_updated = false;
        return;
    }

    out.print("down none\r");
}

void process_props(char *key, char *value) {
    if (key[0] != 'h' && key[0] != 'u') { // humidity, use_time
        logger::debugf("uartapi: received props: %s %s\n", key, value);
    }

    if (strcmp(key, "mode") == 0) {
        if(strcmp(value, "\"auto\"") == 0) {
            _onPresetMode(PresetMode::auto_);
        }

        if(strcmp(value, "\"high\"") == 0) {
            _onPresetMode(PresetMode::high);
        }

        if(strcmp(value, "\"medium\"") == 0) {
            _onPresetMode(PresetMode::medium);
        }

        if(strcmp(value, "\"silent\"") == 0) {
            _onPresetMode(PresetMode::silent);
        }
    } else if (strcmp(key, "power") == 0) {
        if(strcmp(value, "\"off\"") == 0) {
            _onPower(false);
        }

        if(strcmp(value, "\"on\"") == 0) {
            _onPower(true);
        }
    } else if (strcmp(key, "temp_dec") == 0) {
        _onTemperature(atof(value) / 10);
    } else if (strcmp(key, "humidity") == 0) {
        _onHumidity(atoi(value));
    } else if (strcmp(key, "depth") == 0) {
        _onDepth(atoi(value));
    } else if (strcmp(key, "dry") == 0) {
        // not implemented
    } else if (strcmp(key, "child_lock") == 0) {
        // not implemented
    } else if (strcmp(key, "buzzer") == 0) {
        // not implemented
    } else if (strcmp(key, "buzzer") == 0) {
        // not implemented
    } else if (strcmp(key, "use_time") == 0) {
        // not implemented
    } else if (strcmp(key, "button_pressed") == 0) {
        // not implemented
    } else {
        logger::errorf("uartapi: unknown props key: %s value: %s\n", key, value);
    }

}

void process_command() {
    char *token = strtok(buf, "\r\xFE ");

    if (strcmp(token, "") == 0) {
        // after esp boot 0xFE character is received from uart
        // as soon as split happens based on 0xFE update token to the next word
        token = strtok(NULL, " ");
    } else if (strcmp(token, "get_down") == 0) {
        send_down_response();
    } else if (strcmp(token, "net") == 0) {
        out.print("local\r");
    } else if (strcmp(token, "time") == 0) {
        out.print(ntp::get_time());
        out.print("\r");
    } else if (strcmp(token, "props") == 0) {
        while (1) {
            char *key = strtok(NULL, " ");
            char *value = strtok(NULL, " ");

            if (key == NULL || value == NULL) {
                break;
            }

            process_props(key, value);
        }
        respond_ok();
    } else if (strcmp(token, "event") == 0) {
        // TODO: process event
        char *event = strtok(NULL, "\r");
        logger::debugf("uartapi: received event: %s\n", event);
        respond_ok();
    } else if (strcmp(token, "model") == 0) {
        respond_ok();
    } else if (strcmp(token, "ble_config") == 0) {
        respond_ok();
    } else if (strcmp(token, "mcu_version") == 0) {
        respond_ok();
    } else if (strcmp(token, "result") == 0) {
        respond_ok();
    } else if (strcmp(token, "error") == 0) {
        logger::errorf("uartapi: %s\n", buf);
        respond_ok();
    } else if (strcmp(token, "restore") == 0) {
        // config::truncate();
        respond_ok();
    } else if (strcmp(token, "reboot") == 0) {
        respond_ok();
        ESP.restart();
    } else {
        logger::errorf("uartapi: unknown command: %s\n", token);
    }
}

void handle() {
    while (out.available() > 0) {
        curr_char = out.read();

        if (curr_char == '\r') {
            buf[buf_idx] = 0;
            buf_idx = 0;
            process_command();
            break;
        }

        buf[buf_idx] = curr_char;
        buf_idx++;
    }
}
} // namespace uartapi
