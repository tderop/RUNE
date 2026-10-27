#include <Aether/Aether.hpp>

#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>
#include <Editor/EditorWidget.hpp>

#include <Utils/Utils.hpp>
#include <Save/Save.hpp>
#include <Json/Json.hpp>
#include <Config.hpp>
#include <Input/Input.hpp>

#include <string>
#include <vector>
#include <map>
#include <cmath>

// CONFIG

// UI
const int X_START_OFFSET = 140;
const int Y_START_OFFSET = 36;

const int TILE_SIZE = 120;
const int TILE_PADDING = 12;

const int TILES_PER_ROW = 5;
const int TILES_PER_PAGE = 20;

const Aether::Colour DISABLED_ELLIPSE{255, 255, 255, 75};
const Aether::Colour ENABLED_ELLIPSE{255, 255, 255, 255};

// Truths
const int GRID_WIDTH  = (TILE_SIZE+TILE_PADDING)*TILES_PER_ROW;
const int GRID_HEIGHT = (TILE_SIZE+TILE_PADDING)*(TILES_PER_PAGE/TILES_PER_ROW);

// All of the items in Link's pouch are merged into one massive array of max length 420. This is going to be a little annoying.
const std::string INVENTORY_KEY        = "PorchItem";
const std::string INVENTORY_VALUE_KEY  = "PorchItem_Value1";
const std::string INVENTORY_EQUIP_FLAG = "PorchItem_EquipFlag";

const std::map<std::string, std::string> FLAGSP_KEYS = {
    {"Weapons", "PorchSword_FlagSp"},
    {"Bows", "PorchBow_FlagSp"},
    {"Shields", "PorchShield_FlagSp"}
};

const std::map<std::string, std::string> SLOT_KEYS = {
    {"Weapons", "WeaponPorchStockNum"},
    {"Bows", "BowPorchStockNum"},
    {"Shields", "ShieldPorchStockNum"}
};

const std::map<std::string, std::string> VALUESP_KEYS = {
    {"Weapons", "PorchSword_ValueSp"},
    {"Bows", "PorchBow_ValueSp"},
    {"Shields", "PorchShield_ValueSp"}
};

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

std::string formatValue(const std::string& menuName, const InventoryItem& inventoryItem){
    // if menuName is materials, it will be X
    for (const std::string& otherMenu : quantityMenus){
        if (otherMenu == menuName){
            return "x"+std::to_string(inventoryItem.value);
        }
    }

    return "";
}

void showTilePage(EditorMenu* menu, std::vector<InventoryItem> inventoryItems, size_t page, std::vector<Aether::Ellipse*>* ellipses=nullptr){
    // Clear all current widgets
    menu->clearAllWidgets();

    // The menu body, used for positioning tiles
    Aether::Container* menuBody = menu->body();

    int startX = menuBody->x() + X_START_OFFSET;
    int startY = menuBody->y() + Y_START_OFFSET;

    size_t cursorStart = page*TILES_PER_PAGE;
    size_t cursorEnd   = static_cast<size_t>(
        std::min(
            static_cast<int>(cursorStart+TILES_PER_PAGE),
            static_cast<int>(inventoryItems.size())
        )
    );

    for (size_t i = cursorStart; i < cursorEnd; ++i){
        // Get the inventory item
        const InventoryItem& inventoryItem = inventoryItems.at(i);

        // Calculate position of the new tile
        int tileX = startX + (TILE_SIZE+TILE_PADDING)*((i-cursorStart)%TILES_PER_ROW);
        int tileY = startY + ((i-cursorStart)/TILES_PER_ROW)*(TILE_SIZE+TILE_PADDING);

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
                inventoryItem.name,
                std::string("NoLabel"),
                formatValue(menu->menuName(), inventoryItem)
            },
            menu->body()
        );
    }

    // Highlight the right menu dot
    if (ellipses != nullptr){
        for (size_t i = 0; i < ellipses->size(); ++i){
            if (i != page){
                ellipses->at(i)->setColour(DISABLED_ELLIPSE);
            } else {
                ellipses->at(i)->setColour(ENABLED_ELLIPSE);
            }
        }
    }
}

void Rune::MenuBuilder::Inventory(EditorMenu* menu){
    // Disable right stick navigation
    Aether::SetRightStickNavigation(false);

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

        // log("Valid item for "+menu->menuName()+": "+itemName+" ("+actorName+") x"+std::to_string(itemValue), true);

        InventoryItem inventoryItem{actorName, itemName, itemValue, static_cast<size_t>(currentIdx)};
        inventoryItems.push_back(inventoryItem);
    }

    // Calculate max pages
    int maxPage = static_cast<float>(inventoryItems.size()-1) / static_cast<float>(TILES_PER_PAGE);
    int* currentPage = new int(0);

    // The dots showing which page is currently being viewed
    std::vector<Aether::Ellipse*>* ellipses = new std::vector<Aether::Ellipse*>;

    for (size_t i = 0; i <= maxPage; ++i){
        Aether::Ellipse* menuDot = new Aether::Ellipse(
            menu->body()->x()+X_START_OFFSET + (i*(8+TILE_PADDING)), 
            menu->body()->y()+Y_START_OFFSET - 18, 
            8
        );

        menuDot->setColour(DISABLED_ELLIPSE);
        
        menu->addElement(menuDot);
        menu->body()->addElement(menuDot);

        ellipses->push_back(menuDot);
    }

    showTilePage(menu, inventoryItems, *currentPage, ellipses);

    InputSystem& inputService = InputSystem::getService();
    inputService.bind("NextPage", InputObject(InputType::Gamepad, HidNpadButton_StickRRight));
    inputService.bind("PrevPage", InputObject(InputType::Gamepad, HidNpadButton_StickRLeft));

    inputService.connect("NextPage", [menu, inventoryItems, currentPage, maxPage, ellipses](InputSystem& svc, InputState state){
        if (state != InputState::Begin){
            return;
        }

        // Change current page, ignore if at last page
        if (*currentPage >= maxPage){
            return;
        }

        *currentPage += 1;
        showTilePage(menu, inventoryItems, static_cast<size_t>(*currentPage), ellipses);
    });

    inputService.connect("PrevPage", [menu, inventoryItems, currentPage, ellipses](InputSystem& svc, InputState state){
        if (state != InputState::Begin){
            return;
        }

        if (*currentPage <= 0){
            return;
        }

        *currentPage -= 1;
        showTilePage(menu, inventoryItems, static_cast<size_t>(*currentPage), ellipses);
    });

    menu->giveTask([ellipses](){
        InputSystem& inputService = InputSystem::getService();
        inputService.unbind("NextPage");
        inputService.unbind("PrevPage");

        // Enable right stick navigation
        Aether::SetRightStickNavigation(true);

        // Delete the ellipses vector
        delete ellipses;
    });

    return;
}