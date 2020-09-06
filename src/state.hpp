#pragma once

#include "command.hpp"

enum class Command : uint8_t {
    buzzer,
    child_lock,
    power,
    operation_mode,
    dry,
    led_brightness,
    limit_humiditiy,
    net_change,
};

enum class OperationMode : uint8_t {
    auto_mode,
    high,
    medium,
    silent,
};

enum class NetMode : uint8_t {
    uap, // unprovisioned, access point created
    unprov, // unprovisioned, access point disabled (after 1 hour of being uap)
    local, // wifi is properly set up and connected
};

enum class UpdateSource : uint8_t {
    mqtt,
    device,
};

enum class SwitchMode : uint8_t {
    off,
    on,
};

enum class LedMode : uint8_t {
    bright,
    dim,
    off,
};

namespace state {
static const char *TOKEN = "CHANGE_ME"; // + ota pass

static bool power;

void setPower(bool p, UpdateSource source) {
    if (p == power)
        return;

    power = p;

    if (source == UpdateSource::mqtt) {
        command_queue::CommandItem ci;
        ci.command = (uint8_t)Command::power;
        ci.value = (uint8_t)power;
        command_queue::push(ci);
    }
}
} // namespace state
