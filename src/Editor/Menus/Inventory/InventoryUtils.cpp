#include <Editor/MenuBuilder.hpp>
#include <Editor/EditorMenu.hpp>
#include <Editor/Menus/Inventory.hpp>
#include <Editor/Menus/Inventory/Pouch.hpp>

#include <Json/Json.hpp>
#include <Utils/Logger.hpp>
#include <Config.hpp>

#include <filesystem>
#include <functional>

namespace Rune {
    namespace Inventory {
        std::string GetNameFromActorName(const std::string& actorName){
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

        std::string GetThumbPathFromActorName(const std::string& actorName, bool noVerify){
            // Check cache first
            auto path = IconPathCache->get<std::string>(actorName);

            // If path exists, return it
            if (path != std::nullopt){
                return path.value();
            }

            // Build the full path to the thumbnail asset
            std::string fullPath = THUMB_PATH+actorName+".png";

            // Check if there is a file there, and return an empty string if the file does not exist.
            if (!noVerify){
                if (std::filesystem::exists(fullPath)){
                    // Cache the location
                    IconPathCache->put(actorName, fullPath);
                    return fullPath;
                }
                
                // Return an empty string if path does not exist
                return "";
            } else {
                // Return the generated full path without verification
                return fullPath;
            }
        }


        Category GetCategoryFromMenuName(const std::string& menuName){
            auto it = kMenuNameToCategory.find(menuName);
            if (it == kMenuNameToCategory.end()){
                return Category::Unknown;
            }

            return it->second;
        }

        std::string GetThumbPathFromInventoryItem(const Item& inventoryItem){
            // Mainly for handling dyed armor/elixirs
            std::string iconActorName = inventoryItem.iconActorName;
            if (iconActorName.empty()){
                iconActorName = inventoryItem.actorName;
            }

            int32_t value = inventoryItem.value;

            // Individual category handlers
            switch (inventoryItem.category){
                // Armor - dye colours
                case (Category::Armor): {
                    // Set fileId to actorName, build rest of fileId based on value
                    std::string fileId = iconActorName;

                    // Append the dye ID (e.g. .00 for no dye, .01 for dye, don't append anything if undyeable)
                    if (value != -1){
                        std::stringstream dyeIdentifier;
                        dyeIdentifier << "." << std::setfill('0') << std::setw(2) << value;

                        // Build the image identifier
                        const std::string cacheId = fileId + dyeIdentifier.str();

                        // Check if the path is cached
                        auto cachedPath = IconPathCache->get<std::string>(cacheId);
                        if (cachedPath.has_value()){ // If it is cached, return this value
                            return cachedPath.value();
                        }

                        // Path not cached, build the full path
                        std::string fullPath = THUMB_PATH + cacheId + ".png";

                        // Return the full path only if exists
                        if (!std::filesystem::exists(fullPath)){
                            // Default to standard actor name lookup
                            break;
                        } else {
                            // Cache full path after verification and return it
                            IconPathCache->put(cacheId, fullPath);
                            return fullPath;
                        }
                    }

                    break;
                }

                default:
                    // Default to standard actor name lookup
                    break;
            }

            // Return the original path (not dyeable armor)
            return GetThumbPathFromActorName(iconActorName);
        }

        Rune::Json GetActorObject(const std::string& actorName){
            // Get equipment JSON
            Rune::Json* actorJson = nullptr;
            Rune::JsonData::GetJsonById(actorJson, "ActorData");

            // Search for object in JSON
            auto obj = actorJson->find(actorName);
            if (obj == actorJson->end() || !obj->is_object()){
                return Rune::Json{};
            }

            return obj.value();
        }

        std::string FormatValue(const Item& inventoryItem){
            // Quantity-based
            for (const Category& otherMenu: kQuantityMenus){
                if (otherMenu == inventoryItem.category){
                    return "x"+std::to_string(inventoryItem.value);
                }
            }

            // Equipment
            Rune::Json* actorJson;
            Rune::JsonData::GetJsonById(actorJson, "ActorData");

            auto it = kLabelValueKeys.find(inventoryItem.category);
            if (it != kLabelValueKeys.end()){
                std::string labelValueKey = it->second;
                auto it = actorJson->find(inventoryItem.actorName);

                if (it != actorJson->end() && it->is_object()){
                    auto labelValue = it->find(labelValueKey);

                    if (labelValue != it->end()){
                        if (labelValue->is_number()){
                            // Apply modifier to label (so attack up +10 adds 10 to the label number)
                            return FormatModifier(inventoryItem, labelValue->get<int>());
                        } else {
                            Logger::Write("[formatValue (Inventory)] - labelValue is not a number for "+inventoryItem.actorName+"!");
                        }
                    }
                } else {
                    Logger::Write("[formatValue (Inventory)] - No actorJson object found for "+inventoryItem.actorName+"!");
                }
            } else {
                // Logger::Write("[formatValue (Inventory)] - No valueLabelKey found for "+menuName+"!");
            }
            return "";
        }

        void ShowPage(
            EditorMenu* menu, 
            Inventory::PouchItems* inventoryItems,
            size_t page, 
            std::vector<Aether::Ellipse*>& ellipses){
            // Get menu category from name
            Inventory::Category menuCategory = GetCategoryFromMenuName(menu->menuName());

            // Clear all current widgets
            menu->clearAllWidgets();

            // The menu body, used for positioning tiles
            Aether::Container* menuBody = menu->body();

            // Calculate the start coordinates for the tile grid
            int startX = menuBody->x() + kXStartOffset;
            int startY = menuBody->y() + kYStartOffset;
            
            // Calculate where the cursor should start and end in the list of inventory itemsww
            size_t cursorStart = page*kTilesPerPage;
            size_t cursorEnd   = static_cast<size_t>(
                std::min(
                    static_cast<int>(cursorStart+kTilesPerPage),
                    static_cast<int>(inventoryItems->size())
                )
            );

            for (size_t i = cursorStart; i < cursorEnd; ++i){
                // Get the inventory item
                auto inventoryItem = inventoryItems->at(i);

                // Calculate position of the new tile
                int tileX = startX + (kTileSize+kTilePadding)*((i-cursorStart)%kTilesPerRow);
                int tileY = startY + ((i-cursorStart)/kTilesPerRow)*(kTileSize+kTilePadding);

                // Determine which tile type to use
                inventoryItem->x = tileX;
                inventoryItem->y = tileY;

                // Resolve data only for visible page
                // Load data only if uninitialised
                if (!inventoryItem->_initialised){
                    Rune::Json jsonObj = GetActorObject(inventoryItem->actorName);

                    // using lambdas now because it was getting out of control
                    auto getString = [&](const std::string& key){
                        return GetActorValueFromObject<std::string>(jsonObj, key).value_or("");
                    };

                    auto getInt = [&](const std::string& key){
                        return GetActorValueFromObject<int>(jsonObj, key).value_or(-1);
                    };

                    inventoryItem->iconActorName = getString("IconActorName");
                    inventoryItem->nextRankActorName = getString("NextRank");
                    inventoryItem->prevRankActorName = getString("PrevRank");
                    inventoryItem->attack = getInt("Attack");
                    inventoryItem->guard = getInt("Guard");
                    inventoryItem->durability = getInt("Durability");
                    inventoryItem->defence = getInt("Defence");

                    // Set modifier info
                    inventoryItem->modifier = (Modifier)inventoryItem->flagSp;

                    // Data is stored, initialisation is no longer necessary
                    inventoryItem->_initialised = true;
                }

                // Make inventory tile (use widgets)
                // Assign tile a border button, store it for later if first
                Aether::BorderButton* widgetBtn = new Aether::BorderButton(-1, -1, 1, 1, 4, "", 24, [](){});

                // Build the new tile widget
                Rune::MenuBuilder::AddWidgetFromSaveKey(
                    menu,
                    kInventoryKey,
                    true,
                    {
                        inventoryItem,
                        FormatValue(*inventoryItem),
                        widgetBtn
                    },
                    menu->body()
                );
            }

            // Highlight the right menu dot
            for (size_t i = 0; i < ellipses.size(); ++i){
                if (i != page){
                    ellipses.at(i)->setColour(kDisabledEllipse);
                } else {
                    ellipses.at(i)->setColour(kEnabledEllipse);
                }
            }
        }

        Category GetCategoryFromActorName(const std::string& actorName){
            // Do a cache lookup first
            auto category = ActorToCategoryCache->get<Category>(actorName);
            if (category.has_value()){
                return category.value();
            }

            // Cache miss, find the category manually
            for (const auto& [category, prefixes] : kInventoryPrefixes){
                // Get ignores iterator
                const auto ignoreIt = kInventoryIgnores.find(category);
                const bool hasIgnores = (ignoreIt != kInventoryIgnores.end());

                for (const std::string& prefix : prefixes){
                    // Check if the actorName contains the prefix
                    if (actorName.find(prefix) == std::string::npos){
                        continue;
                    }

                    // If it contains the prefix, check that it is not in ignores
                    if (hasIgnores){
                        bool foundIgnore = false;

                        for (const std::string& ignore : ignoreIt->second){
                            // If it contains an ignored prefix, break and continue
                            if (actorName.find(ignore) != std::string::npos){
                                foundIgnore = true;
                                break;
                            }
                        }

                        if (foundIgnore){
                            continue;
                        }
                    }

                    // Found the category, cache and return it
                    ActorToCategoryCache->put(actorName, category);
                    return category;
                }
            }

            return Category::Unknown;
        }

        std::string GetNextArmorRankActorName(const std::string& actorName){
            auto nextRank = GetActorValue<std::string>(actorName, "NextRank");
            if (nextRank == std::nullopt){
                return "";
            }

            return nextRank.value_or("");
        }

        std::string GetPrevArmorRankActorName(const std::string& actorName){
            auto prevRank = GetActorValue<std::string>(actorName, "PrevRank");
            if (prevRank == std::nullopt){
                return "";
            }

            return prevRank.value_or("");
        }

        bool IsArmorUpgradeable(const std::string& actorName){
            // Check next rank
            std::string rankCheck;
            rankCheck = GetNextArmorRankActorName(actorName);
            if (rankCheck.empty()){ // Check prev rank if no next rank
                rankCheck = GetPrevArmorRankActorName(actorName);
            }

            // If rankCheck is still empty, armor is not upgradeable
            return !rankCheck.empty();
        }

        int GetArmorStarNum(const std::string& actorName){
            auto starNum = GetActorValue<int>(actorName, "StarNum");
            if (starNum == std::nullopt){
                return 0;
            }

            return starNum.value_or(0);
        }

        bool IsModifierEnhanced(const Modifier& modifier){
            uint32_t rawValue = static_cast<uint32_t>(modifier);
            return (rawValue & kEnhancedMask) != 0;
        }

        bool IsArmorDyeable(const std::string& actorName){
            auto prevRank = GetActorValue<bool>(actorName, "Dyeable");
            if (prevRank == std::nullopt){
                return false;
            }

            return prevRank.value_or(false);
        }

        uint32_t GetMaxDurability(const std::string& actorName){
            Rune::Json* actorData = nullptr;
            Rune::JsonData::GetJsonById(actorData, "ActorData");

            if (actorData == nullptr){
                // Logger::Importantf("ActorData is nullptr");
                return 0;
            }

            auto actorIt = actorData->find(actorName);
            if (actorIt == actorData->end()){
                // Logger::Importantf("Actor not found in data");
                return 0;
            }

            auto durIt = actorIt->find("Durability");
            if (durIt == actorIt->end()){
                // Logger::Importantf("Is end or not number");
                return 0;
            }

            return static_cast<uint32_t>(durIt->get<int>());
        }

        // Incredible implementation by yours truly
        bool IsCategoryEquipment(Category c){
            return (c == Category::Weapons) ||
                    (c == Category::Bows) ||
                    (c == Category::Shields);
        }

        Modifier UnenhanceModifier(Modifier m){
            return static_cast<Modifier>(static_cast<uint32_t>(m) & ~kEnhancedMask);
        }

        std::string FormatModifier(Item item, const int value){
            if (item.modifier == Modifier::None 
                || item.modifier == Modifier::Unknown){

                return std::to_string(value);
            }

            // remove 0x80000000
            Modifier modifier = item.modifier;
            if (IsModifierEnhanced(modifier)){
                modifier = UnenhanceModifier(modifier);
            }

            // These modifiers do a simple add to the label value
            if (modifier == Modifier::AttackUp || modifier == Modifier::GuardUp){
                return std::to_string(value+item.valueSp);
            }
            
            // Default to the input value if could not format
            return std::to_string(value);
        }

        bool IsActorArrow(std::string_view actorName){
            return (actorName.find(kArrowPrefix) != std::string::npos);
        }

        std::string GetFlagSpKeyFromCategory(Category category){
            auto it = kFlagSpKeys.find(category);
            if (it == kFlagSpKeys.end()){
                return "";
            }

            return it->second;
        }

        std::string GetValueSpKeyFromCategory(Category category){
            auto it = kValueSpKeys.find(category);
            if (it == kValueSpKeys.end()){
                return "";
            }

            return it->second;
        }

    int GetMaxSlotsFromCategory(Category category){
            auto it = kMaxSlots.find(category);
            if (it == kMaxSlots.end()){
                return -1;
            }

            return it->second;
        }
    } // namespace Inventory
} // namespace Rune