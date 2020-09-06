#pragma once

#include <Arduino.h>

#include "ntp.hpp"

namespace command {
char curr_char;
char buf[256];
int buf_idx;
static HardwareSerial &out = Serial;

void setup(HardwareSerial &hs) {
    out = hs;
}

void respond_ok() {
    out.print("ok\r");
}

void process_command() {
    char *command = strtok(buf, "\n ");

    if (strcmp(command, "get_down") == 0) {
        out.print("down none\r");
    } else if (strcmp(command, "net") == 0) {
        out.print("uap\r");
    } else if (strcmp(command, "time") == 0) {
        out.print(ntp::get_time());
        out.print("\n");
    } else if (strcmp(command, "props") == 0) {
        respond_ok();
    } else if (strcmp(command, "event") == 0) {
        respond_ok();
    } else if (strcmp(command, "model") == 0) {
        respond_ok();
    } else if (strcmp(command, "ble_config") == 0) {
        respond_ok();
    } else if (strcmp(command, "mcu_version") == 0) {
        respond_ok();
    } else if (strcmp(command, "result") == 0) {
        respond_ok();
    } else if (strcmp(command, "error") == 0) {
        respond_ok();
    } else if (strcmp(command, "restore") == 0) {
        // config::truncate();
        respond_ok();
    } else if (strcmp(command, "reboot") == 0) {
        respond_ok();
        ESP.restart();
    } else {
        out.printf("Unknown command: %s\n", command);
    }
}

void handle() {
    while (out.available() > 0) {
        curr_char = out.read();

        if (curr_char != '\r') {
            buf[buf_idx] = curr_char;
            buf_idx++;
            continue;
        }

        buf[buf_idx] = 0;
        buf_idx = 0;
        process_command();
    }
}
} // namespace command

namespace command_queue {
struct CommandItem {
    uint8_t command;
    uint8_t value;
};

uint8_t tail = 0;
uint8_t head = 0;
uint8_t count = 0;
uint8_t maxitems = 255;
uint8_t commands[256];
uint8_t values[256];

void push(CommandItem item) {
    // TODO: log error when full
    if (count >= maxitems)
        return; // drop when full

    count++;
    head++;

    if (head > maxitems) {
        head = 0;
    }

    commands[head] = item.command;
    values[head] = item.value;
}

CommandItem pop() {
    if (count == 0)
        return;

    CommandItem ci;
    ci.command = commands[tail];
    ci.value = commands[tail];
    tail++;
    count--;

    if (tail > maxitems) {
        tail = 0;
    }

    return ci;
}
} // namespace command_queue
