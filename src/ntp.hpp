#pragma once

#include <time.h>

namespace ntp {
static time_t now;
static char timeBuff[20];

void setup() {
    configTime(PSTR("UTC0"), "pool.ntp.org"); // TODO: configurable timezone
}

char *get_time() {
    now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    strftime(timeBuff, sizeof(timeBuff), "%Y-%m-%d %H:%M:%S", timeinfo);
    return timeBuff;
}

uint32_t get_unixtime() {
    return time(nullptr);
}
} // namespace ntp
