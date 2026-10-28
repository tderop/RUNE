#include <Editor/EditorMenu.hpp>
#include <Editor/EditorWidget.hpp>

#include <Utils/Utils.hpp>
#include <Config.hpp>

#include <Editor/Inventory.hpp>

#include <any>

void Rune::WidgetBuilder::Tile(EditorWidget* widget){
    // Check if meta was provided
    if (widget->getMeta().empty()){
        log("[Rune::WidgetBuilder::Tile] - No widget metadata provided!");
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
    tileBtn->setString(itemName);
    tileBtn->setTextColour(TEXT_COLOUR);
    tileBtn->onPress([inventoryItem](){
        log(inventoryItem.actorName+
            ", flagSp: "+intToHex(inventoryItem.flagSp)+
            ", valueSp: "+intToHex(inventoryItem.valueSp));
    });

    // Set the colour and add it to the widget
    tileBtn->setBorderColour(EDITOR_SIDEBAR_COLOUR);
    widget->addToElements(tileBtn);

    // Embedded switch for handling different types of tiles, and then their styles
    switch (inventoryItem.type){
        // Standard filled inventory button assigned to an item in the inventory
        case (Rune::Inventory::TileType::Standard): {
            // Style switch
            switch (inventoryItem.style){
                case (Rune::Inventory::TileStyle::NoLabel):{
                    // Make new text
                    Aether::Text* labelElm = new Aether::Text(
                        0, 0,
                        labelText,
                        14
                    );

                    labelElm->setXY(
                        tileBtn->x()+tileBtn->w()-labelElm->textureWidth()-8,
                        tileBtn->y()+tileBtn->h()-labelElm->textureHeight()-10
                    );

                    labelElm->setColour(TEXT_COLOUR);
                    widget->addToElements(labelElm);

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
            tileBtn->onPress([menu, widget](){
                // Add a new slot
                // Find the save key
                Rune::Save::File* saveFile = menu->saveFile();
                auto it = Rune::Inventory::kSlotKeys.find(menu->menuName());

                if (it != Rune::Inventory::kSlotKeys.end()){ // If found, modify
                    std::string slotKey = it->second;
                    Rune::Save::Entry* slotEntry = Rune::Save::GetEntryFromDataName(*saveFile, slotKey);
                    
                    // for bounds checking
                    auto it = Rune::Inventory::kMaxSlots.find(menu->menuName());
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
}