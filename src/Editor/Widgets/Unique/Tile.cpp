#include <Editor/EditorMenu.hpp>
#include <Editor/EditorWidget.hpp>

#include <Utils/Utils.hpp>
#include <Config.hpp>

#include <Editor/Inventory.hpp>

#include <any>

void Rune::WidgetBuilder::Tile(EditorWidget* widget){
    // Check if meta was provided
    if (widget->getMeta().empty()){
        log("[Rune::WidgetBuilder::Tile] - No widget metadata provided!", true);
        return;
    }

    // Unpack the meta
    std::vector<std::any> widgetMeta = widget->getMeta();
    Rune::Inventory::Item inventoryItem = std::any_cast<Rune::Inventory::Item>(widgetMeta.at(0));

    int buttonX = inventoryItem.x;
    int buttonY = inventoryItem.y;
    int buttonH = inventoryItem.h;
    int buttonW = inventoryItem.w;

    std::string itemName  = inventoryItem.name;
    std::string labelText = std::any_cast<std::string>(widgetMeta.at(1));

    // Make the filled button
    Aether::BorderButton* tileBtn = new Aether::BorderButton(
        buttonX,
        buttonY,
        buttonW,
        buttonH,
        4,
        itemName,
        24,
        [](){}
    );

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
                    // nothing for now
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
        }
    }
}