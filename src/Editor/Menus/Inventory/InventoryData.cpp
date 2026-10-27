#include <Editor/Inventory.hpp>

#include <map>
#include <string>
#include <vector>

// CONFIG
const std::map<std::string, std::string> Rune::Inventory::kFlagSpKeys = {
    {"Weapons", "PorchSword_FlagSp"},
    {"Bows", "PorchBow_FlagSp"},
    {"Shields", "PorchShield_FlagSp"}
};

const std::map<std::string, std::string> Rune::Inventory::kSlotKeys = {
    {"Weapons", "WeaponPorchStockNum"},
    {"Bows", "BowPorchStockNum"},
    {"Shields", "ShieldPorchStockNum"}
};

const std::map<std::string, std::string> Rune::Inventory::kValueSpKeys = {
    {"Weapons", "PorchSword_ValueSp"},
    {"Bows", "PorchBow_ValueSp"},
    {"Shields", "PorchShield_ValueSp"}
};

const std::map<std::string, std::string> Rune::Inventory::kLabelValueKeys = {
    {"Weapons", "Attack"}, // Account for attack up
    {"Bows", "Attack"}, // Account for attack up & multishot
    {"Shields", "Guard"}, // Account for guard up
    {"Armor", "Defence"}
};

const std::map<std::string, size_t> Rune::Inventory::kMaxSlots = {
    {"Weapons", 21},
    {"Bows", 13},
    {"Shields", 20},
    {"Armor", 100}, // goof
    {"Materials", 160},
    {"Food", 60},
    {"Key Items", 40},
};

const std::map<std::string, std::vector<std::string>> Rune::Inventory::kInventoryPrefixes = {
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
        {"Item_", "BeeHome", "Animal_", "Obj_FireWoodBundle"}
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

const std::map<std::string, std::vector<std::string>> Rune::Inventory::kInventoryIgnores = {
    {
        "Materials",
        {"Item_Cook", "Item_Roast", "Item_Chilled"}
    },
    {
        "Key Items",
        {"Obj_FireWoodBundle"}
    }
};

extern const std::map<std::string, Rune::Inventory::TileStyle> Rune::Inventory::kLabelTileStyles = {
    {"Weapons", Rune::Inventory::TileStyle::Label},
    {"Bows", Rune::Inventory::TileStyle::Label},
    {"Shields", Rune::Inventory::TileStyle::Label},
    {"Armor", Rune::Inventory::TileStyle::Label}
};