#include <Editor/MenuBuilder.hpp>
#include <Editor/EditorMenu.hpp>
#include <Editor/Inventory.hpp>

#include <Json/Json.hpp>
#include <Utils/Logger.hpp>
#include <Config.hpp>

#include <filesystem>
#include <functional>

std::string Rune::Inventory::GetNameFromActorName(const std::string& actorName){
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

std::string Rune::Inventory::GetThumbPathFromActorName(const std::string& actorName, bool noVerify){
    // Check cache first
    auto path = Rune::Inventory::IconPathCache->get<std::string>(actorName);

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
            Rune::Inventory::IconPathCache->put(actorName, fullPath);
            return fullPath;
        }
        
        // Return an empty string if path does not exist
        return "";
    } else {
        // Return the generated full path without verification
        return fullPath;
    }
}


Rune::Inventory::Category Rune::Inventory::GetCategoryFromMenuName(const std::string& menuName){
    auto it = Rune::Inventory::kMenuNameToCategory.find(menuName);
    if (it == Rune::Inventory::kMenuNameToCategory.end()){
        return Rune::Inventory::Category::Unknown;
    }

    return it->second;
}

std::string Rune::Inventory::GetThumbPathFromInventoryItem(const Rune::Inventory::Item& inventoryItem){
    // Mainly for handling dyed armor/elixirs
    std::string iconActorName = inventoryItem.iconActorName;
    if (iconActorName.empty()){
        iconActorName = inventoryItem.actorName;
    }

    // Check if path is cached
    auto path = Rune::Inventory::IconPathCache->get<std::string>(iconActorName);
    if (path != std::nullopt){
        return path.value();
    }

    int32_t value = inventoryItem.value;

    // Individual category handlers
    switch (inventoryItem.category){
        // Armor - dye colours
        case (Rune::Inventory::Category::Armor): {
            // Set fileId to actorName, build rest of fileId based on value
            std::string fileId = iconActorName;

            // Append the dye ID (e.g. .00 for no dye, .01 for dye, don't append anything if undyeable)
            if (value != -1){
                std::stringstream dyeIdentifier;
                dyeIdentifier << "." << std::setfill('0') << std::setw(2) << value;

                // Build the full image path
                std::string fullPath = THUMB_PATH + fileId + dyeIdentifier.str() + ".png";

                // Return the full path only if exists
                if (!std::filesystem::exists(fullPath)){
                    std::string fallbackPath = Rune::Inventory::GetThumbPathFromActorName(iconActorName);
                    // Logger::Write("[Rune::Inventory] - File does not exist! '"+fullPath+"... using '"+fallbackPath+"'", true);
                    return fallbackPath;
                } else {
                    // Logger::Write("[Rune::Inventory] - File exists! '"+fullPath+"'", true);
                    return fullPath;
                }
            }

            // Return the original path (not dyeable armor)
            return Rune::Inventory::GetThumbPathFromActorName(iconActorName);
        }

        default:
            return Rune::Inventory::GetThumbPathFromActorName(iconActorName);
    }


    return "";
}

Rune::Json Rune::Inventory::GetActorObject(const std::string& actorName){
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

std::string Rune::Inventory::FormatValue(const Rune::Inventory::Item& inventoryItem){
    // Quantity-based
    for (const Rune::Inventory::Category& otherMenu: Rune::Inventory::kQuantityMenus){
        if (otherMenu == inventoryItem.category){
            return "x"+std::to_string(inventoryItem.value);
        }
    }

    // Equipment
    Rune::Json* actorJson;
    Rune::JsonData::GetJsonById(actorJson, "ActorData");

    auto it = Rune::Inventory::kLabelValueKeys.find(inventoryItem.category);
    if (it != Rune::Inventory::kLabelValueKeys.end()){
        std::string labelValueKey = it->second;
        auto it = actorJson->find(inventoryItem.actorName);

        if (it != actorJson->end() && it->is_object()){
            auto labelValue = it->find(labelValueKey);

            if (labelValue != it->end()){
                if (labelValue->is_number()){
                    return std::to_string(labelValue->get<int>());
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

void Rune::Inventory::ShowPage(
    EditorMenu* menu, 
    std::vector<Rune::Inventory::Item>& inventoryItems,
    size_t page, 
    std::vector<Aether::Ellipse*>* ellipses){
    
    // Get menu category from name
    Inventory::Category menuCategory = Rune::Inventory::GetCategoryFromMenuName(menu->menuName());

    // Clear all current widgets
    menu->clearAllWidgets();

    // The menu body, used for positioning tiles
    Aether::Container* menuBody = menu->body();

    int startX = menuBody->x() + Rune::Inventory::kXStartOffset;
    int startY = menuBody->y() + Rune::Inventory::kYStartOffset;

    size_t cursorStart = page*Rune::Inventory::kTilesPerPage;
    size_t cursorEnd   = static_cast<size_t>(
        std::min(
            static_cast<int>(cursorStart+Rune::Inventory::kTilesPerPage),
            static_cast<int>(inventoryItems.size())
        )
    );

    // For shifting equipment inventories
    int offsetX = 0;
    // if (IsCategoryEquipment(menuCategory)){ offsetX = 52; }

    for (size_t i = cursorStart; i < cursorEnd; ++i){
        // Get the inventory item
        Rune::Inventory::Item& inventoryItem = inventoryItems.at(i);

        // Calculate position of the new tile
        int tileX = startX + (Rune::Inventory::kTileSize+Rune::Inventory::kTilePadding)*((i-cursorStart)%Rune::Inventory::kTilesPerRow) + offsetX;
        int tileY = startY + ((i-cursorStart)/Rune::Inventory::kTilesPerRow)*(Rune::Inventory::kTileSize+Rune::Inventory::kTilePadding);

        // Determine which tile type to use
        inventoryItem.x = tileX;
        inventoryItem.y = tileY;

        // Resolve data only for visible page
        // Load data only if uninitialised
        if (!inventoryItem._initialised){
            Rune::Json jsonObj = Rune::Inventory::GetActorObject(inventoryItem.actorName);

            // using lambdas now because it was getting out of control
            auto getString = [&](const std::string& key){
                return Rune::Inventory::GetActorValueFromObject<std::string>(jsonObj, key).value_or("");
            };

            auto getInt = [&](const std::string& key){
                return Rune::Inventory::GetActorValueFromObject<int>(jsonObj, key).value_or(-1);
            };

            inventoryItem.iconActorName = getString("IconActorName");
            inventoryItem.nextRankActorName = getString("NextRank");
            inventoryItem.prevRankActorName = getString("PrevRank");
            inventoryItem.attack = getInt("Attack");
            inventoryItem.guard = getInt("Guard");
            inventoryItem.durability = getInt("Durability");
            inventoryItem.defence = getInt("Defence");

            // Set modifier info
            inventoryItem.modifier = (Rune::Inventory::Modifier)inventoryItem.flagSp;

            // Set thumbnail
            inventoryItem.thumbPath = Rune::Inventory::GetThumbPathFromInventoryItem(inventoryItem);

            // Data is stored, initialisation is no longer necessary
            inventoryItem._initialised = true;
        }

        // Make inventory tile (use widgets)

        // Assign tile a border button, store it for later if first
        Aether::BorderButton* widgetBtn = new Aether::BorderButton(-1, -1, 1, 1, 4, "", 24, [](){});
        // if (firstBtn == nullptr){
        //     firstBtn = widgetBtn;
        // }

        // Build the new tile widget
        Rune::MenuBuilder::AddWidgetFromSaveKey(
            menu,
            Rune::Inventory::kInventoryKey,
            true,
            {
                inventoryItem,
                Rune::Inventory::FormatValue(inventoryItem),
                widgetBtn
            },
            menu->body()
        );
    }

    // If first button is not null, focus it
    // if (firstBtn != nullptr){
    //     Aether::Element* focus = firstBtn;

    //     // Iterate through the Aether element heirarchy to ensure correct focussing
    //     while (focus != nullptr && focus->parent() != nullptr) {
    //         Aether::Element* parent = focus->parent();
    //         parent->setFocused(focus);
    //         focus = parent;
    //     }
    // }

    // Highlight the right menu dot
    if (ellipses != nullptr){
        for (size_t i = 0; i < ellipses->size(); ++i){
            if (i != page){
                ellipses->at(i)->setColour(Rune::Inventory::kDisabledEllipse);
            } else {
                ellipses->at(i)->setColour(Rune::Inventory::kEnabledEllipse);
            }
        }
    }
}

Result Rune::Inventory::PreloadTextures(
    Aether::Renderer* renderer, 
    std::function<void(const std::string&,std::unique_ptr<Aether::Drawable>)> onDecoded, 
    std::function<void(size_t, size_t)> onProgress){

    // return 0;

    // Return if directory does not exist
    if (!std::filesystem::exists(THUMB_PATH)){
        return LibnxError_NotFound;
    }

    // Iterate through and cache textures
    size_t progress = 0;
    size_t maxProgress = std::distance(
        std::filesystem::directory_iterator(THUMB_PATH),
        std::filesystem::directory_iterator{}
    ); 

    int thumbSize = static_cast<int>(Rune::Inventory::kTileSize*Rune::Inventory::kThumbScale);
    std::string suffix = "|"+std::to_string(thumbSize)+"x"+std::to_string(thumbSize);

    for (const auto& entry : std::filesystem::directory_iterator(THUMB_PATH)){
        // Progress
        onProgress(++progress, maxProgress);
        
        const std::string path = entry.path().string();
        const std::string key = path+suffix;

        // Get the drawable after rendering the image surface
        std::unique_ptr<Aether::Drawable> drawable(renderer->renderImageSurface(path, thumbSize, thumbSize));

        // If bad drawable, return
        if (!drawable || drawable->type() != Aether::Drawable::Type::Surface){
            return LibnxError_IoError;
        }

        // On decoded, hand graphics work to main thread
        onDecoded(key, std::move(drawable));
    }

    return 0;
}

Result Rune::Inventory::CacheIconPaths(){
    Rune::Json actorNameJson;

    return 0;
}

Rune::Inventory::Category Rune::Inventory::GetCategoryFromActorName(const std::string& actorName){
    // Do a cache lookup first
    auto category = ActorToCategoryCache->get<Rune::Inventory::Category>(actorName);
    if (category.has_value()){
        return category.value();
    }

    // Cache miss, find the category manually
    for (const auto& [category, prefixes] : Rune::Inventory::kInventoryPrefixes){
        // Get ignores iterator
        const auto ignoreIt = Rune::Inventory::kInventoryIgnores.find(category);
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

    return Rune::Inventory::Category::Unknown;
}

std::string Rune::Inventory::GetNextArmorRankActorName(const std::string& actorName){
    auto nextRank = Rune::Inventory::GetActorValue<std::string>(actorName, "NextRank");
    if (nextRank == std::nullopt){
        return "";
    }

    return nextRank.value_or("");
}

std::string Rune::Inventory::GetPrevArmorRankActorName(const std::string& actorName){
    auto prevRank = Rune::Inventory::GetActorValue<std::string>(actorName, "PrevRank");
    if (prevRank == std::nullopt){
        return "";
    }

    return prevRank.value_or("");
}

bool Rune::Inventory::IsArmorUpgradeable(const std::string& actorName){
    // Check next rank
    std::string rankCheck;
    rankCheck = Rune::Inventory::GetNextArmorRankActorName(actorName);
    if (rankCheck.empty()){ // Check prev rank if no next rank
        rankCheck = Rune::Inventory::GetPrevArmorRankActorName(actorName);
    }

    // If rankCheck is still empty, armor is not upgradeable
    return !rankCheck.empty();
}

int Rune::Inventory::GetArmorStarNum(const std::string& actorName){
    auto starNum = Rune::Inventory::GetActorValue<int>(actorName, "StarNum");
    if (starNum == std::nullopt){
        return 0;
    }

    return starNum.value_or(0);
}

bool Rune::Inventory::IsModifierEnhanced(const Modifier& modifier){
    uint32_t rawValue = static_cast<uint32_t>(modifier);
    return (rawValue & Rune::Inventory::kEnhancedMask) != 0;
}

bool Rune::Inventory::IsArmorDyeable(const std::string& actorName){
    auto prevRank = Rune::Inventory::GetActorValue<bool>(actorName, "Dyeable");
    if (prevRank == std::nullopt){
        return false;
    }

    return prevRank.value_or(false);
}

uint32_t Rune::Inventory::GetMaxDurability(const std::string& actorName){
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
bool Rune::Inventory::IsCategoryEquipment(Rune::Inventory::Category c){
    return (c == Rune::Inventory::Category::Weapons) ||
            (c == Rune::Inventory::Category::Bows) ||
            (c == Rune::Inventory::Category::Shields);
}