#pragma once

#include <map>

class EditorMenu;

namespace Rune{
    namespace MenuBuilder{
        /** @brief Create an entry-based menu */
        void Entry(EditorMenu* menu);

        /** @brief Create an 'inventory-style' menu */
        void Inventory(EditorMenu* menu);

        /** @brief Create a menu for all entries. Used by the Dev Editor. */
        void AllEntries(EditorMenu* menu);
    }
};

using MenuBuilderFunc = void(*)(EditorMenu*);
using MenuBuilderMap  = std::map<std::string, MenuBuilderFunc>;

inline const MenuBuilderMap MENU_BUILDERS = {
    {"AllEntries", Rune::MenuBuilder::AllEntries},
    {"Inventory", Rune::MenuBuilder::Inventory},
    {"Entry", Rune::MenuBuilder::Entry}
};