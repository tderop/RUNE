#pragma once

#include <Aether/Aether.hpp>
#include <string>
#include <map>

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

        // All of the items in Link's pouch are merged into one massive array of max length 420. This is going to be a little annoying.
        inline const std::string kInventoryKey       = "PorchItem";
        inline const std::string kInventoryValueKey  = "PorchItem_Value1";
        inline const std::string kInventoryEquipFlag = "PorchItem_EquipFlag";

        extern const std::map<std::string, std::string> kFlagSpKeys;
        extern const std::map<std::string, std::string> kSlotKeys;
        extern const std::map<std::string, std::string> kValueSpKeys;
        extern const std::map<std::string, size_t> kMaxSlots;

        extern const std::map<std::string, std::vector<std::string>> kInventoryPrefixes;
        extern const std::map<std::string, std::vector<std::string>> kInventoryIgnores;

        extern const std::map<std::string, std::string> kLabelValueKeys;

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

        extern const std::map<std::string, Rune::Inventory::TileStyle> kLabelTileStyles;

        struct Item{
            std::string actorName;
            std::string name;
            int32_t value;
            size_t index;

            int32_t flagSp=0; // buff
            int32_t valueSp=0; // buff value

            TileStyle style=TileStyle::NoLabel;
            TileType type=TileType::Standard;

            int x=0;
            int y=0;
            int w=kTileSize;
            int h=kTileSize;

            Item(
                std::string _actorName, 
                std::string _name, 
                int32_t _value, 
                size_t _index,
                int32_t _flagSp=0,
                int32_t _valueSp=0,
                TileStyle _style=TileStyle::NoLabel,
                TileType _type=TileType::Standard
            ) : actorName(_actorName), name(_name), value(_value), index(_index), flagSp(_flagSp), 
                valueSp(_valueSp), style(_style), type(_type) {}

            Item() = default;
        };

        std::string getThumbPathFromActorName(const std::string& actorName);
        std::string getNameFromActorName(const std::string& actorName);
    } // namespace Inventory
} // namespace Rune