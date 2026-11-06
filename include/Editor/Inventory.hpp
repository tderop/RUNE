#pragma once

#include <Aether/Aether.hpp>
#include <any>
#include <optional>
#include <functional>
#include <string>
#include <map>

#include <Utils/Cache.hpp>
#include <Json/Json.hpp>

#include <Editor/MenuBuilder.hpp>
#include <Editor/EditorMenu.hpp>

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

        inline Cache* IconPathCache = new Cache(420);
        inline Cache* actorCategoryCache = new Cache(420);
        inline Cache* IconTextureCache = new Cache(1559);

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

        inline const uint32_t kEnhancedMask = 0x80000000;
        enum class Modifier : uint32_t{
            AttackUp=0x1,
            DurabilityUp=0x2,
            CriticalHit=0x4,
            LongThrow=0x8,
            MultiShot=0x10,
            Zoom=0x20,
            QuickShot=0x40,
            SurfMaster=0x80,

            // Half of these gold variants I'm psure don't even exist but we'll see
            AttackUpPlus=AttackUp|kEnhancedMask,
            DurabilityUpPlus=DurabilityUp|kEnhancedMask,
            CriticalHitPlus=CriticalHit|kEnhancedMask,
            LongThrowPlus=LongThrow|kEnhancedMask,
            MultiShotPlus=MultiShot|kEnhancedMask,
            ZoomPlus=Zoom|kEnhancedMask,
            QuickShotPlus=QuickShot|kEnhancedMask,
            SurfMasterPlus=SurfMaster|kEnhancedMask,

            None=0,
            Unknown=0xFFFFFFFF
        };

        struct ModifierInfo {
            Modifier modifier;
            const char* name;
        };

        inline constexpr ModifierInfo modifierNames[] = {
            {Modifier::AttackUp, "Attack Up"},
            {Modifier::DurabilityUp, "Durability Up"},
            {Modifier::CriticalHit, "Critical Hit"},
            {Modifier::LongThrow, "Long Throw"},
            {Modifier::MultiShot, "Multi Shot"},
            {Modifier::Zoom, "Zoom"},
            {Modifier::QuickShot, "Quick Shot"},
            {Modifier::SurfMaster, "Surf Master"},
            {Modifier::None, "None"},
            {Modifier::Unknown, "Unknown"}
        };

        /**
         * @brief Get a modifier's name from its modifier Enum
         * @param Modifier the modifier Enum to evaluate
         * @return name string
         */
        inline std::string ModifierName(Modifier modifier){
            // Strip of the enhanced mask to get raw modifier
            uint32_t rawModifier = static_cast<uint32_t>(modifier) & ~kEnhancedMask;

            for (const auto& info : modifierNames){
                if (static_cast<uint32_t>(info.modifier) == rawModifier){
                    // Add 'Plus' to the name if it contains the enhanced mask
                    return std::string(info.name) 
                    + ((static_cast<uint32_t>(modifier) & kEnhancedMask) ? "+" : "");
                }
            }

            return "unknown";
        }

        // All of the items in Link's pouch are merged into one massive array of max length 420. This is going to be a little annoying.
        inline const std::string kInventoryKey       = "PorchItem";
        inline const std::string kInventoryValueKey  = "PorchItem_Value1";
        inline const std::string kInventoryEquipFlag = "PorchItem_EquipFlag";

        // The value by which durability is multiplied by when it is saved
        inline const int kDurabilityMultiplier = 100;
        inline const int kMaxInventoryQuantity = 999;
        inline const int kMaxDyeColour = 15;

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
            size_t localIndex;

            Category category;

            int32_t flagSp=0; // buff
            int32_t valueSp=0; // buff value

            Modifier modifier=Modifier::None;

            TileStyle style=TileStyle::NoLabel;
            TileType type=TileType::Standard;

            std::string iconActorName;
            std::string thumbPath;

            std::string nextRankActorName;
            std::string prevRankActorName;

            std::string flagSpKey;
            std::string valueSpKey;

            int attack=-1; // Attack damage (weapons and bows)
            int guard=-1; // Shield guard (shields only)
            int durability=-1; // Durability (weapons, bows, and shields)
            int defence=-1; // Attack resistance (armor only)

            int x=0;
            int y=0;
            int w=kTileSize;
            int h=kTileSize;

            bool _initialised=false;

            Item(
                std::string _actorName, 
                std::string _name, 
                int32_t _value, 
                size_t _index,
                size_t _localIndex,
                Category _category,
                int32_t _flagSp=0,
                int32_t _valueSp=0,
                TileStyle _style=TileStyle::NoLabel,
                TileType _type=TileType::Standard,
                std::string _thumbPath=""
            ) : actorName(_actorName), name(_name), value(_value), index(_index), localIndex(_localIndex), category(_category),
                flagSp(_flagSp), valueSp(_valueSp), style(_style), type(_type), thumbPath(_thumbPath) {}

            Item() = default;
        };

        std::string GetThumbPathFromActorName(const std::string& actorName, bool noVerify=false);
        std::string GetThumbPathFromInventoryItem(const Item& inventoryItem);
        std::string GetNameFromActorName(const std::string& actorName);
        Category GetCategoryFromMenuName(const std::string& menuName);
        
        template <typename T>
        std::optional<T> GetEquipmentValue(const std::string& actorName, const std::string& key){
            // Get equipment json
            Rune::Json* equipmentJson = nullptr;
            Rune::JsonData::GetJsonById(equipmentJson, "EquipmentData");

            // Find object
            auto obj = equipmentJson->find(actorName);
            if (obj == equipmentJson->end() || !obj->is_object()){
                return std::nullopt;
            }

            // Find value
            auto it = obj->find(key);
            if (it == obj->end()){
                return std::nullopt;
            }

            // Return value
            return it->get<T>();
        }

        /**
         * @brief Get the Rune::Json object for equipment from its actor name. Returns an empty object if no object was found.
         * @param actorName actor name of the equipment
         */
        Rune::Json GetEquipmentObject(const std::string& actorName);

        /**
         * @brief Get a value safely from a JSON object
         * @param json the JSON to search
         * @param key the key to look for
         */
        template <typename T>
        std::optional<T> GetEquipmentValueFromObject(const Rune::Json& json, const std::string& key){
            // Find the value
            auto it = json.find(key);
            if (it == json.end()){
                return std::nullopt;
            }

            // Return the value
            return it->get<T>();
        }

        std::string FormatValue(const Rune::Inventory::Item& inventoryItem);
        void ShowPage(EditorMenu* menu, std::vector<Rune::Inventory::Item>& inventoryItems, size_t page, std::vector<Aether::Ellipse*>* ellipses=nullptr);

        /**
         * @brief Render all the icon textures. Fails if no textures exist.
         * @param progress Function to run for every progress iteration
         * @return Error code
         */
        Result PreloadTextures(Aether::Renderer* renderer, std::function<void(const std::string&,std::unique_ptr<Aether::Drawable>)> onDecoded, std::function<void(size_t, size_t)> onProgress=[](size_t, size_t){});

        /**
         * @brief Load all actorname icon paths. 
         * @return Error code
         */
        Result CacheIconPaths();

        /**
         * @brief Determine an item's category from its actor name
         * @note About 99.9% accurate
         * @param actorName The item's actor name
         * @return The Rune::Inventory::Category of the actor
         */
        Category GetCategoryFromActorName(const std::string& actorName);
    } // namespace Inventory
} // namespace Rune