#include <Aether/Aether.hpp>

#include <Editor/EditorMenu.hpp>
#include <Editor/EditorWidget.hpp>

#include <Utils/Utils.hpp>
#include <Utils/Logger.hpp>
#include <Config.hpp>

#include <Editor/Inventory.hpp>

#include <any>

// Tile-specific constants
const bool SHOW_AFTER_RENDER = false;

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
    Inventory::Item inventoryItem = std::any_cast<Inventory::Item>(widgetMeta.at(0));

    int buttonX = inventoryItem.x;
    int buttonY = inventoryItem.y;
    int buttonH = inventoryItem.h;
    int buttonW = inventoryItem.w;

    std::string itemName  = inventoryItem.name;
    std::string labelText = std::any_cast<std::string>(widgetMeta.at(1));
    Aether::BorderButton* tileBtn = std::any_cast<Aether::BorderButton*>(widgetMeta.at(2));

    // Apply button attributes
    tileBtn->setXYWH(buttonX, buttonY, buttonW, buttonH);
    tileBtn->setBorderColour(EDITOR_SIDEBAR_COLOUR);

    // Determine whether to use an image or plain text for the button
    std::string thumbPath = inventoryItem.thumbPath;
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

    // Default tile press behaviour - open editor
    // Handle opening the edit overlay when the tile is pressed
    // Open the widget overlay
    tileBtn->onPress([widget, inventoryItem]() {
        Logger::Write(inventoryItem.actorName+
            ", flagSp: "+intToHex(inventoryItem.flagSp)+
            ", valueSp: "+intToHex(inventoryItem.valueSp));

        // // Build the overlay
        // Get the widget overlay and set the title to show what we are editing
        WidgetOverlay* ovl = widget->getWidgetOverlay();
        ovl->setTitleLabel("Edit Item - "+inventoryItem.name);
        ovl->setDeveloperNote(Save::GetKeyDevNote(widget->getSaveEntry()->dataName));

        // Show the widget overlay
        widget->getWindow()->addOverlay(ovl);

            /**
             * HERE
             * WORK FROM HERE
             * WORK ON THE WEAPON EDITOR UI
             * MAKE A SWITCH FOR DIFFERENT CATEGORIES OF ITEMS
             */

        // A background frame for the item image
        Aether::Rectangle* imgFrame = new Aether::Rectangle(
            60, 0,
            280, 280,
            6
        );
        imgFrame->setY((ovl->panel()->y())+(ovl->panel()->h()/2)-(imgFrame->h()/2));
        imgFrame->setColour(MUTED_BACKGROUND_COLOUR);
        ovl->addTempElement(imgFrame);

        // A separator between the image and the properties
        Aether::Rectangle* imgSeparator = new Aether::Rectangle(
            imgFrame->x()+imgFrame->w()+24, imgFrame->y(),
            1, imgFrame->h()
        );
        imgSeparator->setColour(LIGHT_SEPARATOR_COLOUR);
        ovl->addTempElement(imgSeparator);

        // Item properties
        int propertiesX = imgSeparator->x()+24;

        Aether::Text* propertiesText = new Aether::Text(
            propertiesX, imgSeparator->y(),
            "Item Properties",
            22  
        );
        propertiesText->setColour(TEXT_COLOUR);
        ovl->addTempElement(propertiesText);

        Aether::Rectangle* propertiesBox = new Aether::Rectangle(
            propertiesX, propertiesText->y()+propertiesText->textureHeight()+16,
            496, 239,
            6
        );
        propertiesBox->setColour(MUTED_BACKGROUND_COLOUR);
        ovl->addTempElement(propertiesBox);

        // Declarations
        Aether::Stepper* durabilityStepper;

        // Determine how values are filled based on item type
        switch (inventoryItem.category){
            case Inventory::Category::Weapons:
            case Inventory::Category::Bows:
            case Inventory::Category::Shields: {
                // Durability stepper
                durabilityStepper = new Aether::Stepper(
                    propertiesBox->x()+16, propertiesBox->y()+12,
                    "Durability"
                );
                durabilityStepper->setMax(__INT32_MAX__/Inventory::kDurabilityMultiplier);
                durabilityStepper->setValue(inventoryItem.value/Inventory::kDurabilityMultiplier);

                ovl->addTempElement(durabilityStepper);
                break;
            }

            default: {
                break;
            }
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
            std::string thumbPath = Inventory::GetThumbPathFromInventoryItem(inventoryItem);
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
            Aether::Text* bigName = new Aether::Text(
                0, 0,
                inventoryItem.name, 20
            );
            bigName->setXY(
                imgFrame->x()+(imgFrame->w()/2)-(bigName->textureWidth()/2),
                imgFrame->y()+imgFrame->h()-bigName->textureHeight()-16
            );

            // Add everything to the overlay and the per-update trove
            ovl->addTempElement(bigThumbnail);
            ovlTrove->addElement(bigThumbnail);

            ovl->addTempElement(bigName);
            ovlTrove->addElement(bigName);
        };

        // update the UI
        updateOvl();

        // For handling close
        widget->getWidgetOverlay()->onCloseSignal->connectWithId([widget, ovlTrove, inventoryItem, durabilityStepper](size_t connId, bool isSave){
            // Memory management
            widget->getWidgetOverlay()->onCloseSignal->disconnect(connId);
            delete ovlTrove;

            // Save handling (TEMPORARY FOR TESTING)
            if (isSave){
                // Update porchValue
                Save::Entry* valueEntry = Save::GetEntryFromDataName(*widget->getMenu()->saveFile(), Inventory::kInventoryValueKey);
                auto& porchValue = std::get<std::vector<int>>(valueEntry->value);
                porchValue[inventoryItem.index] = durabilityStepper->value()*Inventory::kDurabilityMultiplier;

                widget->getMenu()->requestReload();
            }
        });
    });

    // Embedded switch for handling different types of tiles, and then their styles
    switch (inventoryItem.type){
        // Standard filled inventory button assigned to an item in the inventory
        case (Inventory::TileType::Standard): {
            // Style switch
            switch (inventoryItem.style){
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
                auto it = Inventory::kSlotKeys.find(inventoryItem.category);

                if (it != Inventory::kSlotKeys.end()){ // If found, modify
                    std::string slotKey = it->second;
                    Save::Entry* slotEntry = Save::GetEntryFromDataName(*saveFile, slotKey);
                    
                    // for bounds checking
                    auto it = Inventory::kMaxSlots.find(inventoryItem.category);
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