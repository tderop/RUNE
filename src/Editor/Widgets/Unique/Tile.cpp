#include <Editor/EditorMenu.hpp>
#include <Editor/EditorWidget.hpp>

#include <Utils/Utils.hpp>
#include <Config.hpp>

#include <any>

void Rune::WidgetBuilder::Tile(EditorWidget* widget){
    // Check if meta was provided
    if (widget->getMeta().empty()){
        log("[Rune::WidgetBuilder::Tile] - No widget metadata provided!", true);
        return;
    }

    // Unpack the meta
    std::vector<std::any> widgetMeta = widget->getMeta();

    int buttonX = std::any_cast<int>(widgetMeta.at(0));
    int buttonY = std::any_cast<int>(widgetMeta.at(1));
    int buttonW = std::any_cast<int>(widgetMeta.at(2));
    int buttonH = std::any_cast<int>(widgetMeta.at(3));
    std::string itemName = std::any_cast<std::string>(widgetMeta.at(4));

    // Make the filled button
    Aether::BorderButton* tileBtn = new Aether::BorderButton(
        buttonX,
        buttonY,
        buttonW,
        buttonH,
        4,
        itemName,
        16,
        [](){}
    );

    // Set the colour and add it to the widget
    tileBtn->setBorderColour(EDITOR_SIDEBAR_COLOUR);
    widget->addToElements(tileBtn);

    // Make the label
    std::string labelStyle = std::any_cast<std::string>(widgetMeta.at(5));
    std::string labelText  = std::any_cast<std::string>(widgetMeta.at(6));

    if (labelStyle == "NoLabel"){
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
    }
}