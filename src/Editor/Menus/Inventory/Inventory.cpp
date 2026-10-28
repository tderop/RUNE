#include <Aether/Aether.hpp>

#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>
#include <Editor/EditorWidget.hpp>

#include <Utils/Utils.hpp>
#include <Save/Save.hpp>
#include <Json/Json.hpp>
#include <Config.hpp>
#include <Input/Input.hpp>

#include <Editor/Inventory.hpp>

#include <string>
#include <vector>
#include <map>
#include <cmath>

/**
 * Rune::Inventory::Item Value:
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

std::string formatValue(const std::string& menuName, const Rune::Inventory::Item& inventoryItem){
    // Quantity-based
    for (const std::string& otherMenu : quantityMenus){
        if (otherMenu == menuName){
            return "x"+std::to_string(inventoryItem.value);
        }
    }

    // Equipment
    Rune::Json* equipmentJson;
    Rune::JsonData::GetJsonById(equipmentJson, "EquipmentData");

    auto it = Rune::Inventory::kLabelValueKeys.find(menuName);
    if (it != Rune::Inventory::kLabelValueKeys.end()){
        std::string labelValueKey = it->second;
        auto it = equipmentJson->find(inventoryItem.actorName);

        if (it != equipmentJson->end() && it->is_object()){
            auto labelValue = it->find(labelValueKey);

            if (labelValue->is_number()){
                return std::to_string(labelValue->get<int>());
            } else {
                log("[formatValue (Inventory)] - labelValue is not a number for "+inventoryItem.actorName+"!");
            }
        } else {
            log("[formatValue (Inventory)] - No equipmentJson object found for "+inventoryItem.actorName+"!");
        }
    } else {
        log("[formatValue (Inventory)] - No valueLabelKey found for "+menuName+"!");
    }
    return "";
}

void showTilePage(EditorMenu* menu, std::vector<Rune::Inventory::Item> inventoryItems, size_t page, std::vector<Aether::Ellipse*>* ellipses=nullptr){
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

    Aether::BorderButton* firstBtn = nullptr;

    for (size_t i = cursorStart; i < cursorEnd; ++i){
        // Get the inventory item
        Rune::Inventory::Item& inventoryItem = inventoryItems.at(i);

        // Calculate position of the new tile
        int tileX = startX + (Rune::Inventory::kTileSize+Rune::Inventory::kTilePadding)*((i-cursorStart)%Rune::Inventory::kTilesPerRow);
        int tileY = startY + ((i-cursorStart)/Rune::Inventory::kTilesPerRow)*(Rune::Inventory::kTileSize+Rune::Inventory::kTilePadding);

        // Determine which tile type to use
        inventoryItem.x = tileX;
        inventoryItem.y = tileY;

        // Make inventory tile (use widgets)

        // Assign tile a border button, store it for later if first
        Aether::BorderButton* widgetBtn = new Aether::BorderButton(-1, -1, 1, 1, 4, "", 24, [](){});
        if (firstBtn == nullptr){
            firstBtn = widgetBtn;
        }

        // Build the new tile widget
        Rune::MenuBuilder::AddWidgetFromSaveKey(
            menu,
            Rune::Inventory::kInventoryKey,
            true,
            {
                inventoryItem,
                formatValue(menu->menuName(), inventoryItem),
                widgetBtn
            },
            menu->body()
        );
    }

    // If first button is not null, focus it
    if (firstBtn != nullptr){
        menu->body()->setFocused(firstBtn);
    }

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

void Rune::MenuBuilder::Inventory(EditorMenu* menu){
    log("[Rune::MenuBuilder::Inventory] - Running menu builder...");

    // Disable right stick navigation
    Aether::SetRightStickNavigation(false);

    // Determine if there are inventory keys for the current menu
    auto prefixEntry = Rune::Inventory::kInventoryPrefixes.find(menu->menuName());

    // Do not proceed if no prefixes
    if (prefixEntry == Rune::Inventory::kInventoryPrefixes.end()){
        return;
    }

    log("[Rune::MenuBuilder::Inventory] - Got prefix entry!");

    std::vector<std::string> prefixes = prefixEntry->second;

    // Check for ignores too
    auto ignoresEntry = Rune::Inventory::kInventoryIgnores.find(menu->menuName());
    std::vector<std::string> ignores;

    if (ignoresEntry != Rune::Inventory::kInventoryIgnores.end()){
        ignores = ignoresEntry->second;
    }

    // Get a list of all the valid indexes for this category
    std::vector<Rune::Inventory::Item> inventoryItems;

    Rune::Save::Entry* inventoryEntry = Rune::Save::GetEntryFromDataName(*menu->saveFile(),Rune::Inventory::kInventoryKey);
    auto inventory = std::get<std::vector<std::string>>(inventoryEntry->value);

    // Get the itemValues entry entry
    Rune::Save::Entry* valuesEntry = Rune::Save::GetEntryFromDataName(*menu->saveFile(), Rune::Inventory::kInventoryValueKey);
    auto itemValues = std::get<std::vector<int32_t>>(valuesEntry->value); 

    // Get buffs data
    auto flagSpKeyEntry  = Rune::Inventory::kFlagSpKeys.find(menu->menuName());
    auto valueSpKeyEntry = Rune::Inventory::kValueSpKeys.find(menu->menuName());
    
    Rune::Save::Entry* flagSpEntry;
    Rune::Save::Entry* valueSpEntry;

    std::vector<int32_t> flagSp;
    std::vector<int32_t> valueSp;

    if (flagSpKeyEntry != Rune::Inventory::kFlagSpKeys.end() 
        && valueSpKeyEntry != Rune::Inventory::kValueSpKeys.end())
    {     
        flagSpEntry  = Rune::Save::GetEntryFromDataName(*menu->saveFile(), flagSpKeyEntry->second);
        valueSpEntry = Rune::Save::GetEntryFromDataName(*menu->saveFile(), valueSpKeyEntry->second);

        flagSp = std::get<std::vector<int32_t>>(flagSpEntry->value);
        valueSp = std::get<std::vector<int32_t>>(valueSpEntry->value);
    }

    int currentIdx = -1;
    int localIdx = 0; // this index only increments when a valid slot has been found

    auto tileStyleEntry = Rune::Inventory::kLabelTileStyles.find(menu->menuName());
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
        std::string itemName = getNameFromActorName(actorName);
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
        
        Rune::Inventory::Item inventoryItem{
            actorName, 
            itemName, 
            itemValue, 
            static_cast<size_t>(currentIdx),
            itemFlagSp,
            itemValueSp,
            tileStyle
        };

        inventoryItems.push_back(inventoryItem);
        localIdx++;
    }

    // Pad to fill all available unlocked slots
    // // Show plus button only on this route
    auto it = Rune::Inventory::kSlotKeys.find(menu->menuName());
    if (it != Rune::Inventory::kSlotKeys.end()){
        // Get the slot entry
        Rune::Save::Entry* slotEntry = Rune::Save::GetEntryFromDataName(*menu->saveFile(), it->second);

        // Get the max slots
        int maxUnlockedSlots = std::get<int32_t>(slotEntry->value);

        // Fill with padding slots
        for (int i = (inventoryItems.size()-1); i < (maxUnlockedSlots-1); ++i){
            Rune::Inventory::Item newItem;
            newItem.type = Rune::Inventory::TileType::Empty;

            inventoryItems.push_back(newItem);
        }

        // Get absolute max slots
        auto it = Rune::Inventory::kMaxSlots.find(menu->menuName());
        if (it != Rune::Inventory::kMaxSlots.end()){
            size_t maxSlots = it->second;

            if (inventoryItems.size() < maxSlots){
                Rune::Inventory::Item addItem;
                addItem.type = Rune::Inventory::TileType::Add;

                inventoryItems.push_back(addItem);
            }
        }
        
    } else { // Pad to fill all available slots
        auto it = Rune::Inventory::kMaxSlots.find(menu->menuName());
        if (it != Rune::Inventory::kMaxSlots.end()){
            // Get max possible slots
            int maxSlots = it->second;

            // Add empty slots for every available slot
            for (int i = (inventoryItems.size()-1); i < maxSlots-1; ++i){
                Rune::Inventory::Item newItem;
                newItem.type = Rune::Inventory::TileType::Empty;

                inventoryItems.push_back(newItem);
            }
        }
    }

    // Calculate max pages
    int maxPage = static_cast<float>(inventoryItems.size()-1) / static_cast<float>(Rune::Inventory::kTilesPerPage);
    int* currentPage = new int(0);

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

            // Disable it by default (will be enabled if applicable by passing ellipse vector into showTilePage)
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
    showTilePage(menu, inventoryItems, *currentPage, ellipses);

    // Bind navigation actions
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