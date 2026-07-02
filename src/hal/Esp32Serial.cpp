#include "hal/Esp32Serial.h"
#include <Arduino.h>

namespace hal {

void Esp32Serial::begin(uint32_t baudRate) {
    Serial.begin(baudRate);
}

void Esp32Serial::print(const char* message) {
    Serial.print(message);
}

void Esp32Serial::println(const char* message) {
    Serial.println(message);
}

void Esp32Serial::println() {
    Serial.println();
}

bool Esp32Serial::available() {
    return Serial.available() > 0;
}

char Esp32Serial::read() {
    return static_cast<char>(Serial.read());
}

} // namespace hal
