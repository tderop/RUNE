#include <Aether/Aether.hpp>

#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>
#include <Editor/EditorWidget.hpp>

#include <Utils/Utils.hpp>
#include <Utils/Logger.hpp>
#include <Save/Save.hpp>
#include <Json/Json.hpp>
#include <Config.hpp>
#include <Input/Input.hpp>

#include <Editor/Inventory.hpp>

#include <algorithm>
#include <string>
#include <vector>
#include <map>

/**
 * Rune::Inventory::Item Value:
 * - Weapons (Swords, Shields, Bows): Durability (MAX __INT32_MAX__) * 100 (why x100 I do not know, f32 support without f32 array?)
 * - Materials, Food, Key Items: Quantity (MAX 999)
 * - Armor: Dye Color (0 = undyed, -1 = not dyeable, 15 = max dye color - needs more RE)
 */
void Rune::MenuBuilder::Inventory(EditorMenu* menu){
    Logger::Write("[Rune::MenuBuilder::Inventory] - Running menu builder...");

    // Disable right stick navigation
    Aether::SetRightStickNavigation(false);

    // Get the menu category
    Rune::Inventory::Category menuCategory = Rune::Inventory::GetCategoryFromMenuName(menu->menuName());

    // Determine if there are inventory keys for the current menu
    auto prefixEntry = Rune::Inventory::kInventoryPrefixes.find(menuCategory);

    // Do not proceed if no prefixes
    if (prefixEntry == Rune::Inventory::kInventoryPrefixes.end()){
        return;
    }

    Logger::Write("[Rune::MenuBuilder::Inventory] - Got prefix entry!");

    std::vector<std::string> prefixes = prefixEntry->second;

    // Check for ignores too
    auto ignoresEntry = Rune::Inventory::kInventoryIgnores.find(menuCategory);
    std::vector<std::string> ignores;
    
    if (ignoresEntry != Rune::Inventory::kInventoryIgnores.end()){
        ignores = ignoresEntry->second;
    }

    // Get a list of all the valid indexes for this category
    auto inventoryItems = std::make_shared<std::vector<Rune::Inventory::Item>>();

    Rune::Save::Entry* inventoryEntry = Rune::Save::GetEntryFromDataName(*menu->saveFile(),Rune::Inventory::kInventoryKey);
    auto inventory = std::get<std::vector<std::string>>(inventoryEntry->value);

    // Get the itemValues entry entry
    Rune::Save::Entry* valuesEntry = Rune::Save::GetEntryFromDataName(*menu->saveFile(), Rune::Inventory::kInventoryValueKey);
    auto itemValues = std::get<std::vector<int32_t>>(valuesEntry->value); 

    // Get buffs data
    auto flagSpKeyEntry  = Rune::Inventory::kFlagSpKeys.find(menuCategory);
    auto valueSpKeyEntry = Rune::Inventory::kValueSpKeys.find(menuCategory);
    
    std::string flagSpKey;
    std::string valueSpKey;

    Rune::Save::Entry* flagSpEntry;
    Rune::Save::Entry* valueSpEntry;

    std::vector<int32_t> flagSp;
    std::vector<int32_t> valueSp;

    if (flagSpKeyEntry != Rune::Inventory::kFlagSpKeys.end() 
        && valueSpKeyEntry != Rune::Inventory::kValueSpKeys.end())
    {     
        flagSpKey  = flagSpKeyEntry->second;
        valueSpKey = valueSpKeyEntry->second;

        flagSpEntry  = Rune::Save::GetEntryFromDataName(*menu->saveFile(), flagSpKeyEntry->second);
        valueSpEntry = Rune::Save::GetEntryFromDataName(*menu->saveFile(), valueSpKeyEntry->second);

        flagSp = std::get<std::vector<int32_t>>(flagSpEntry->value);
        valueSp = std::get<std::vector<int32_t>>(valueSpEntry->value);
    }

    int currentIdx = -1;
    int localIdx = 0; // this index only increments when a valid slot has been found

    auto tileStyleEntry = Rune::Inventory::kLabelTileStyles.find(menuCategory);
    Rune::Inventory::TileStyle tileStyle = Rune::Inventory::TileStyle::NoLabel;

    // unpack style pair
    if (tileStyleEntry != Rune::Inventory::kLabelTileStyles.end()){
        tileStyle = tileStyleEntry->second;
    }

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
        std::string itemName = Rune::Inventory::GetNameFromActorName(actorName);
        int32_t itemValue = itemValues.at(currentIdx);

        // Get buffs
        int32_t itemFlagSp = 0;
        int32_t itemValueSp = 0;

        // Check that there are buffs for this inventory type and apply them
        if (!flagSp.empty() && static_cast<int>(flagSp.size()) >= localIdx+1){
            itemFlagSp = flagSp.at(localIdx);
        }

        if (!valueSp.empty() && static_cast<int>(valueSp.size()) >= localIdx+1){
            itemValueSp = valueSp.at(localIdx);
        }

        // Make a new inventory item object representing the item in Link's pouch
        Rune::Inventory::Item inventoryItem{
            actorName, 
            itemName, 
            itemValue, 
            static_cast<size_t>(currentIdx),
            static_cast<size_t>(localIdx),
            Rune::Inventory::GetCategoryFromMenuName(menu->menuName()),
            itemFlagSp,
            itemValueSp,
            tileStyle
        };

        // Add Sp keys
        inventoryItem.flagSpKey  = flagSpKey;
        inventoryItem.valueSpKey = valueSpKey;

        // Store the inventory item
        inventoryItems->push_back(inventoryItem);

        // Increment the local index as the value was successfully parsed
        localIdx++;
    }

    // An empty inventory slot template for this category
    Rune::Inventory::Item emptySlot{"", "", 0, 0, 
        0, menuCategory, 0, 0, tileStyle,
        Rune::Inventory::TileType::Empty, ""};
    
    // If there are available flagSp/valueSp keys, then set them
    emptySlot.flagSpKey = flagSpKey;
    emptySlot.valueSpKey = valueSpKey;

    // Find the first empty slot in PorchItem
    auto emptyIt = std::find_if(inventory.begin(), inventory.end(),
        [](const std::string& s){ return s.empty(); });

    // Currently, if no value is found, this could be very destructive
    // Additionally there are no checks to ensure the vector does not go beyond a size of 420 too
    // so this could result in save file corruption if handled wrong
    bool foundSafeIdx=false;
    if (emptyIt != inventory.end()){
        foundSafeIdx=true;
        currentIdx = std::distance(inventory.begin(), emptyIt);
        // Logger::Importantf("Actor name at %d: %s", currentIdx, inventory.at(currentIdx).c_str());
    }

    // Pad to fill all available unlocked slots
    // // Show plus button only on this route
    auto it = Rune::Inventory::kSlotKeys.find(menuCategory);

    // In this case, the menu is upgradeable
    if (it != Rune::Inventory::kSlotKeys.end() && foundSafeIdx){
        // Get the max slots unlocked from the save file
        Rune::Save::Entry* slotEntry = Rune::Save::GetEntryFromDataName(*menu->saveFile(), it->second);
        int maxUnlockedSlots = std::get<int32_t>(slotEntry->value);

        // Fill with padding slots up to unlocked max
        // Break out if no safe index was found to prevent data corruption
        for (int i = (inventoryItems->size()-1); i < (maxUnlockedSlots-1) && foundSafeIdx; ++i){
            // Copy the empty slot template and update index values
            Rune::Inventory::Item newItem = emptySlot;
            newItem.index = currentIdx;
            newItem.localIndex = localIdx;

            inventoryItems->push_back(newItem);

            // Keep incrementing currentIdx until another valid slot is found
            foundSafeIdx = false;
            for (int j = ++currentIdx; j < Rune::Inventory::kInventoryMaxCount; ++j){
                // Safe index to overwrite was found, break
                if (inventory.at(j).empty()){
                    currentIdx = j;
                    foundSafeIdx=true;

                    break;
                }
            }

            // Increment local index, guaranteed to be safe due to hard cap on max slots
            localIdx++;
        }

        // If the number of available slots is not the maximum and safe slots are still available, create an "add"
        // tile so the user can add another slot
        auto it = Rune::Inventory::kMaxSlots.find(menuCategory);

        if (it != Rune::Inventory::kMaxSlots.end() && foundSafeIdx){
            size_t maxSlots = it->second;

            if (inventoryItems->size() < maxSlots){
                Rune::Inventory::Item addItem;
                addItem.category = menuCategory;
                addItem.type = Rune::Inventory::TileType::Add;

                inventoryItems->push_back(addItem);
            }
        }
        
    } else { // Pad to fill all available slots
        auto it = Rune::Inventory::kMaxSlots.find(menuCategory);
        if (it != Rune::Inventory::kMaxSlots.end() && foundSafeIdx){
            // Get max possible slots
            int maxSlots = it->second;

            // Add empty slots for every available slot
            // Do not continue if no more safe slots
            for (int i = (inventoryItems->size()-1); i < maxSlots-1 && foundSafeIdx; ++i){
                Rune::Inventory::Item newItem = emptySlot;
                newItem.index = currentIdx;
                inventoryItems->push_back(newItem);

                // Search for new valid slot
                foundSafeIdx = false;
                for (int j = ++currentIdx; j < Rune::Inventory::kInventoryMaxCount; ++j){
                    if (inventory.at(j).empty()){
                        currentIdx = j;
                        foundSafeIdx = true;

                        break;
                    }
                }
            }

        }
    }

    // Calculate max pages
    int maxPage = static_cast<float>(inventoryItems->size()-1) / static_cast<float>(Rune::Inventory::kTilesPerPage);
    auto currentPage = std::make_shared<int>(0);

    // The dots showing which page is currently being viewed
    std::vector<Aether::Ellipse*>* ellipses = new std::vector<Aether::Ellipse*>;

    // Do not show navigation dot when there is only one page
    if (maxPage > 0){
        for (int i = 0; i <= maxPage; ++i){
            // Create a new circle
            Aether::Ellipse* menuDot = new Aether::Ellipse(
                0,
                menu->body()->y()+Rune::Inventory::kYStartOffset - 18, 
                Rune::Inventory::kDotWidth
            );

            // Disable it by default (will be enabled if applicable by passing ellipse vector into Rune::Inventory::ShowPage)
            menuDot->setColour(Rune::Inventory::kDisabledEllipse);
            
            // The body owns the dot
            menu->body()->addElement(menuDot);

            // Store the new circle
            ellipses->push_back(menuDot);
        }
    }

    // Calculate values for dot repositioning
    float centerPosition = (menu->body()->x() + Rune::Inventory::kXStartOffset + (Rune::Inventory::kGridWidth/2));
    int dotStride = Rune::Inventory::kDotWidth + Rune::Inventory::kDotPadding;
    int dotStart = centerPosition - (((ellipses->size())-1)*(dotStride))/2.0f;

    // Reposition menu dots
    for (int i = 0; i < static_cast<int>(ellipses->size()); ++i){
        Aether::Ellipse* menuDot = ellipses->at(i);
        menuDot->setX(dotStart + (i*dotStride));
    }

    // Show the first page
    Rune::Inventory::ShowPage(menu, *inventoryItems, *currentPage, ellipses);

    // Bind navigation actions
    InputSystem& inputService = InputSystem::getService();
    inputService.bind("NextPage", InputObject(InputType::Gamepad, HidNpadButton_ZR));
    inputService.bind("PrevPage", InputObject(InputType::Gamepad, HidNpadButton_ZL));

    inputService.connect("NextPage", [menu, inventoryItems, currentPage, maxPage, ellipses](InputSystem& svc, InputState state){
        if (state != InputState::Begin || maxPage == 0){
            return;
        }

        // Change current page, wrap around if at max page
        if (*currentPage >= maxPage){
            *currentPage = 0;
        } else {
            *currentPage += 1;
        }

        Rune::Inventory::ShowPage(menu, *inventoryItems, static_cast<size_t>(*currentPage), ellipses);
    });

    inputService.connect("PrevPage", [menu, inventoryItems, currentPage, maxPage, ellipses](InputSystem& svc, InputState state){
        if (state != InputState::Begin || maxPage == 0){
            return;
        }

        if (*currentPage <= 0){
            *currentPage = maxPage;
        } else {
            *currentPage -= 1;
        }

        Rune::Inventory::ShowPage(menu, *inventoryItems, static_cast<size_t>(*currentPage), ellipses);
    });

    menu->giveTask([ellipses](){
        // Unbind pagination actions
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