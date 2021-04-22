#pragma once

enum class Command : uint8_t {
    buzzer,
    child_lock,
    power,
    preset_mode,
    dry,
    led_brightness,
    limit_humiditiy,
    net_change,
};

enum class PresetMode : uint8_t {
    auto_,
    high,
    medium,
    silent,
};

enum class NetMode : uint8_t {
    uap, // unprovisioned, access point created
    unprov, // unprovisioned, access point disabled (after 1 hour of being uap)
    local, // wifi is properly set up and connected
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
static bool power;
static PresetMode preset_mode;

static float temperature;
static uint8_t humidity;
static uint8_t depth;

static bool power_updated;
static bool preset_mode_updated;
} // namespace state
