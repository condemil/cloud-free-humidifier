#pragma once

#include <Arduino.h>

namespace helpers {
class elapsedMillis // modified version of https://github.com/pfeerick/elapsedMillis
{
  private:
    uint16_t ms;

  public:
    elapsedMillis(void) {
        ms = millis();
    }
    operator uint16_t() const {
        return millis() - ms;
    }
    elapsedMillis &operator=(uint16_t val) {
        ms = millis() - val;
        return *this;
    }
};
} // namespace helpers
