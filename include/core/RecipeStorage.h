#pragma once

#include "hal/IFileSystem.h"
#include <WString.h>
#include <cstdint>

namespace core {

static constexpr char RECIPE_DIR[]        = "/recipes";
static constexpr char RECIPE_INDEX_PATH[] = "/recipes/index.json";
static constexpr uint8_t MAX_RECIPES      = 20;

class RecipeStorage {
public:
    explicit RecipeStorage(hal::IFileSystem& fs);

    void begin();

    // Returns JSON array [{id, title}, ...]
    String listRecipes();

    // Returns full recipe JSON or "" if not found
    String loadRecipe(uint16_t id);

    // Saves recipe JSON, assigns id if new (id==0). Returns assigned id or 0 on error.
    uint16_t saveRecipe(const char* json);

    // Deletes recipe by id
    bool deleteRecipe(uint16_t id);

private:
    hal::IFileSystem& _fs;

    uint16_t nextId();
    String   recipePath(uint16_t id);
};

} // namespace core
