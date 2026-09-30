/**
 * @brief The main editor menu used for editing save files.
 */

#include <switch.h>
#include <Aether/Aether.hpp>

#include <Screens/Editor.hpp>
#include <Config.hpp>
#include <Utils/Utils.hpp>

#include <vector>
#include <string>

Editor::Editor(Aether::Window* window) : window(window){
    // data
    std::vector<std::string> tabs = {
        "Stats",
        "Equipment",
        "Items",
        "Dev Editor"
    };

    // metadata about how tabs should be filled
    // to be used with the "showTabByName" helper
    std::unordered_map<std::string, nullptr_t> tabData = {};
}

Editor::~Editor(){}

void Editor::buildSidebar(){
    // make the background frame
    Aether::Rectangle* sidebarFrame = new Aether::Rectangle(
        0,
        0,
        WINDOW_RESOLUTION_X / 4,
        WINDOW_RESOLUTION_Y
    );

    sidebarFrame->setColour(EDITOR_SIDEBAR_COLOUR);

    // logo image
    Aether::Image* logoImg = new Aether::Image(
        0,
        0,
        ASSET_PATHS.at("triforce"),
        Aether::Render::Wait
    );

    // log("asset path: "+ASSET_PATHS.at("triforce"));

    logoImg->setScaleDimensions(
        24,
        24
    );

    // logo text
    Aether::Text* logoText = new Aether::Text(
        0,
        0,
        "RUNE",
        24
    );

    logoText->setColour(TEXT_COLOUR);

    // RUNE logo separator
    Aether::Rectangle* logoSeparator = new Aether::Rectangle(
        0,
        0,
        sidebarFrame->w()*(2/3),
        1
    );

    logoSeparator->setColour(SEPARATOR_COLOUR);
    logoSeparator->setX(
        sidebarFrame->x()+(sidebarFrame->w()/2)
        -(logoSeparator->w()/2)
    );

    // align everything
    log("Rendering image....");

    logoImg->renderSync();

    log("Image successfully rendered!");
    
    logoImg->setX(logoSeparator->x());

    logoText->setX(logoImg->x()+logoImg->textureWidth()+TEXT_X_OFFSET);

    logoText->setY(logoSeparator->y()-(logoText->textureHeight())-TEXT_Y_OFFSET);
    logoImg->setY(logoText->y());

    // add all elements
    this->addElement(sidebarFrame);


    /**
     * 00:00:12.911 |W| HLE.GuestThread.46 KernelSvc OutputDebugString: [!! AETHER !!] - Couldn't render filled rectangle texture: Invalid values
     */
    this->addElement(logoImg);
    this->addElement(logoText);
    this->addElement(logoSeparator);
}

void Editor::onLoad(){
    this->buildSidebar();
}

void Editor::onUnload(){

}