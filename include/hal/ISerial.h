#pragma once

#include <cstdint>
#include <cstddef>

namespace hal {

class ISerial {
public:
    virtual ~ISerial() = default;

    virtual void begin(uint32_t baudRate) = 0;
    virtual void print(const char* message) = 0;
    virtual void println(const char* message) = 0;
    virtual void println() = 0;
    virtual bool available() = 0;
    virtual char read() = 0;
};

} // namespace hal
