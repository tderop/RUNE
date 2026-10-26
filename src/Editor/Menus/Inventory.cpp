#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>

#include <Utils/Utils.hpp>
#include <Save/Save.hpp>
#include <Json/Json.hpp>

#include <string>
#include <vector>
#include <map>

// CONFIG

// All of the items in Link's pouch are merged into one massive array of max length 420. This is going to be a little annoying.
const std::string INVENTORY_KEY = "PorchItem";
const std::map<std::string, std::vector<std::string>> INVENTORY_PREFIXES = {
    {
        "Weapons",
        {"Weapon_Sword", "Weapon_Lsword", "Weapon_Spear"}
    },
    {
        "Bows",
        {"Weapon_Bow"}
    },
    {
        "Shields",
        {"Weapon_Shield"}
    },
    {
        "Armor",
        {"Armor_"}
    },
    {
        "Materials",
        {"Item_", "BeeHome", "Animal_", "Obj_FirewoodBundle"}
    },
    {
        "Food",
        {"Item_Cook", "Item_Roast", "Item_Chilled"}
    },
    {
        "Key Items",
        {"Obj_", "PlayerStole2", "GameRomHorse"}
    }
}; // Not proud of this way of identifying inventory category. I blame BotW partially here for being abhorrently inconsistent, especially with the materials tab.

const std::map<std::string, std::vector<std::string>> INVENTORY_IGNORES = {
    {
        "Materials",
        {"Item_Cook", "Item_Roast", "Item_Chilled"}
    },
    {
        "Key Items",
        {"Obj_FirewoodBundle"}
    }
};

struct InventoryItem{
    std::string actorName;
    std::string name;
    size_t index;

    InventoryItem(std::string _actorName, std::string _name, size_t _index) : actorName(_actorName), name(_name), index(_index) {}
    InventoryItem() = default;
};

std::string getNameFromActorName(const std::string& actorName){
    // get json obj
    Rune::Json* actorNameJson;
    Result rc = Rune::JsonData::GetJsonById(actorNameJson, "ActorNames");
    if (R_FAILED(rc)){
        return actorName;
    }

    // find json entry for in ActorNames for actorName
    auto actorNameEntry = actorNameJson->find(actorName);
    if (actorNameEntry == actorNameJson->end() || !actorNameEntry->is_string()){
        return actorName;
    }

    // get the readable name
    return actorNameEntry->get<std::string>();
}

void Rune::MenuBuilder::Inventory(EditorMenu* menu){
    // Determine if there are inventory keys for the current menu
    auto prefixEntry = INVENTORY_PREFIXES.find(menu->menuName());

    // Do not proceed if no prefixes
    if (prefixEntry == INVENTORY_PREFIXES.end()){
        return;
    }

    std::vector<std::string> prefixes = prefixEntry->second;

    // Check for ignores too
    auto ignoresEntry = INVENTORY_IGNORES.find(menu->menuName());
    std::vector<std::string> ignores;

    if (ignoresEntry != INVENTORY_IGNORES.end()){
        ignores = ignoresEntry->second;
    }


    // Get a list of all the valid indexes for this category
    std::vector<InventoryItem> inventoryItems;

    Rune::Save::Entry* inventoryEntry = Rune::Save::GetEntryFromDataName(*menu->saveFile(),INVENTORY_KEY);
    auto inventory = std::get<std::vector<std::string>>(inventoryEntry->value);

    int currentIdx = -1;
    for (const std::string& actorName : inventory){
        // Two linear searches through prefixes and ignores
        currentIdx++;

        // Check if valid prefix
        bool foundPrefix = false;

        for (const std::string& prefix : prefixes){
            // If the prefix was found, check ignores too
            if ((actorName.find(prefix) != std::string::npos)){
                foundPrefix = true;
                break;
            }
        }

        // No prefix found? Continue to next inventory item
        if (!foundPrefix){
            continue;
        }  

        // Valid prefix found, check if this item is ignored
        if (!ignores.empty()){
            bool foundIgnore = false;

            // Iterate through ignores
            for (const std::string& ignore : ignores){
                // An ignore was found, break and continue to next prefix
                if (actorName.find(ignore) != std::string::npos){
                    foundIgnore = true;
                    break;
                }
            }

            // ignore this one
            if (foundIgnore){
                continue;
            }
        }

        // A valid item, process and make inventory object
        std::string itemName = getNameFromActorName(actorName);
        log("Valid item for "+menu->menuName()+": "+itemName);

        inventoryItems.emplace_back(actorName, itemName, currentIdx);

        // Make inventory tile
    }

    return;
}