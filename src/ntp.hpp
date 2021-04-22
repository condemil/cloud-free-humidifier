#pragma once

#include <time.h>

#include "logger.hpp"

namespace ntp {
static char timeBuff[20];
time_t now;
struct tm *timeinfo;

void setup() {
    configTime(PSTR("UTC0"), "pool.ntp.org"); // TODO: configurable timezone
}

char *get_time() {
    now = time(nullptr);
    timeinfo = localtime(&now);

    strftime(timeBuff, sizeof(timeBuff), "%Y-%m-%d %H:%M:%S", timeinfo);

    return timeBuff;
}
} // namespace ntp
