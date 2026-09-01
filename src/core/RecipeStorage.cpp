#include <Arduino.h>
#include <ArduinoJson.h>
#include "core/RecipeStorage.h"
#include <cstdio>

namespace core {

RecipeStorage::RecipeStorage(hal::IFileSystem& fs) : _fs(fs) {}

void RecipeStorage::begin() {
    _fs.makeDir(RECIPE_DIR);
}

String RecipeStorage::recipePath(uint16_t id) {
    char buf[32];
    snprintf(buf, sizeof(buf), "/recipes/%u.json", id);
    return String(buf);
}

String RecipeStorage::listRecipes() {
    String index;
    if (_fs.readFile(RECIPE_INDEX_PATH, index) && index.length() > 2) return index;

    // Rebuild index by scanning directory
    struct Ctx { String result; bool first; };
    Ctx ctx{ "[\n", true };

    _fs.listDir(RECIPE_DIR, [](const char* name, void* raw) {
        auto* c = static_cast<Ctx*>(raw);
        // Skip index.json
        if (String(name).endsWith("index.json")) return;

        String content;
        String fullPath = String("/recipes/") + name;
        // We only need id and title — parse minimal fields
        // (file name is "<id>.json")
        String fname = String(name);
        int dot = fname.lastIndexOf('.');
        if (dot < 0) return;
        String idStr = fname.substring(0, dot);
        uint16_t id = static_cast<uint16_t>(idStr.toInt());
        if (id == 0) return;

        // Read title from file
        extern hal::IFileSystem* _gFs; // forward declared below
        (void)raw; // suppress warning — ctx carries fs reference via closure trick
        // We must read the file — use the ctx trick: store fs ptr in ctx
    }, &ctx);

    // Simpler: just list by reading each file
    // Reset and do it properly with a dedicated scan
    DynamicJsonDocument doc(4096);
    JsonArray arr = doc.to<JsonArray>();

    // We need fs access — store it in a lambda-compatible way
    struct ScanCtx {
        hal::IFileSystem* fs;
        JsonArray*        arr;
    };
    ScanCtx scanCtx{&_fs, &arr};

    _fs.listDir(RECIPE_DIR, [](const char* name, void* raw) {
        auto* sc = static_cast<ScanCtx*>(raw);
        String fname = String(name);
        if (fname.endsWith("index.json")) return;
        int dot = fname.lastIndexOf('.');
        if (dot < 0) return;
        uint16_t id = static_cast<uint16_t>(fname.substring(0, dot).toInt());
        if (id == 0) return;

        char path[32];
        snprintf(path, sizeof(path), "/recipes/%s", name);
        String content;
        if (!sc->fs->readFile(path, content)) return;

        DynamicJsonDocument entry(2048);
        if (deserializeJson(entry, content) != DeserializationError::Ok) return;

        JsonObject obj = sc->arr->createNestedObject();
        obj["id"]         = id;
        obj["title"]      = entry["title"] | "";
        obj["waterTotal"] = entry["waterTotal"] | 0;
        obj["totalSecs"]  = entry["totalSecs"]  | 0;
    }, &scanCtx);

    String result;
    serializeJson(doc, result);

    // Cache the index
    _fs.writeFile(RECIPE_INDEX_PATH, result.c_str());
    return result;
}

String RecipeStorage::loadRecipe(uint16_t id) {
    String content;
    if (!_fs.readFile(recipePath(id).c_str(), content)) return "";
    return content;
}

uint16_t RecipeStorage::saveRecipe(const char* json) {
    DynamicJsonDocument doc(4096);
    if (deserializeJson(doc, json) != DeserializationError::Ok) return 0;

    uint16_t id = doc["id"] | 0;
    if (id == 0) {
        id = nextId();
        doc["id"] = id;
    }

    String out;
    serializeJson(doc, out);

    if (!_fs.writeFile(recipePath(id).c_str(), out.c_str())) return 0;

    // Invalidate cached index
    _fs.removeFile(RECIPE_INDEX_PATH);
    return id;
}

bool RecipeStorage::deleteRecipe(uint16_t id) {
    bool ok = _fs.removeFile(recipePath(id).c_str());
    _fs.removeFile(RECIPE_INDEX_PATH); // invalidate cache
    return ok;
}

uint16_t RecipeStorage::nextId() {
    uint16_t maxId = 0;
    _fs.listDir(RECIPE_DIR, [](const char* name, void* raw) {
        auto* m = static_cast<uint16_t*>(raw);
        String fname = String(name);
        if (fname.endsWith("index.json")) return;
        int dot = fname.lastIndexOf('.');
        if (dot < 0) return;
        uint16_t id = static_cast<uint16_t>(fname.substring(0, dot).toInt());
        if (id > *m) *m = id;
    }, &maxId);
    return maxId + 1;
}

} // namespace core
