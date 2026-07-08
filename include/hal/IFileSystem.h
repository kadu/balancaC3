#pragma once

#include <WString.h>
#include <cstdint>

namespace hal {

class IFileSystem {
public:
    virtual ~IFileSystem() = default;

    virtual bool begin() = 0;

    virtual bool   exists(const char* path) = 0;
    virtual bool   readFile(const char* path, String& out) = 0;
    virtual bool   writeFile(const char* path, const char* content) = 0;
    virtual bool   removeFile(const char* path) = 0;
    virtual bool   makeDir(const char* path) = 0;

    // List files in a directory — calls cb(filename) for each entry
    virtual void   listDir(const char* path, void (*cb)(const char* name, void* ctx), void* ctx) = 0;
};

} // namespace hal
