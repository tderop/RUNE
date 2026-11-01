#pragma once

#include <Aether/Aether.hpp>
#include <string>
#include <map>

#include <Utils/Cache.hpp>

namespace Rune{
    namespace Inventory{
        // UI Configurations
        inline constexpr int kXStartOffset = 140;
        inline constexpr int kYStartOffset = 42;

        inline constexpr int kTileSize = 120;
        inline constexpr int kTilePadding = 12;

        inline constexpr int kTilesPerRow = 5;
        inline constexpr int kTilesPerPage = 20;

        inline constexpr int kDotWidth = 8;
        inline constexpr int kDotPadding = 12;

        inline constexpr int kGridWidth  = (kTileSize+kTilePadding)*kTilesPerRow;
        inline constexpr int kGridHeight = (kTileSize+kTilePadding)*(kTilesPerPage/kTilesPerRow);
        
        inline constexpr float kThumbScale = 0.85;

        inline const Aether::Colour kDisabledEllipse{255, 255, 255, 75};
        inline const Aether::Colour kEnabledEllipse{255, 255, 255, 255};

        static Rune::Cache* iconPathCache = new Rune::Cache(420);

        // Determines which inventory tab an item belongs to
        enum class Category{
            Weapons,
            Bows,
            Arrows,
            Shields,
            Armor,
            Materials,
            Food,
            KeyItems,
            Unknown
        };

        // All of the items in Link's pouch are merged into one massive array of max length 420. This is going to be a little annoying.
        inline const std::string kInventoryKey       = "PorchItem";
        inline const std::string kInventoryValueKey  = "PorchItem_Value1";
        inline const std::string kInventoryEquipFlag = "PorchItem_EquipFlag";

        // For elixirs
        inline const std::string kElixirPrefix = "Item_Cook_C_17"; // other effects are like Item_Cook_C_17_DefenseUp for example
        // Look into CookEffect0 (vector2f array)

        extern const std::map<std::string, Category> kMenuNameToCategory;

        extern const std::map<Category, std::string> kFlagSpKeys;
        extern const std::map<Category, std::string> kSlotKeys;
        extern const std::map<Category, std::string> kValueSpKeys;
        extern const std::map<Category, size_t> kMaxSlots;

        extern const std::map<Category, std::vector<std::string>> kInventoryPrefixes;
        extern const std::map<Category, std::vector<std::string>> kInventoryIgnores;

        extern const std::map<Category, std::string> kLabelValueKeys;
        inline const std::vector<Category> kQuantityMenus = {Category::Materials, Category::Food, Category::KeyItems};

        inline constexpr int kMaxMaterialQuantity = 999;
        inline constexpr int kMaxWeaponDurability = __INT32_MAX__;
        
        enum class TileType{
            Standard,
            Empty,
            Add
        };

        enum class TileStyle{
            NoLabel,
            Label
        };

        extern const std::map<Category, Rune::Inventory::TileStyle> kLabelTileStyles;

        struct Item{
            std::string actorName;
            std::string name;
            int32_t value;
            size_t index;

            Category category;

            int32_t flagSp=0; // buff
            int32_t valueSp=0; // buff value

            TileStyle style=TileStyle::NoLabel;
            TileType type=TileType::Standard;

            std::string iconActorName;
            std::string thumbPath;

            std::string nextRankActorName;
            std::string prevRankActorName;

            int attack=-1; // Attack damage (weapons and bows)
            int guard=-1; // Shield guard (shields only)
            int durability=-1; // Durability (weapons, bows, and shields)
            int defence=-1; // Attack resistance (armor only)

            int x=0;
            int y=0;
            int w=kTileSize;
            int h=kTileSize;

            Item(
                std::string _actorName, 
                std::string _name, 
                int32_t _value, 
                size_t _index,
                Category _category,
                int32_t _flagSp=0,
                int32_t _valueSp=0,
                TileStyle _style=TileStyle::NoLabel,
                TileType _type=TileType::Standard,
                std::string _thumbPath=""
            ) : actorName(_actorName), name(_name), value(_value), index(_index), category(_category),
                flagSp(_flagSp), valueSp(_valueSp), style(_style), type(_type), thumbPath(_thumbPath) {}

            Item() = default;
        };

        std::string GetThumbPathFromActorName(const std::string& actorName, bool noVerify=false);
        std::string GetThumbPathFromInventoryItem(const Item& inventoryItem);
        std::string GetNameFromActorName(const std::string& actorName);
        Category GetCategoryFromMenuName(const std::string& menuName);

    } // namespace Inventory
} // namespace Rune