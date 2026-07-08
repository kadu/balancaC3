#include "hal/Esp32FileSystem.h"
#include <LittleFS.h>

namespace hal {

bool Esp32FileSystem::begin() {
    return LittleFS.begin(true); // formatOnFail=true
}

bool Esp32FileSystem::exists(const char* path) {
    return LittleFS.exists(path);
}

bool Esp32FileSystem::readFile(const char* path, String& out) {
    File f = LittleFS.open(path, "r");
    if (!f) return false;
    out = f.readString();
    f.close();
    return true;
}

bool Esp32FileSystem::writeFile(const char* path, const char* content) {
    File f = LittleFS.open(path, "w");
    if (!f) return false;
    f.print(content);
    f.close();
    return true;
}

bool Esp32FileSystem::removeFile(const char* path) {
    return LittleFS.remove(path);
}

bool Esp32FileSystem::makeDir(const char* path) {
    if (LittleFS.exists(path)) return true;
    return LittleFS.mkdir(path);
}

void Esp32FileSystem::listDir(const char* path, void (*cb)(const char* name, void* ctx), void* ctx) {
    File dir = LittleFS.open(path);
    if (!dir || !dir.isDirectory()) return;
    File entry = dir.openNextFile();
    while (entry) {
        if (!entry.isDirectory()) cb(entry.name(), ctx);
        entry = dir.openNextFile();
    }
}

} // namespace hal
