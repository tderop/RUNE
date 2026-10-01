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

int TAB_BUTTON_HEIGHT = 64; // px

Editor::Editor(Aether::Window* window) : window(window){
    // data
    this->tabs = {
        "Stats",
        "Equipment",
        "Items",
        "Dev Editor"
    };

    // metadata about how tabs should be filled
    // to be used with the "showTabByName" helper
    this->tabData = {
        {
            "Stats",
            EditorTab{
                .menus={"Stats", "Coordinates", "Completionism"}
            }
        },

        {
            "Equipment",
            EditorTab{
                .menus={"Weapons", "Bows", "Shields", "Armor"}
            }
        },

        {
            "Items",
            EditorTab{
                .menus={"Materials", "Food", "Key Items"}
            }
        }
    };
}

Editor::~Editor(){}

/**
 * @brief Reposition a rectangle to align with a button to show it is selected.
 */
void Editor::positionSelectIndicator(Aether::Rectangle* selectIndicator, Aether::ListButton* targetBtn){
    selectIndicator->setH(
        (int)((targetBtn->h())*(2.0/3.0))
    );

    // log("Select Indicator Height: "+std::to_string(selectIndicator->h())+"px");

    selectIndicator->setXY(
        targetBtn->x()+(int)(TEXT_X_OFFSET/2.0),
        (targetBtn->y())+((int)targetBtn->h()/2.0)-((int)selectIndicator->h()/2.0) // yummy
    );
    // log("Select Indicator X: "+std::to_string(selectIndicator->x())+"px");
    // log("Select Indicator Y: "+std::to_string(selectIndicator->y())+"px");

    for (Aether::ListButton* otherBtn : this->tabButtons){
        if (otherBtn != targetBtn){
            otherBtn->setTextColour(TEXT_COLOUR);
            // otherBtn->setLineColour(SEPARATOR_COLOUR);
        }
        else{
            otherBtn->setTextColour(TOP_R);
            // otherBtn->setLineColour(TEXT_COLOUR);
        }
    }
}

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

    // build tab buttons
    Aether::Container* tabButtonList = new Aether::Container(
        logoSeparator->x(),
        logoSeparator->y()+36,
        logoSeparator->w(),
        96 //,
        // Aether::Padding::None
    );
    // tabButtonList->setScrollBarColour(SEPARATOR_COLOUR);
    // tabButtonList->setShowScrollBar(false);

    // create tab buttons
    Aether::Rectangle* selIndicator = new Aether::Rectangle(0, 0, 3, 1);
    selIndicator->setColour(TOP_R);

    int tabIdx = 0;

    for (std::string tabName : this->tabs){
        // make a button
        Aether::ListButton* tabButton;
        tabButton = new Aether::ListButton(
            tabName,
            [this, tabName, selIndicator, tabIdx](){
                log(tabName+" was selected");
                positionSelectIndicator(selIndicator, this->tabButtons.at(tabIdx));
            }
        );

        tabButton->setH(TAB_BUTTON_HEIGHT);
        tabButton->setW(logoSeparator->w());

        tabButton->setX(tabButtonList->x());
        tabButton->setY(tabButtonList->y()+(TAB_BUTTON_HEIGHT*tabIdx));

        tabButton->setLineColour(Aether::Colour{0, 0, 0, 0});

        tabButtons.push_back(tabButton);
        tabButtonList->addElement(tabButton);

        tabIdx++;
    }

    // reposition select indicator
    positionSelectIndicator(selIndicator, tabButtons.at(0));

    // adjust height of list accordingly
    int listHeight = tabButtons.at(0)->h();
    for (Aether::ListButton* button : tabButtons){
        int buttonHeight = button->h();
        listHeight += buttonHeight;
    }

    if (listHeight > 0){
        tabButtonList->setH(listHeight);
    }

    Aether::Rectangle* debugRect = new Aether::Rectangle(
        tabButtonList->x(),
        tabButtonList->y(),
        tabButtonList->w(),
        tabButtonList->h(),
        4
    );

    debugRect->setColour(MUTED_BACKGROUND_COLOUR);

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

    // this->addElement(debugRect);
    this->addElement(tabButtonList);
    this->addElement(selIndicator);

    this->addElement(copyrightAndDisclaimer);
}

void Editor::showTab(){
    // get all tabs
}

void Editor::onLoad(){
    this->buildScreen();
}

void Editor::onUnload(){

}