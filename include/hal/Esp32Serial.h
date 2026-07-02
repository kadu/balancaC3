#pragma once

#include "hal/ISerial.h"

namespace hal {

class Esp32Serial final : public ISerial {
public:
    void begin(uint32_t baudRate) override;
    void print(const char* message) override;
    void println(const char* message) override;
    void println() override;
    bool available() override;
    char read() override;
};

} // namespace hal
