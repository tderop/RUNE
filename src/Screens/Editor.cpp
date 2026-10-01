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

void Editor::buildScreen(){
    // make the background frame
    Aether::Rectangle* sidebarFrame = new Aether::Rectangle(
        0,
        0,
        WINDOW_RESOLUTION_X / 4,
        WINDOW_RESOLUTION_Y
    );

    sidebarFrame->setColour(EDITOR_SIDEBAR_COLOUR);

    // logo image (UNscrapped?????)
    Aether::Image* logoImg = new Aether::Image(
        0,
        0,
        ASSET_PATHS.at("triforce"),
        Aether::Render::Wait
    );

    logoImg->setScaleDimensions(28, 28);
    logoImg->renderSync();

    // logo text
    Aether::Text* logoText = new Aether::Text(
        0,
        0,
        "RUNE - Editor",
        28
    );

    logoText->setColour(TEXT_COLOUR);

    // RUNE logo separator
    // INTEGER DIVISION BUG OH MY FUCKING GOD
    // sidebarFrame->w()*(2/3)
    int logoSeparatorWidth = static_cast<int>(sidebarFrame->w()*(3.0/4.0));
    Aether::Rectangle* logoSeparator = new Aether::Rectangle(
        0,
        0,
        logoSeparatorWidth,
        2
    );

    logoSeparator->setColour(SEPARATOR_COLOUR);

    // align everything
    logoSeparator->setX(
        sidebarFrame->x()+(sidebarFrame->w()/2)
        -(logoSeparator->w()/2)
    );

    // background
    Aether::Rectangle* background = new Aether::Rectangle(
        0,
        0,
        WINDOW_RESOLUTION_X,
        WINDOW_RESOLUTION_Y
    );

    background->setColour(BACKGROUND_COLOUR);

    // top panel
    Aether::Rectangle* topPanel = new Aether::Rectangle(
        sidebarFrame->w(),
        0,
        WINDOW_RESOLUTION_X-sidebarFrame->w(),
        72
    );

    topPanel->setColour(MUTED_BACKGROUND_COLOUR);

    logoImg->setX(logoSeparator->x());
    logoText->setX(logoImg->x()+logoImg->textureWidth()+TEXT_X_OFFSET);

    logoSeparator->setY(71);
    logoText->setY(logoSeparator->y()-(logoText->textureHeight())-TEXT_Y_OFFSET);
    logoImg->setY(logoText->y());

    // copyright text
    Aether::Text* copyrightAndDisclaimer = new Aether::Text(
        logoSeparator->x(),
        sidebarFrame->h(),
        "RUNE © 2026 tderop",
        12
    );

    copyrightAndDisclaimer->setY(sidebarFrame->h()-copyrightAndDisclaimer->textureHeight()-TEXT_Y_OFFSET);
    copyrightAndDisclaimer->setColour(MUTED_TEXT_COLOR);

    // add all elements
    this->addElement(background);

    this->addElement(topPanel);
    this->addElement(sidebarFrame);

    this->addElement(logoText);

    /**
     * 00:00:12.911 |W| HLE.GuestThread.46 KernelSvc OutputDebugString: [!! AETHER !!] - Couldn't render filled rectangle texture: Invalid values
     */
    this->addElement(logoSeparator);

    // wasn't even a bug with the image
    this->addElement(logoImg);
    this->addElement(copyrightAndDisclaimer);
}

void Editor::onLoad(){
    this->buildScreen();
}

void Editor::onUnload(){

}