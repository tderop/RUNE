/**
 * @brief The main editor menu used for editing save files.
 */

#include <switch.h>
#include <Aether/Aether.hpp>

#include <Screens/Editor.hpp>
#include <Config.hpp>
#include <Utils/Utils.hpp>
#include <Input/Input.hpp>
#include <Save/Save.hpp>

#include <vector>
#include <string>

#include <Application/Application.hpp>
#include <Widgets/EditorWidget.hpp>
#include <Widgets/WidgetOverlay.hpp>


int TAB_BUTTON_HEIGHT = 64; // px
int EDITOR_TOP_Y = 72;
int MAX_TITLE_LENGTH = 16;

EditorScreen::EditorScreen(Rune::Application* app) : app(app) {
    // init
    this->trove = new Trove();

    // data
    this->tabs = {
        "Stats",
        "Equipment",
        "Items",
        "Dev Editor"
    };

    // metadata about how tabs should be filled
    // to be used with the "showTabByName" helper
}

/**
 * @brief Delete visible widgets when deleted.
 */
EditorScreen::~EditorScreen(){
    this->cleanEditorWidgets();
    delete this->trove;
}

/**
 * @brief Reposition a rectangle to align with a button to show it is selected.
 */
void EditorScreen::positionTabSelectIndicator(Aether::Rectangle* selectIndicator, Aether::ListButton* targetBtn){
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

void EditorScreen::nextMenu(InputSystem& inputService, InputState state){
    if (state != InputState::Begin){
        return;
    }

    this->showMenu(this->selectedMenu+1);
}

void EditorScreen::prevMenu(InputSystem& inputService, InputState state){
    if (state != InputState::Begin){
        return;
    }

    this->showMenu(this->selectedMenu-1);
}

void EditorScreen::buildScreen(){
    // make the background frame
    log("[EDITOR] - Making sidebar frame...");
    Aether::Rectangle* sidebarFrame = new Aether::Rectangle(
        0,
        0,
        WINDOW_RESOLUTION_X / 4,
        WINDOW_RESOLUTION_Y
    );

    sidebarFrame->setColour(EDITOR_SIDEBAR_COLOUR);

    // logo image (UNscrapped?????)
    log("[EDITOR] - Making logo image...");
    Aether::Image* logoImg = new Aether::Image(
        0,
        0,
        ASSET_PATHS.at("triforce"),
        Aether::Render::Wait
    );

    logoImg->setScaleDimensions(28, 28);
    logoImg->renderSync();

    // logo text
    log("[EDITOR] - Making logo text...");
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
    log("[EDITOR] - Making logo separator...");
    int logoSeparatorWidth = static_cast<int>(sidebarFrame->w()*(3.0/4.0));
    Aether::Rectangle* logoSeparator = new Aether::Rectangle(
        0,
        0,
        logoSeparatorWidth,
        2
    );

    logoSeparator->setColour(SEPARATOR_COLOUR);

    // background
    log("[EDITOR] - Making background...");
    Aether::Rectangle* background = new Aether::Rectangle(
        0,
        0,
        WINDOW_RESOLUTION_X,
        WINDOW_RESOLUTION_Y
    );

    background->setColour(BACKGROUND_COLOUR);

    // top panel
    log("[EDITOR] - Making top panel...");
    topPanel = new Aether::Rectangle(
        sidebarFrame->w(),
        0,
        WINDOW_RESOLUTION_X-sidebarFrame->w(),
        EDITOR_TOP_Y
    );

    topPanel->setColour(MUTED_BACKGROUND_COLOUR);

    // separators
    log("[EDITOR] - Making top separator...");
    Aether::Rectangle* topSeparator = new Aether::Rectangle(
        topPanel->x(),
        topPanel->y()+topPanel->h(),
        topPanel->w(),
        1
    );

    log("[EDITOR] - Making bottom separator...");
    Aether::Rectangle* bottomSeparator = new Aether::Rectangle(
        topPanel->x(),
        BOTTOM_SEPARATOR_Y,
        topPanel->w(),
        1
    );

    topSeparator->setColour(SEPARATOR_COLOUR);
    bottomSeparator->setColour(SEPARATOR_COLOUR);

    // align everything
    log("[EDITOR] - Aligning elements...");
    logoSeparator->setX(
        sidebarFrame->x()+(sidebarFrame->w()/2)
        -(logoSeparator->w()/2)
    );

    logoImg->setX(logoSeparator->x());
    logoText->setX(logoImg->x()+logoImg->textureWidth()+TEXT_X_OFFSET);

    logoSeparator->setY(topSeparator->y());
    logoText->setY(logoSeparator->y()-(logoText->textureHeight())-TEXT_Y_OFFSET);
    logoImg->setY(logoText->y());

    // copyright text
    log("[EDITOR] - Making copyright text...");
    Aether::Text* copyrightAndDisclaimer = new Aether::Text(
        logoSeparator->x(),
        sidebarFrame->h(),
        "RUNE © 2026 tderop",
        12
    );

    copyrightAndDisclaimer->setY(sidebarFrame->h()-copyrightAndDisclaimer->textureHeight()-TEXT_Y_OFFSET);
    copyrightAndDisclaimer->setColour(MUTED_TEXT_COLOR);

    // build tab buttons
    log("[EDITOR] - Making tab button container...");
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
    log("[EDITOR] - Making tab buttons...");
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
                positionTabSelectIndicator(selIndicator, this->tabButtons.at(tabIdx));
                this->showTab(tabName);
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
    log("[EDITOR] - Positioning select indicator and showing tab...");
    positionTabSelectIndicator(selIndicator, tabButtons.at(0));

    // adjust height of list accordingly
    log("[EDITOR] - Adjusting height of tab button container...");
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

    // the editor scroll
    Aether::List* editorScroll = new Aether::List(
        sidebarFrame->w(),
        topPanel->h(),
        WINDOW_RESOLUTION_X-sidebarFrame->w(),
        (WINDOW_RESOLUTION_Y-topPanel->h())-(WINDOW_RESOLUTION_Y-bottomSeparator->y()) // overlap bug fix
    );

    // the control bar
    Aether::ControlBar* controlBar = new Aether::ControlBar();
    controlBar->addControl(Aether::Button::MINUS, "Exit");
    controlBar->addControl(Aether::Button::PLUS, "Save");

    // add all elements
    log("[EDITOR] - Adding elements...");
    this->addElement(background);
    this->addElement(controlBar);

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

    this->addElement(topSeparator);
    this->addElement(bottomSeparator);

    this->addElement(copyrightAndDisclaimer);

    this->addElement(editorScroll);
    this->editorScroll = editorScroll;

    // build the popup list
    this->popupList = new Aether::PopupList("Edit Value"); // change to use readable name
    this->popupList->setBackLabel("Back");
    this->popupList->setOKLabel("OK");
    this->popupList->setBackgroundColour(BACKGROUND_COLOUR);
    this->popupList->setLineColour(SEPARATOR_COLOUR);
    this->popupList->setListLineColour(Aether::Colour{0, 0, 0, 0});
    this->popupList->setTextColour(TEXT_COLOUR);

    this->widgetOverlay = new WidgetOverlay();
    
    log("[EDITOR] - Screen built successfully");
}

void EditorScreen::showTab(std::string tabName){
    // if new tab is old tab, ignore
    if (this->selectedTab == tabName){
        log("[EDITOR] - Already selected tab '"+tabName+"'! Ignoring...");
        return;
    }

    log("[EDITOR] - Cleaning temporary elements...");
    this->trove->clear();

    log("[EDITOR] - Resetting selected menu, setting selected tab, and clearing menu labels...");
    this->selectedMenu = 0;
    this->selectedTab = tabName;
    this->menuLabels.clear();

    EditorTab tabMeta = Rune::Editor::TabData.at(tabName);
    
    int totalMenus = tabMeta.menus.size();
    int totalTextureLength = 0;

    log("[EDITOR] - Initialisaing local menuLabels...");
    std::vector<Aether::BorderButton*> menuLabels;
    int idx = 0;
    int textPadding = TEXT_X_OFFSET*3;

    log("[EDITOR] - Building menu labels...");
    for (std::string menuName : tabMeta.menus){
        // std::string fullMenuName = menuName;
        Aether::BorderButton* menuText = new Aether::BorderButton(0, 0, 196, 48, 1, menuName, 18, [this, idx](){
            this->showMenu(idx);
        });

        if (idx != 0){
            Aether::BorderButton* prevLabel = menuLabels.at(idx-1);
            menuText->setX(
                prevLabel->x()+prevLabel->w()+(textPadding)
            );

            // if ((int)fullMenuName.size() < MAX_TITLE_LENGTH){
            //     menuText->setX(menuText->x()+(6*(MAX_TITLE_LENGTH-menuName.size())));
            // }
        }

        totalTextureLength += menuText->w();
        menuLabels.push_back(menuText);

        idx++;
    }

    int isOdd = totalMenus%2;
    int middleIdx = ((totalMenus/2)-1)+isOdd;

    int topPanelCenterX = (this->topPanel->x())+(this->topPanel->w()/2);
    int xOffset;

    Aether::BorderButton* centerLabel = menuLabels.at(middleIdx);

    log("[EDITOR] - Middle Index: "+std::to_string(middleIdx));
    log("[EDITOR] - Calculating X offset...");

    if (isOdd){
        xOffset = topPanelCenterX-(centerLabel->x())-(centerLabel->w()/2);
    }
    else{
        Aether::BorderButton* nextLabel = menuLabels.at(middleIdx+1);
        xOffset = topPanelCenterX-(centerLabel->x())-(centerLabel->w())-(textPadding/2);
    }

    // transform all labels and display them
    int topPanelCenterY = (this->topPanel->y())+(this->topPanel->h()/2);
    int menuLabelY = topPanelCenterY-(menuLabels.at(0)->h()/2);

    log("[EDITOR] - Repositioning menu labels...");
    for (Aether::BorderButton* menuLabel : menuLabels){
        menuLabel->setXY(
            menuLabel->x()+xOffset,
            menuLabelY
        );

        menuLabel->setTextColour(TEXT_COLOUR);
        menuLabel->setBorderColour(Aether::Colour{0, 0, 0, 0});
        
        log("[EDITOR] - Adding element to trove...");
        this->trove->addElement(menuLabel);
        this->menuLabels.push_back(menuLabel);

        this->addElement(menuLabel);
    }

    log("[EDITOR] - Successfully repositioned all menu labels.");

    this->showMenu(0);

    log("[EDITOR] - Tab shown successfully!");
    log("==========================================================");
}

void EditorScreen::showMenu(int menuIdx){
    // check if idx exists
    if (menuIdx < 0 || menuIdx > (int)this->menuLabels.size()-1){
        log("[EDITOR] - MenuIdx: "+std::to_string(menuIdx)+", either less than 0 or greater than "+std::to_string((int)this->menuLabels.size()-1)+".");
        return;
    }

    // no.
    // if (menuIdx == this->selectedMenu){
    //     log("[EDITOR] - Already viewing menu '"+std::to_string(menuIdx)+"'! Ignoring...");
    //     return;
    // }

    log("[EDITOR] - Preparing to show menu at index "+std::to_string(menuIdx));
    this->selectedMenu = menuIdx;

    if (this->menuLabelSelIndicator == nullptr){
        log("[EDITOR] - Creating menu label select indicator...");
        this->menuLabelSelIndicator = new Aether::Rectangle(0, 0, 60, 2);
        this->menuLabelSelIndicator->setColour(TOP_R);

        this->addElement(menuLabelSelIndicator);
        log("[EDITOR] - Menu label select indicator created!");
    }

    log("[EDITOR] - Finding selected button...");
    Aether::BorderButton* selectedBtn = this->menuLabels.at(menuIdx);

    // reposition the select indicator
    menuLabelSelIndicator->setXY(
        selectedBtn->x()+(int)(selectedBtn->w()/2.0)-(int)(this->menuLabelSelIndicator->w()/2.0),
        selectedBtn->y()+selectedBtn->h()-(this->menuLabelSelIndicator->h())-TEXT_X_OFFSET
    );

    // recolor text
    log("[EDITOR] - Recolouring text...");
    for (Aether::BorderButton* otherBtn : this->menuLabels){
        if (otherBtn != selectedBtn){
            otherBtn->setTextColour(TEXT_COLOUR);
        }
        else{
            otherBtn->setTextColour(TOP_R);
        }
    }

    // get more menu data
    log("[EDITOR] - Getting menu data...");

    EditorTab& thisTabData = Rune::Editor::TabData.at(this->selectedTab);
    const std::string& menuName = thisTabData.menus.at(menuIdx);
    log("[EDITOR] - Got menu name and selected tab data: '"+menuName+"'");
    
    // get this tab's entries
    log("[EDITOR] - Finding entries for menu: '"+menuName+"'...");
    auto it = thisTabData.menuEntries.find(menuName);

    log("[EDITOR] - Find operation complete.");

    const std::vector<std::string>* menuEntries = nullptr;

    if (it == thisTabData.menuEntries.end()){
        log("[EDITOR] - No entries configured for menu '"+menuName+"'.");
        log("----------------------------------");
    } else {
        log("[EDITOR] - Found the menu entries.");
        menuEntries = &it->second;
    }

    // clear all old widgets
    log("[EDITOR] - Cleaning old widgets...");
    if (this->tempWidgets.size() > 0){
        this->cleanEditorWidgets();
    }

    // non-copying loop through the entries
    log("[EDITOR] - Building new widgets...");
    if (menuEntries == nullptr){
        log("[EDITOR] - Menu shown with no widgets.");
        return;
    }
    log("[EDITOR] - menuEntries is NOT null, creating entry widgets...");

    for (const std::string& saveKey : *menuEntries){
        // Handling bespoke widgets
        this->makeWidget(saveKey);
    }

    log("[EDITOR] - Built all widgets!");

    log("[EDITOR] - Menu shown successfully!");
    log("----------------------------------");
}

Aether::PopupList* EditorScreen::getPopupList(){
    return this->popupList;
}

void EditorScreen::editSaveFile(Rune::Save::File* saveFile){
    this->cleanEditorWidgets();
    if (saveFile == nullptr){
        this->saveFile = nullptr;
        return;
    }

    this->ownedSaveFile = *saveFile;
    this->saveFile = &this->ownedSaveFile;
    this->showTab(this->tabs.at(0));
}

void EditorScreen::onLoad(){
    // show menu 0
    
    // wire up inputs
    InputSystem& inputService = InputSystem::getService();

    // navigate to next menu when pressing R
    inputService.bind("NextMenu", InputObject(InputType::Gamepad, HidNpadButton_R));
    inputService.connect("NextMenu", [this](InputSystem& inputService, InputState state){
        this->nextMenu(inputService, state);
    });

    // navigate to previous menu when pressing L
    inputService.bind("PrevMenu", InputObject(InputType::Gamepad, HidNpadButton_L));
    inputService.connect("PrevMenu", [this](InputSystem& inputService, InputState state){
        this->prevMenu(inputService, state);
    });

    // exit the editor
    inputService.bind("ExitEditor", InputObject(InputType::Gamepad, HidNpadButton_Minus));
    inputService.connect("ExitEditor", [this](InputSystem& inputService, InputState state){
        this->app->showScreen("Menu");
    });
}

void EditorScreen::onUnload(){
    // disconnect input listeners
    InputSystem& inputService = InputSystem::getService();

    inputService.unbind("NextMenu");
    inputService.unbind("PrevMenu");
    inputService.unbind("ExitEditor");

    // clear trove
    this->trove->clear();

    // unset the save file
    this->saveFile = nullptr;
    this->selectedTab = ""; // ez fix

    log("============== [EDITOR] - Screen successfully unloaded. ==============");
}