#include <switch.h>
#include <Aether/Aether.hpp>

#include <Editor/EditorMenu.hpp>
#include <Editor/EditorWidget.hpp>

#include <Utils/Utils.hpp>
#include <Utils/Logger.hpp>
#include <Config.hpp>

#include <Editor/Inventory.hpp>
#include <Application/Keyboard.hpp>

#include <memory>
#include <string>
#include <any>

// Tile-specific constants
const bool SHOW_AFTER_RENDER = false;

const int PROPERTY_EDITOR_SPACING = 16; // 16px between value steppers/dropdowns/inputs etc
const int ARMOR_STAR_SIZE = 22; // The font size of the armor stars
const int BUFF_STAR_SIZE = 18; // The size of the buff indicator star for equipment tiles

const Aether::Colour ENHANCED_COLOUR{255, 255, 0, 255};

// I'm fed up of writing Rune:: icl
using namespace Rune;

void WidgetBuilder::Tile(EditorWidget* widget){
    // Check if meta was provided
    if (widget->getMeta().empty()){
        Logger::Write("[WidgetBuilder::Tile] - No widget metadata provided!");
        return;
    }

    EditorMenu* menu = widget->getMenu();

    // Unpack the meta
    std::vector<std::any> widgetMeta = widget->getMeta();
    Inventory::Item originalItem = std::any_cast<Inventory::Item>(widgetMeta.at(0));
    auto inventoryItem = std::make_shared<Inventory::Item>(originalItem);

    int buttonX = inventoryItem->x;
    int buttonY = inventoryItem->y;
    int buttonH = inventoryItem->h;
    int buttonW = inventoryItem->w;

    std::string itemName  = inventoryItem->name;
    std::string labelText = std::any_cast<std::string>(widgetMeta.at(1));
    Aether::BorderButton* tileBtn = std::any_cast<Aether::BorderButton*>(widgetMeta.at(2));

    // Apply button attributes
    tileBtn->setXYWH(buttonX, buttonY, buttonW, buttonH);
    tileBtn->setBorderColour(EDITOR_SIDEBAR_COLOUR);

    // Determine whether to use an image or plain text for the button
    std::string thumbPath = inventoryItem->thumbPath;
    Aether::Image* thumbImg = nullptr;

    // If the thumbnail path exists, add an image
    if (!thumbPath.empty()){
        // Get the thumbnail size and generate the key to lookup in the texture cache
        int thumbSize = static_cast<int>(Inventory::kTileSize*Inventory::kThumbScale);
        std::string textureKey = thumbPath+"|"+std::to_string(thumbSize)+"x"+std::to_string(thumbSize);

        // Check if there is a cached texture
        auto textureOpt = Inventory::IconTextureCache->get<Aether::TextureHandle>(
            textureKey
        );

        Aether::TextureHandle texture;

        // Use cached texture if already exists
        if (textureOpt != std::nullopt){
            Logger::Write("[WidgetBuilder::Tile] - Found cached texture for '"+textureKey+"'");
            texture = textureOpt.value();
            thumbImg = new Aether::Image(0, 0, texture, Aether::Render::Wait);
        } // If there is no cached texture, just pass the thumbnail path and generate the texture now 
        else {
            Logger::Write("[WidgetBuilder::Tile] - Failed to find cached texture '"+textureKey+"'");
            thumbImg = new Aether::Image(0, 0, thumbPath, Aether::Render::Wait);
        }

        thumbImg->setScaleDimensions(thumbSize, thumbSize);

        // Reposition the thumbnail to the center of the button
        thumbImg->setXY(
            tileBtn->x()+tileBtn->w()/2-thumbImg->scaleWidth()/2,
            tileBtn->y()+tileBtn->h()/2-thumbImg->scaleHeight()/2
        );

        // Add the image to the widget's elements (lifetime controlled by widget)
        widget->addToElements(thumbImg);

    } else { // Otherwise, just show the name of the item
        tileBtn->setString(itemName);
        tileBtn->setTextColour(TEXT_COLOUR);
    }

    // Set the colour and add it to the widget
    tileBtn->setBorderColour(EDITOR_SIDEBAR_COLOUR);
    
    widget->addToElements(tileBtn);

    // SHOW_AFTER_RENDER hides the widget until the thumbnail image has been rendered
    // It was previously a style choice, until I decided it felt somewhat jarring, and 
    // the standard async rendering was better.
    if (thumbImg != nullptr && SHOW_AFTER_RENDER){
        tileBtn->setHidden(true);
        thumbImg->onRenderDone([tileBtn](){
            tileBtn->setHidden(false);
        });

        thumbImg->renderAsync();
    } else if (thumbImg != nullptr) {
        thumbImg->renderAsync();
    }

    // Add a star to the tile if there is a buff
    if (inventoryItem->modifier != Inventory::Modifier::None){
        Aether::Image* buffIcon = new Aether::Image(
            tileBtn->x()+8, tileBtn->y()+8,
            ASSET_PATHS.at("star_alt"), Aether::Render::Wait
        );

        // Colour if enhanced
        if (Inventory::IsModifierEnhanced(inventoryItem->modifier)){
            buffIcon->setColour(ENHANCED_COLOUR);
        }

        // Resize and render
        buffIcon->setScaleDimensions(BUFF_STAR_SIZE, BUFF_STAR_SIZE);
        buffIcon->renderAsync();

        // Add to tile
        widget->addToElements(buffIcon);
    }

    // Default tile press behaviour - open editor
    // Handle opening the edit overlay when the tile is pressed
    // Open the widget overlay
    /**
     * Handle showing the edit overlay when the tile is pressed
     * Most overlay options are handled on a per-category basis
     */
    tileBtn->onPress([widget, originalItem, inventoryItem]() {
        Logger::Write(inventoryItem->actorName+
            ", flagSp: "+intToHex(inventoryItem->flagSp)+
            ", valueSp: "+intToHex(inventoryItem->valueSp));

        // Get the widget overlay and set the title to show what we are editing
        WidgetOverlay* ovl = widget->getWidgetOverlay();
        ovl->setTitleLabel("Edit Item - "+(inventoryItem->name.empty() ? "Unassigned" : inventoryItem->name));
        ovl->setDeveloperNote(Save::GetKeyDevNote(widget->getSaveEntry()->dataName));

        // Show the widget overlay
        widget->getWindow()->addOverlay(ovl);

        // A background frame for the item image
        Aether::Rectangle* imgFrame = new Aether::Rectangle(60, 0, 280, 280, 8);
        imgFrame->setY((ovl->panel()->y())+(ovl->panel()->h()/2)-(imgFrame->h()/2));
        imgFrame->setColour(MUTED_BACKGROUND_COLOUR);
        ovl->addTempElement(imgFrame);

        // A separator between the image and the rest of the overlay contents
        Aether::Rectangle* imgSeparator = new Aether::Rectangle(imgFrame->x()+imgFrame->w()+24, imgFrame->y(), 1, imgFrame->h());
        imgSeparator->setColour(LIGHT_SEPARATOR_COLOUR);
        ovl->addTempElement(imgSeparator);

        int propertiesX = imgSeparator->x()+24;
        Aether::Text* propertiesText = new Aether::Text(propertiesX, imgSeparator->y(), "Item Properties", 22  );
        propertiesText->setColour(TEXT_COLOUR);
        ovl->addTempElement(propertiesText);

        // The background rectangle for "containing" the property editors
        Aether::Rectangle* propertiesBox = new Aether::Rectangle(propertiesX, propertiesText->y()+propertiesText->textureHeight()+16, 496, 200, 6);
        propertiesBox->setColour(MUTED_BACKGROUND_COLOUR);
        ovl->addTempElement(propertiesBox);
        
        // Create the swap button for swapping the actor
        // Padding: 26px
        Aether::BorderButton* swapBtn = new Aether::BorderButton(0, 0, 148, 42, 3, "", 1, [](){});
        swapBtn->setXY(propertiesBox->x(), propertiesBox->y()+propertiesBox->h()+12);
        swapBtn->setBorderColour(LIGHT_SEPARATOR_COLOUR);
        swapBtn->setFillColour(LIGHT_BACKGROUND_COLOUR);

        Aether::Image* swapImg = new Aether::Image(0, 0, ASSET_PATHS.at("swap"), Aether::Render::Wait);
        swapImg->setScaleDimensions(24, 24);
        swapImg->renderAsync();
        swapImg->setXY(swapBtn->x()+32, swapBtn->y()+swapBtn->h()/2-swapImg->scaleHeight()/2);

        Aether::Text* swapText = new Aether::Text(0, 0, "Swap", 18);
        swapText->setXY(swapImg->x()+swapImg->scaleWidth()+12, swapBtn->y()+swapBtn->h()/2-swapText->textureHeight()/2);

        ovl->addTempElement(swapBtn);
        swapBtn->addElement(swapImg);
        swapBtn->addElement(swapText);

        Aether::BorderButton* deleteBtn = new Aether::BorderButton(0, 0, 148, 42, 3, "", 1, [](){});
        deleteBtn->setXY(swapBtn->x()+swapBtn->w()+26, propertiesBox->y()+propertiesBox->h()+12);
        deleteBtn->setBorderColour(Aether::Colour{96, 0, 0, 150});
        deleteBtn->setFillColour(Aether::Colour{180, 0, 0, 255});

        Aether::Image* deleteImg = new Aether::Image(0, 0, ASSET_PATHS.at("bin"), Aether::Render::Wait);
        deleteImg->setScaleDimensions(24, 24);
        deleteImg->renderAsync();
        deleteImg->setXY(deleteBtn->x()+24, deleteBtn->y()+deleteBtn->h()/2-deleteImg->scaleHeight()/2);

        Aether::Text* deleteText = new Aether::Text(0, 0, "Delete", 18);
        deleteText->setXY(deleteImg->x()+deleteImg->scaleWidth()+12, deleteBtn->y()+deleteBtn->h()/2-deleteText->textureHeight()/2);

        ovl->addTempElement(deleteBtn);
        deleteBtn->addElement(deleteImg);
        deleteBtn->addElement(deleteText);
        
        // Reconfigure dev note if the note exists for this category
        auto devNoteIt = Inventory::kCategoryDevNotes.find(inventoryItem->category);
        if (devNoteIt != Inventory::kCategoryDevNotes.end()){
            ovl->setDeveloperNoteWrap(324);
            ovl->setDeveloperNote(devNoteIt->second);
            ovl->setDeveloperNoteX(propertiesBox->x()+propertiesBox->w()+24);
        }

        int propertyElmX = propertiesX+PROPERTY_EDITOR_SPACING;
        int propertyElmY = propertiesBox->y()+12;

        // PorchItem_Value1
        Aether::Stepper* primaryValueStepper = new Aether::Stepper(propertyElmX, propertyElmY);
        primaryValueStepper->setString("Primary Val.");

        // PorchItem_FlagSp
        Aether::Dropdown* secondaryFlagDropdown = nullptr;
        // PorchItem_ValueSp
        Aether::Stepper* secondaryValueStepper = nullptr;

        // Track how many property editors are visible for the "nothing to edit here" text
        std::vector<Aether::Element*> visibleEditors;

        /**
         * Handle various category-specific tile functions for modifying the widget overlay
         * 
         * This switch is quite gross because of how much it performs, but I feel like making another
         * "builder-type" system here is unnecessary and will potentially complicate things more
         */
        switch (inventoryItem->category){
            case Inventory::Category::Weapons:
            case Inventory::Category::Bows:
            case Inventory::Category::Shields: {
                /**
                 * Set the properties of the primary value spinner
                 * For weapons, this is the durability of the weapon
                 * The maximum value of durability seems to be the s32 max
                 * 
                 * Set the value of the stepper to the value in save data divided by 100
                 * This is because save data stores durability as 100*remaining.hit.points
                 */
                primaryValueStepper->setString("Durability");
                primaryValueStepper->setMax(__INT32_MAX__/Inventory::kDurabilityMultiplier);
                primaryValueStepper->setValue(inventoryItem->value/Inventory::kDurabilityMultiplier);

                /**
                 * This is the modifier dropdown, it shows a list - when clicked - of all the available buffs 
                 * for this item category.
                 * Currently it feels a bit awkward to use, since B does not close the menu as it closes the whole widget
                 */
                secondaryFlagDropdown = new Aether::Dropdown(
                    propertyElmX, primaryValueStepper->y()+primaryValueStepper->h()+PROPERTY_EDITOR_SPACING,
                    "Modifier", 180, 180
                );
                secondaryFlagDropdown->setValue(Inventory::ModifierName((Inventory::Modifier)inventoryItem->flagSp));
                
                /**
                 * Build the map of available buffs for the modifier dropdown to use
                 * 
                 * The dropdown takes a map with the key being the visual value, and the value being whatever data is stored
                 * for that visual value
                 * The actual selected value (in this example, a modifier) can be obtained
                 * by using dropdown->getValueData<Inventory::Modifier>(); 
                 */
                std::map<std::string, std::any> buffMap;
                const auto& categoryModifiers = Inventory::kCategoryModifiers.find(inventoryItem->category);
                if (categoryModifiers != Inventory::kCategoryModifiers.end()){
                    for (const Inventory::Modifier& modifier : categoryModifiers->second){
                        buffMap[Inventory::ModifierName(modifier, categoryModifiers->first)] = std::make_any<Inventory::Modifier>(modifier);
                    }
                }
                secondaryFlagDropdown->setMap(buffMap);
                visibleEditors.push_back(secondaryFlagDropdown);

                /**
                 * This is a stepper for setting the value of the modifier - for example
                 * attack up *3* or multi shot x*5*
                 * 
                 * The max value of the modifier seems to be 65,535 - the maximum unsigned 16 bit integer value
                 * which appears to be a rule enforced by the game at runtime, as PorchItem_ValueSp is s32_array_data 
                 */
                secondaryValueStepper = new Aether::Stepper(
                    primaryValueStepper->x(),
                    secondaryFlagDropdown->y()+secondaryFlagDropdown->h()+16,
                    "Modifier Val."
                );
                secondaryValueStepper->setMax(__UINT16_MAX__);
                secondaryValueStepper->setValue(inventoryItem->valueSp);
                visibleEditors.push_back(secondaryValueStepper);

                /**
                 * Make it so the numpad shows when the value stepper's text box is pressed
                 * for the user to manually input a new value
                 */
                secondaryValueStepper->onPress([secondaryValueStepper](){
                    float outFloat;
                    Result rc = Keyboard::getNumberInput(
                        &outFloat, 
                        "Enter a number... ("
                        +std::to_string(secondaryValueStepper->min())+
                        " → "+std::to_string(secondaryValueStepper->max())+")", 
                        secondaryValueStepper->min(),
                        secondaryValueStepper->max(),
                        false,
                        (secondaryValueStepper->min() < 0)
                    );

                    if (R_SUCCEEDED(rc)){
                        secondaryValueStepper->setValue(static_cast<int>(outFloat));
                    }
                });

                ovl->addTempElement(secondaryValueStepper);
                ovl->addTempElement(secondaryFlagDropdown);
                break;
            }

            case Inventory::Category::Arrows:
            case Inventory::Category::Materials:
            case Inventory::Category::Food:
            case Inventory::Category::KeyItems: {
                /**
                 * All of these inventory categories support a quantity value
                 * Only a few key items (e.g. Spirit Orbs and Korok Seeds) have a relevant quantity value
                 * The other key items are editable for the sake of simpler code
                 * 
                 * Meal-type foods are not stackable, and editing the quantity value for these has no effect
                 */
                primaryValueStepper->setString("Quantity");
                primaryValueStepper->setMax(Inventory::kMaxInventoryQuantity);
                primaryValueStepper->setValue(inventoryItem->value);
                break;
            }

            case Inventory::Category::Armor: {
                /**
                 * Armor's primary value is dye colour
                 * The colours are as follows (in order from value 1 to 15):
                 * - Blue
                 * - Red
                 * - Yellow
                 * - White
                 * - Black
                 * - Purple
                 * - Green
                 * - Light Blue
                 * - Orange
                 * - Navy
                 * - Peach
                 * - Crimson
                 * - Light Yellow
                 * - Brown
                 * - Grey
                 */
                primaryValueStepper->setString("Dye Colour");
                primaryValueStepper->setValue(inventoryItem->value);
                int yCursor = primaryValueStepper->y();

                /**
                 * Armor with the primary value PorchItem_Value1[localIdx] set to -1 is considered
                 * undyeable, so a primary value stepper is not needed
                 * 
                 * The primary value stepper is not deleted, but it is hidden with maximum and minimum
                 * values both set to -1 to ensure the value cannot be changed
                 */
                if (Inventory::IsArmorDyeable(inventoryItem->actorName)){
                    primaryValueStepper->setMax(Inventory::kMaxDyeColour);
                    yCursor += primaryValueStepper->h()+PROPERTY_EDITOR_SPACING;
                } else {
                    // Prevent editing, hide, and do not increment y cursor
                    primaryValueStepper->setMin(-1);
                    primaryValueStepper->setMax(-1);
                    primaryValueStepper->setHidden(true);
                }

                /**
                 * If the armor is upgradeable (if it has a next rank/previous rank entry in my generated
                 * equipment_data.json), then a stepper for setting the rank should be made
                 * 
                 * The maximum number of stars is 4 and the minimum is 0, however BotW stores armor star numbers
                 * starting from 1 and ending at 5, meaning there is a 1 offset between the visuals and the backend
                 */
                if (Inventory::IsArmorUpgradeable(inventoryItem->actorName)){
                    auto starCount = std::make_shared<int>(Inventory::GetArmorStarNum(inventoryItem->actorName));
                    if (*starCount <= 0){ break; }

                    Aether::Stepper* armorLevelStepper = new Aether::Stepper(primaryValueStepper->x(), yCursor);
                    visibleEditors.push_back(armorLevelStepper);
                    
                    armorLevelStepper->setString("Stars");
                    armorLevelStepper->setMin(0); // min visual stars is 0
                    armorLevelStepper->setMax(4); // max visual stars is 4

                    Aether::Text* stars = new Aether::Text(0, 0, "", ARMOR_STAR_SIZE);
                    for (int s = 0; s < *starCount-1; s++){ stars->setString(stars->string()+"★"); }

                    stars->setXY(imgFrame->x()+12, (imgFrame->y()+stars->textureHeight()/2));
                    ovl->addTempElement(stars);

                    // Set its value to the current star value
                    armorLevelStepper->setValue((*starCount) - 1); // Visual is off by one - metadata is 1 for 0 stars, 5 for 4 stars
                    armorLevelStepper->onValueChange([starCount, armorLevelStepper, inventoryItem, stars](){
                        // Get the new value
                        int newStarCount = armorLevelStepper->value() + 1; // Account for visual being off by one

                        // Compare it to the previous star count
                        int diff = newStarCount - *starCount;
                        if (diff == 0){
                            return; // No change, ignore
                        }

                        std::string newActor;
                        if (diff > 0) {
                            // Incremented by at least 1
                            newActor = Inventory::GetNextArmorRankActorName(inventoryItem->actorName);
                        } else { 
                            // Decremented by at least 1
                            newActor = Inventory::GetPrevArmorRankActorName(inventoryItem->actorName);
                        }

                        if (newActor.empty()){
                            return; // Prevent actor name corruption
                        }

                        Logger::Importantf("Current actorName: %s, New actorName: %s", inventoryItem->actorName.c_str(), newActor.c_str());

                        // Update actor name
                        inventoryItem->actorName = newActor;

                        // Update starCount
                        *starCount = newStarCount;

                        // Update star visual
                        stars->setString("");
                        for (int s = 0; s < *starCount-1; s++){ stars->setString(stars->string()+"★"); }
                    });

                    ovl->addTempElement(armorLevelStepper);
                }

                break;
            }

            default: {
                break;
            }
        }

        // Make it so pressing the primary value stepper text box opens a numpad for manual entry
        primaryValueStepper->onPress([primaryValueStepper](){
            float outFloat;
            Result rc = Keyboard::getNumberInput(
                &outFloat, 
                "Enter a number... ("+std::to_string(primaryValueStepper->min())+" → "+std::to_string(primaryValueStepper->max())+")", 
                primaryValueStepper->min(),
                primaryValueStepper->max(),
                false,
                (primaryValueStepper->min() < 0)
            );

            if (R_SUCCEEDED(rc)){
                primaryValueStepper->setValue(static_cast<int>(outFloat));
            }
        });
        ovl->addTempElement(primaryValueStepper);

        // Add to the steppers list
        if (primaryValueStepper->isVisible()){
            visibleEditors.push_back(primaryValueStepper);
        }

        // Show "nothing to edit" confirmation to user if there are no visible editors
        if (visibleEditors.size() <= 0){
            Aether::Text* noEditText = new Aether::Text(0, 0, "No editable properties!", 28);
            noEditText->setXY(propertiesBox->x()+propertiesBox->w()/2-noEditText->textureWidth()/2, propertiesBox->y()+propertiesBox->h()/2-noEditText->textureHeight()/2);
            noEditText->setColour(MUTED_TEXT_COLOR);
            ovl->addTempElement(noEditText);
        }

        Trove* ovlTrove = new Trove();

        // The lambda to run when any values have been updated
        std::function<void()> updateOvl = [
            widget, inventoryItem, ovl, ovlTrove, // Data and pointers
            imgFrame // Other UI elements for relative sizing/positioning
        ](){
            // Clear the ovlTrove
            ovlTrove->clear();

            // Get the thumbnail path, if none found, then use placeholder thumbnail
            std::string thumbPath = Inventory::GetThumbPathFromInventoryItem(*inventoryItem);
            if (thumbPath.empty()){
                thumbPath = PLACEHOLDER_THUMB_PATH;
            }

            // Create the image, resize and render it
            Aether::Image* bigThumbnail = new Aether::Image(
                0, 0,
                thumbPath,
                Aether::Render::Wait
            );
            bigThumbnail->setScaleDimensions(210, 210);
            bigThumbnail->renderAsync();
            bigThumbnail->setXY(
                imgFrame->x()+(imgFrame->w()/2)-(bigThumbnail->scaleWidth()/2),
                imgFrame->y()+10
            );

            // Create the text and reposition it
            Aether::Text* bigName = new Aether::Text(0, 0, (inventoryItem->name.empty() ? "Unassigned" : inventoryItem->name), 20);
            bigName->setXY(imgFrame->x()+(imgFrame->w()/2)-(bigName->textureWidth()/2), imgFrame->y()+imgFrame->h()-bigName->textureHeight()-16);

            // Add everything to the overlay and the per-update trove
            ovl->addTempElement(bigThumbnail);
            ovlTrove->addElement(bigThumbnail);

            ovl->addTempElement(bigName);
            ovlTrove->addElement(bigName);
        };

        // update the UI
        updateOvl();

        // For handling close
        widget->getWidgetOverlay()->onCloseSignal->connectWithId([
                widget, ovlTrove, originalItem, 
                inventoryItem, primaryValueStepper, secondaryFlagDropdown, 
                secondaryValueStepper
            ]
            (size_t connId, bool isSave){
            // Memory management
            widget->getWidgetOverlay()->onCloseSignal->disconnect(connId);
            delete ovlTrove;

            // Save handling (TEMPORARY FOR TESTING)
            if (isSave){
                Save::File* saveFile = widget->getMenu()->saveFile();

                // Get inventory ID list for updating actorName
                Save::Entry* porchItemEntry = Save::GetEntryFromDataName(*saveFile, Inventory::kInventoryKey);
                auto& porchItem = std::get<std::vector<std::string>>(porchItemEntry->value);
                porchItem[inventoryItem->index] = inventoryItem->actorName;

                // Get the porch item primary value for updating durability/dye colour/quantity etc.
                Save::Entry* porchItemPrimaryValueEntry = Save::GetEntryFromDataName(*saveFile, Inventory::kInventoryValueKey);
                auto& porchItemPrimaryValue = std::get<std::vector<int32_t>>(porchItemPrimaryValueEntry->value);

                // Update the data
                int newPrimaryValue = primaryValueStepper->value();

                // Validate/reform data on a per-item basis
                switch (inventoryItem->category){
                    case Inventory::Category::Weapons:
                    case Inventory::Category::Bows:
                    case Inventory::Category::Shields: {
                        // Apply multiplier
                        newPrimaryValue *= Inventory::kDurabilityMultiplier;
                        break;
                    }

                    default: {
                        break;
                    }
                }

                // Modify save file
                porchItemPrimaryValue[inventoryItem->index] = newPrimaryValue;

                // Manage secondary flag (buff)
                if (secondaryFlagDropdown != nullptr){
                    Save::Entry* porchItemSecondaryFlagsEntry = Save::GetEntryFromDataName(
                        *saveFile,
                        inventoryItem->flagSpKey
                    );

                    auto& porchItemSecondaryFlags = std::get<std::vector<int32_t>>(
                        porchItemSecondaryFlagsEntry->value
                    );

                    int32_t oldFlag = porchItemSecondaryFlags.at(inventoryItem->localIndex);

                    // Get the data value from the dropdown, specify it is a modifier, use the old flag if nullopt, and cast to int
                    porchItemSecondaryFlags[inventoryItem->localIndex] 
                    = static_cast<int32_t>(
                        secondaryFlagDropdown->getValueData<Inventory::Modifier>()
                        .value_or((Inventory::Modifier)oldFlag)
                    );
                }

                // Manage secondary value (buff value)
                if (secondaryValueStepper != nullptr){
                    Save::Entry* porchItemSecondaryValueEntry = Save::GetEntryFromDataName(
                        *saveFile, 
                        inventoryItem->valueSpKey
                    );

                    Logger::Important("Getting the porch item secondary value...");
                    auto& porchItemSecondaryValue = std::get<std::vector<int32_t>>(porchItemSecondaryValueEntry->value);

                    Logger::Important("Updating value...");
                    porchItemSecondaryValue[inventoryItem->localIndex] = secondaryValueStepper->value();

                    Logger::Important("Done.");
                }

                // Reload to update inventory item data
                widget->getMenu()->requestReload();
            } else {
                // If it was not saved, reset inventoryItem to the original value and continue
                *inventoryItem = originalItem;
            }
        });
    });

    // Embedded switch for handling different types of tiles, and then their styles
    switch (inventoryItem->type){
        // Standard filled inventory button assigned to an item in the inventory
        case (Inventory::TileType::Standard): {
            // Style switch
            switch (inventoryItem->style){
                case (Inventory::TileStyle::NoLabel):{
                    // Make new text
                    Aether::Text* valueText = new Aether::Text(
                        0, 0,
                        labelText,
                        18
                    );

                    valueText->setXY(
                        tileBtn->x()+14,
                        tileBtn->y()+tileBtn->h()-valueText->textureHeight()-12
                    );

                    valueText->setColour(TEXT_COLOUR);
                    widget->addToElements(valueText);
                    break;
                }

                case (Inventory::TileStyle::Label): {
                    // Make a label
                    Aether::Rectangle* labelBg = new Aether::Rectangle(
                        0, 0,
                        Inventory::kTileSize/3,
                        Inventory::kTileSize/5,
                        3
                    );

                    labelBg->setXY(
                        tileBtn->x()+tileBtn->w()-labelBg->w()/2-Inventory::kTilePadding,
                        tileBtn->y()+tileBtn->h()-labelBg->h()/2-6
                    );

                    labelBg->setColour(MUTED_BACKGROUND_COLOUR);

                    // Make the text
                    Aether::Text* textElm = new Aether::Text(
                        0, 0,
                        labelText,
                        16
                    );
                    
                    textElm->setXY(
                        labelBg->x()+labelBg->w()/2-textElm->textureWidth()/2,
                        labelBg->y()+labelBg->h()/2-textElm->textureHeight()/2
                    );

                    textElm->setColour(TEXT_COLOUR);

                    // Add everything to the widget
                    widget->addToElements(labelBg);
                    widget->addToElements(textElm);

                    break;
                }
            }

            break;
        }

        // An empty slot representing available space
        case (Inventory::TileType::Empty): {
            // Stub
            break;
        }

        // An add button showing that an additional slot can be added
        case (Inventory::TileType::Add): {
            // Show "+" symbol
            tileBtn->setString("+");
            tileBtn->onPress([menu, inventoryItem, widget](){
                // Add a new slot
                // Find the save key
                Save::File* saveFile = menu->saveFile();
                auto it = Inventory::kSlotKeys.find(inventoryItem->category);

                if (it != Inventory::kSlotKeys.end()){ // If found, modify
                    std::string slotKey = it->second;
                    Save::Entry* slotEntry = Save::GetEntryFromDataName(*saveFile, slotKey);
                    
                    // for bounds checking
                    auto it = Inventory::kMaxSlots.find(inventoryItem->category);
                    int maxSlots = -1;

                    if (it != Inventory::kMaxSlots.end()){
                        maxSlots = it->second;
                    }

                    // calculate new value
                    int32_t newSlotCount = std::get<int32_t>(slotEntry->value)+1;
                    if (newSlotCount > maxSlots && maxSlots >= 0){ // if there is a max slots limit and it has been exceeded
                        newSlotCount = maxSlots; // default to maxSlots
                    }

                    // update value
                    slotEntry->value = newSlotCount;

                    // reload
                    menu->requestReload();
                }
            });
        }
    }
}