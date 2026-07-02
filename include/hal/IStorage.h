#pragma once

#include <cstddef>

namespace hal {

class IStorage {
public:
    virtual ~IStorage() = default;

    virtual bool getString(const char* key, char* out, size_t maxLen) = 0;
    virtual void putString(const char* key, const char* value) = 0;
    virtual bool hasKey(const char* key) = 0;
    virtual void remove(const char* key) = 0;
};

} // namespace hal
