#include <Editor/EditorMenu.hpp>
#include <Editor/EditorWidget.hpp>

#include <Utils/Utils.hpp>
#include <Utils/Logger.hpp>
#include <Config.hpp>

#include <Editor/Inventory.hpp>

#include <any>

const bool SHOW_AFTER_RENDER = false;

void Rune::WidgetBuilder::Tile(EditorWidget* widget){
    // Check if meta was provided
    if (widget->getMeta().empty()){
        Logger::Write("[Rune::WidgetBuilder::Tile] - No widget metadata provided!");
        return;
    }

    EditorMenu* menu = widget->getMenu();

    // Unpack the meta
    std::vector<std::any> widgetMeta = widget->getMeta();
    Rune::Inventory::Item inventoryItem = std::any_cast<Rune::Inventory::Item>(widgetMeta.at(0));

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
        int thumbSize = static_cast<int>(Rune::Inventory::kTileSize*Rune::Inventory::kThumbScale);
        std::string textureKey = thumbPath+"|"+std::to_string(thumbSize)+"x"+std::to_string(thumbSize);

        // Check if there is a cached texture
        auto textureOpt = Rune::Inventory::IconTextureCache->get<Aether::TextureHandle>(
            textureKey
        );

        Aether::TextureHandle texture;

        // Use cached texture if already exists
        if (textureOpt != std::nullopt){
            Logger::Write("[Rune::WidgetBuilder::Tile] - Found cached texture for '"+textureKey+"'");
            texture = textureOpt.value();
            thumbImg = new Aether::Image(0, 0, texture, Aether::Render::Wait);
        } // If there is no cached texture, just pass the thumbnail path and generate the texture now 
        else {
            Logger::Write("[Rune::WidgetBuilder::Tile] - Failed to find cached texture '"+textureKey+"'");
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

    // Embedded switch for handling different types of tiles, and then their styles
    switch (inventoryItem.type){
        // Standard filled inventory button assigned to an item in the inventory
        case (Rune::Inventory::TileType::Standard): {
            // Style switch
            switch (inventoryItem.style){
                case (Rune::Inventory::TileStyle::NoLabel):{
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

                case (Rune::Inventory::TileStyle::Label): {
                    // Make a label
                    Aether::Rectangle* labelBg = new Aether::Rectangle(
                        0, 0,
                        Rune::Inventory::kTileSize/3,
                        Rune::Inventory::kTileSize/5,
                        3
                    );

                    labelBg->setXY(
                        tileBtn->x()+tileBtn->w()-labelBg->w()/2-Rune::Inventory::kTilePadding,
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
        case (Rune::Inventory::TileType::Empty): {
            // Stub
            break;
        }

        // An add button showing that an additional slot can be added
        case (Rune::Inventory::TileType::Add): {
            // Show "+" symbol
            tileBtn->setString("+");
            tileBtn->onPress([menu, inventoryItem, widget](){
                // Add a new slot
                // Find the save key
                Rune::Save::File* saveFile = menu->saveFile();
                auto it = Rune::Inventory::kSlotKeys.find(inventoryItem.category);

                if (it != Rune::Inventory::kSlotKeys.end()){ // If found, modify
                    std::string slotKey = it->second;
                    Rune::Save::Entry* slotEntry = Rune::Save::GetEntryFromDataName(*saveFile, slotKey);
                    
                    // for bounds checking
                    auto it = Rune::Inventory::kMaxSlots.find(inventoryItem.category);
                    int maxSlots = -1;

                    if (it != Rune::Inventory::kMaxSlots.end()){
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

    // Handle opening the edit overlay when the tile is pressed
    // Open the widget overlay
    tileBtn->onPress([widget, inventoryItem](){
        Logger::Write(inventoryItem.actorName+
            ", flagSp: "+intToHex(inventoryItem.flagSp)+
            ", valueSp: "+intToHex(inventoryItem.valueSp));

        // // Build the overlay
        // Get the widget overlay and set the title to show what we are editing
        WidgetOverlay* ovl = widget->getWidgetOverlay();
        ovl->setTitleLabel("Edit Item - "+inventoryItem.name);
        
        // Show the widget overlay
        widget->getWindow()->addOverlay(ovl);
    });
}