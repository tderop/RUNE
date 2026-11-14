#include <Editor/Menus/Inventory.hpp>

#include <map>
#include <string>
#include <vector>

// CONFIG
const std::map<std::string, Rune::Inventory::Category> Rune::Inventory::kMenuNameToCategory = {
    {"Weapons", Rune::Inventory::Category::Weapons},
    {"Bows", Rune::Inventory::Category::Bows},
    {"Shields", Rune::Inventory::Category::Shields},
    {"Armor", Rune::Inventory::Category::Armor},
    {"Materials", Rune::Inventory::Category::Materials},
    {"Food", Rune::Inventory::Category::Food},
    {"Key Items", Rune::Inventory::Category::KeyItems},
};

const std::map<Rune::Inventory::Category, std::string> Rune::Inventory::kFlagSpKeys = {
    {Rune::Inventory::Category::Weapons, "PorchSword_FlagSp"},
    {Rune::Inventory::Category::Bows, "PorchBow_FlagSp"},
    {Rune::Inventory::Category::Shields, "PorchShield_FlagSp"}
};

const std::map<Rune::Inventory::Category, std::string> Rune::Inventory::kSlotKeys = {
    {Rune::Inventory::Category::Weapons, "WeaponPorchStockNum"},
    {Rune::Inventory::Category::Bows, "BowPorchStockNum"},
    {Rune::Inventory::Category::Shields, "ShieldPorchStockNum"}
};

const std::map<Rune::Inventory::Category, std::string> Rune::Inventory::kValueSpKeys = {
    {Rune::Inventory::Category::Weapons, "PorchSword_ValueSp"},
    {Rune::Inventory::Category::Bows, "PorchBow_ValueSp"},
    {Rune::Inventory::Category::Shields, "PorchShield_ValueSp"}
};

const std::map<Rune::Inventory::Category, std::string> Rune::Inventory::kLabelValueKeys = {
    {Rune::Inventory::Category::Weapons, "Attack"}, // Account for attack up
    {Rune::Inventory::Category::Bows, "Attack"}, // Account for attack up & multishot
    {Rune::Inventory::Category::Shields, "Guard"}, // Account for guard up
    {Rune::Inventory::Category::Armor, "Defence"}
};

const std::map<Rune::Inventory::Category, std::vector<Rune::Inventory::Modifier>> Rune::Inventory::kCategoryModifiers = {
    {
        Rune::Inventory::Category::Weapons,
        {
            Rune::Inventory::Modifier::AttackUp, Rune::Inventory::Modifier::DurabilityUp, 
            Rune::Inventory::Modifier::CriticalHit, Rune::Inventory::Modifier::LongThrow,
            Rune::Inventory::Modifier::AttackUpPlus, Rune::Inventory::Modifier::DurabilityUpPlus,
            Rune::Inventory::Modifier::None
        }
    },
    {
        Rune::Inventory::Category::Bows,
        {
            Rune::Inventory::Modifier::AttackUp, Rune::Inventory::Modifier::DurabilityUp,
            Rune::Inventory::Modifier::MultiShot, Rune::Inventory::Modifier::Zoom,
            Rune::Inventory::Modifier::QuickShot, Rune::Inventory::Modifier::AttackUpPlus,
            Rune::Inventory::Modifier::DurabilityUpPlus, Rune::Inventory::Modifier::MultiShotPlus,
            Rune::Inventory::Modifier::None
        }
    },
    {
        Rune::Inventory::Category::Shields,
        {
            Rune::Inventory::Modifier::GuardUp, Rune::Inventory::Modifier::DurabilityUp,
            Rune::Inventory::Modifier::SurfMaster, Rune::Inventory::Modifier::GuardUpPlus,
            Rune::Inventory::Modifier::DurabilityUpPlus, Rune::Inventory::Modifier::None
        }
    }
};

const std::map<Rune::Inventory::Category, size_t> Rune::Inventory::kMaxSlots = {
    {Rune::Inventory::Category::Weapons, 20},
    {Rune::Inventory::Category::Bows, 13},
    {Rune::Inventory::Category::Shields, 20},
    {Rune::Inventory::Category::Armor, 100}, // goof
    {Rune::Inventory::Category::Materials, 160},
    {Rune::Inventory::Category::Food, 60},
    {Rune::Inventory::Category::KeyItems, 40},
};

const std::map<Rune::Inventory::Category, std::vector<std::string>> Rune::Inventory::kInventoryPrefixes = {
    {
        Rune::Inventory::Category::Weapons,
        {"Weapon_Sword", "Weapon_Lsword", "Weapon_Spear"}
    },
    {
        Rune::Inventory::Category::Bows,
        {"Weapon_Bow", "Arrow"}
    },
    {
        Rune::Inventory::Category::Shields,
        {"Weapon_Shield"}
    },
    {
        Rune::Inventory::Category::Armor,
        {"Armor_"}
    },
    {
        Rune::Inventory::Category::Materials,
        {"Item_", "BeeHome", "Animal_", "Obj_FireWoodBundle"}
    },
    {
        Rune::Inventory::Category::Food,
        {"Item_Cook", "Item_Roast", "Item_Chilled"}
    },
    {
        Rune::Inventory::Category::KeyItems,
        {"Obj_", "PlayerStole2", "GameRomHorse"}
    }
}; // Not proud of this way of identifying inventory category. I blame BotW partially here for being abhorrently inconsistent, especially with the materials tab.

const std::map<Rune::Inventory::Category, std::vector<std::string>> Rune::Inventory::kInventoryIgnores = {
    {
        Rune::Inventory::Category::Materials,
        {"Item_Cook", "Item_Roast", "Item_Chilled"}
    },
    {
        Rune::Inventory::Category::KeyItems,
        {"Obj_FireWoodBundle"}
    }
};

const std::map<Rune::Inventory::Category, Rune::Inventory::TileStyle> Rune::Inventory::kLabelTileStyles = {
    {Rune::Inventory::Category::Weapons, Rune::Inventory::TileStyle::Label},
    {Rune::Inventory::Category::Bows, Rune::Inventory::TileStyle::Label},
    {Rune::Inventory::Category::Shields, Rune::Inventory::TileStyle::Label},
    {Rune::Inventory::Category::Armor, Rune::Inventory::TileStyle::Label}
};

const std::map<Rune::Inventory::Category, std::string> Rune::Inventory::kCategoryDevNotes = {
    {
        Rune::Inventory::Category::Weapons,
        "Edit three key values: Durability, Modifier, and Modifier Value\n"
        "────────────────\n"
        "• Durability: Remaining hits until the weapon breaks, hard-capped at 65,535.\n\n"
        "• Modifier: The weapon's buff. Normal and plus versions are purely cosmetic.\n\n"
        "• Modifier Value: Buff strength, e.g. Attack Up + 25."
    },
    {
        Rune::Inventory::Category::Bows,
        "Edit three key values: Durability, Modifier, and Modifier Value\n"
        "────────────────\n"
        "• Durability: Remaining hits until the bow breaks, hard-capped at 65,535.\n\n"
        "• Modifier: The bow's buff. Normal and plus versions are purely cosmetic.\n\n"
        "• Modifier Value: Buff strength, e.g. Multi Shot x3."
    },
    {
        Rune::Inventory::Category::Shields,
        "Edit three key values: Durability, Modifier, and Modifier Value\n"
        "────────────────\n"
        "• Durability: Remaining hits until the shield breaks, hard-capped at 65,535.\n\n"
        "• Modifier: The shield's buff. Normal and plus versions are purely cosmetic.\n\n"
        "• Modifier Value: Buff strength, e.g. Guard Up + 25."
    },
    {
        Rune::Inventory::Category::Armor,
        ""
    },
};