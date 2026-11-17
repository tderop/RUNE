#pragma once

#include <Save/Save.hpp>
#include <Editor/Menus/Inventory.hpp>

#include <vector>

namespace Rune {
    namespace Inventory {
        /**
         * @brief An object representing link's pouch (or 'porch')
         */
        class Pouch {
            private:
                Cache entryCache_=Cache();
                PouchItems items_;
                Save::File* saveFile_;

                /**
                 * @brief Make a key from a category and a value custom identifier (e.g. flagSp) to access the cached entry
                 * @note Used by Pouch::getOrAdd()
                 */ 
                std::string makeCacheKey(Category category, const std::string& id);

                /**
                 * @brief Get a value in the entry cache at a provided key, 
                 * or add it if it doesn't exist and read it from save data
                 */
                std::vector<int32_t> getOrAdd(Category category, const std::string& id, const std::string& vkey);
            
            public:
                /**
                 * @brief Construct a new pouch object
                 * @param saveFile The save file the pouch represents
                 */
                Pouch(Save::File* saveFile);

                /** 
                 * @brief Check if the pouch has an item by its actor name
                 * @param actorName The actor name to check
                 * @return If the pouch contains the item
                 */
                bool hasItem(std::string_view actorName);

                /**
                 * @brief Get a list of all the items in Link's pouch
                 */
                PouchItems getItems();

                /**
                 * @brief Get a list of items in a specified inventory category
                 * @param category The target category
                 * @param slots The number of slots wanted (may return less)
                 * @return The list of items in the specified inventory category
                 */
                PouchItems getItemsByCategory(Category category, size_t slots);

                /**
                 * @brief Reset the category of all blank tiles to Category::Unknown
                 */
                void resetBlankSlots();

                /**
                 * @brief Save the changes made to the pouch to the save file
                 */
                void save();

                ~Pouch();
        };
    }
}