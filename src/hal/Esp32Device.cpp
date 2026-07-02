#include "hal/Esp32Device.h"
#include <Arduino.h>

namespace hal {

void Esp32Device::restart() {
    ESP.restart();
}

} // namespace hal
