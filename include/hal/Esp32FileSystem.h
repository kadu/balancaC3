#pragma once

#include "hal/IFileSystem.h"

namespace hal {

class Esp32FileSystem final : public IFileSystem {
public:
    bool begin() override;

    bool exists(const char* path) override;
    bool readFile(const char* path, String& out) override;
    bool writeFile(const char* path, const char* content) override;
    bool removeFile(const char* path) override;
    bool makeDir(const char* path) override;

    void listDir(const char* path, void (*cb)(const char* name, void* ctx), void* ctx) override;
};

} // namespace hal
