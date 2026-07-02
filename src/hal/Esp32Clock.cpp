#include "hal/Esp32Clock.h"
#include <Arduino.h>

namespace hal {

uint32_t Esp32Clock::millis() {
    return ::millis();
}

} // namespace hal
