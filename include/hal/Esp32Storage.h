#pragma once

#include "hal/IStorage.h"

namespace hal {

class Esp32Storage final : public IStorage {
public:
    bool getString(const char* key, char* out, size_t maxLen) override;
    void putString(const char* key, const char* value) override;
    bool hasKey(const char* key) override;
    void remove(const char* key) override;
};

} // namespace hal
