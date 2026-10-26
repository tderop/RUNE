#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>
#include <Editor/EditorWidget.hpp>

#include <Utils/Utils.hpp>
#include <Save/Save.hpp>
#include <Json/Json.hpp>
#include <Config.hpp>

#include <string>
#include <vector>
#include <map>

// CONFIG

// UI
const int X_START_OFFSET = 140;
const int Y_START_OFFSET = 36;

const int TILE_SIZE = 120;
const int TILE_PADDING = 12;

const int TILES_PER_ROW = 5;

// All of the items in Link's pouch are merged into one massive array of max length 420. This is going to be a little annoying.
const std::string INVENTORY_KEY = "PorchItem";
const std::string INVENTORY_VALUE_KEY = "PorchItem_Value1";

// Bounds
const int MAX_MATERIAL_QUANTITY = 999;
const int MAX_WEAPON_DURABILITY = __INT32_MAX__;

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
    int32_t value;

    size_t index;

    InventoryItem(std::string _actorName, std::string _name, int32_t _value, size_t _index) : actorName(_actorName), name(_name), value(_value), index(_index) {}
    InventoryItem() = default;
};

/**
 * InventoryItem Value:
 * - Weapons (Swords, Shields, Bows): Durability (MAX __INT32_MAX__) * 100 (why x100 I do not know, f32 support without f32 array?)
 * - Materials, Food, Key Items: Quantity (MAX 999)
 * - Armor: Dye Color (0 = undyed, -1 = not dyeable, 15 = max dye color - needs more RE)
 */

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

// Arrows may need to be handled separately
std::vector<std::string> quantityMenus = {"Materials", "Food", "Key Items"};

std::string formatValue(const std::string& menuName, int32_t value){
    // if menuName is materials, it will be X
    for (const std::string& otherMenu : quantityMenus){
        if (otherMenu == menuName){
            return "x"+std::to_string(value);
        }
    }

    return "";
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

    // Get the itemValues entry entry
    Rune::Save::Entry* valuesEntry = Rune::Save::GetEntryFromDataName(*menu->saveFile(), INVENTORY_VALUE_KEY);
    auto itemValues = std::get<std::vector<int32_t>>(valuesEntry->value); 

    int currentIdx = -1;
    int localIdx = 0; // this index only increments when a valid slot has been found

    // The menu body, used for positioning tiles
    Aether::Container* menuBody = menu->body();

    int startX = menuBody->x() + X_START_OFFSET;
    int startY = menuBody->y() + Y_START_OFFSET;

    // Iterate through inventory and populate slots
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
        int32_t itemValue = itemValues.at(currentIdx);

        log("Valid item for "+menu->menuName()+": "+itemName+" ("+actorName+") x"+std::to_string(itemValue), true);

        InventoryItem inventoryItem{actorName, itemName, itemValue, static_cast<size_t>(currentIdx)};
        inventoryItems.push_back(inventoryItem);

        // Calculate position of the new tile
        int tileX = startX + (TILE_SIZE+TILE_PADDING)*(localIdx%TILES_PER_ROW);
        int tileY = startY + (localIdx/TILES_PER_ROW)*(TILE_SIZE+TILE_PADDING);

        // Make inventory tile (use widgets)
        Rune::MenuBuilder::AddWidgetFromSaveKey(
            menu,
            INVENTORY_KEY,
            true,
            {
                tileX, 
                tileY, 
                TILE_SIZE, 
                TILE_SIZE, 
                itemName,
                std::string("NoLabel"),
                formatValue(menu->menuName(), itemValue)
            },
            menu->body()
        );

        localIdx++;
    }

    return;
}