#include "hal/Esp32Storage.h"
#include <Preferences.h>
#include <cstring>

// All keys live under the "app" NVS namespace.
static constexpr char NVS_NAMESPACE[] = "app";

namespace hal {

bool Esp32Storage::getString(const char* key, char* out, size_t maxLen) {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    if (!prefs.isKey(key)) { prefs.end(); return false; }
    String val = prefs.getString(key, "");
    prefs.end();
    strncpy(out, val.c_str(), maxLen - 1);
    out[maxLen - 1] = '\0';
    return true;
}

void Esp32Storage::putString(const char* key, const char* value) {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putString(key, value);
    prefs.end();
}

bool Esp32Storage::hasKey(const char* key) {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    bool has = prefs.isKey(key);
    prefs.end();
    return has;
}

void Esp32Storage::remove(const char* key) {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);
    prefs.remove(key);
    prefs.end();
}

} // namespace hal
