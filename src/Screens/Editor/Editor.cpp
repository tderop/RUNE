/**
 * @brief The main editor menu used for editing save files.
 */

#include <switch.h>
#include <Aether/Aether.hpp>

#include <Screens/Editor.hpp>
#include <Config.hpp>
#include <Utils/Utils.hpp>
#include <Utils/Logger.hpp>
#include <Input/Input.hpp>
#include <Save/Save.hpp>

#include <vector>
#include <string>

#include <Application/Application.hpp>
#include <Application/Audio.hpp>

// Widgets used to modify values
#include <Editor/EditorWidget.hpp>
#include <Editor/WidgetOverlay.hpp>

// Menus used to hold widgets
#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>

// Constants
int TAB_BUTTON_HEIGHT = 64; // px
int EDITOR_TOP_Y = 72; // px
int MAX_TITLE_LENGTH = 16; // px

float CAPTION_IMAGE_SCALE = 0.9f; // relative width of the image relative to the last ListButton in the tabs list

/**
 * The constructor for the editor screen. Runs once when a new screen is
 * created in the application manager.
 */
EditorScreen::EditorScreen(Rune::Application* app) : app(app) {
    // Make troves
    // This trove stores elements to be destroyed when the *tab* is changed (left sidebar)
    this->pageTrove = new Trove();
    // This trove stores elements to be destroyed when the window's screen is changed (Editor unloads)
    this->screenTrove = new Trove();

    this->onCursorMove([](){
        Rune::Audio::playSound("cursor");
    });
}

/**
 * @brief Delete visible widgets when deleted.
 */
EditorScreen::~EditorScreen(){
    this->cleanMenu();
    this->pageTrove->clear();
    this->screenTrove->clear();
    delete this->pageTrove;
    delete this->screenTrove;
}

/**
 * @brief Reposition a rectangle to align with a button to show it is selected.
 */
void EditorScreen::positionTabSelectIndicator(Aether::Rectangle* selectIndicator, Aether::ListButton* targetBtn){
    selectIndicator->setH(
        (static_cast<float>(targetBtn->h()))*(2.0f/3.0f)
    );

    selectIndicator->setXY(
        targetBtn->x() + (TEXT_X_OFFSET/2.0f),
        (targetBtn->y()) 
        + (static_cast<float>(targetBtn->h())/2.0f) 
        - (static_cast<float>(selectIndicator->h())/2.0f) // yummy
    );

    for (Aether::ListButton* otherBtn : this->tabButtons){
        if (otherBtn != targetBtn){
            otherBtn->setTextColour(TEXT_COLOUR);
        }
        else{
            otherBtn->setTextColour(TOP_R);
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
    Logger::Write("[EDITOR] - Making sidebar frame...");
    Aether::Rectangle* sidebarFrame = new Aether::Rectangle(
        0,
        0,
        WINDOW_RESOLUTION_X / 4,
        WINDOW_RESOLUTION_Y
    );

    sidebarFrame->setColour(EDITOR_SIDEBAR_COLOUR);

    // logo image (UNscrapped?????)
    Logger::Write("[EDITOR] - Making logo image...");
    Aether::Image* logoImg = new Aether::Image(
        0,
        0,
        ASSET_PATHS.at("triforce"),
        Aether::Render::Wait
    );

    logoImg->setScaleDimensions(28, 28);
    logoImg->renderSync();

    // logo text
    Logger::Write("[EDITOR] - Making logo text...");
    Aether::Text* logoText = new Aether::Text(
        0,
        0,
        "RUNE - Editor",
        28
    );

    logoText->setColour(TEXT_COLOUR);

    // RUNE logo separator
    Logger::Write("[EDITOR] - Making logo separator...");
    int logoSeparatorWidth = static_cast<int>(sidebarFrame->w()*(3.0/4.0));
    Aether::Rectangle* logoSeparator = new Aether::Rectangle(
        0,
        0,
        logoSeparatorWidth,
        2
    );

    logoSeparator->setColour(SEPARATOR_COLOUR);

    // background
    Logger::Write("[EDITOR] - Making background...");
    Aether::Rectangle* background = new Aether::Rectangle(
        0,
        0,
        WINDOW_RESOLUTION_X,
        WINDOW_RESOLUTION_Y
    );

    background->setColour(BACKGROUND_COLOUR);

    // top panel
    Logger::Write("[EDITOR] - Making top panel...");
    topPanel = new Aether::Rectangle(
        sidebarFrame->w(),
        0,
        WINDOW_RESOLUTION_X-sidebarFrame->w(),
        EDITOR_TOP_Y
    );

    topPanel->setColour(MUTED_BACKGROUND_COLOUR);

    // separators
    Logger::Write("[EDITOR] - Making top separator...");
    Aether::Rectangle* topSeparator = new Aether::Rectangle(
        topPanel->x(),
        topPanel->y()+topPanel->h(),
        topPanel->w(),
        1
    );

    Logger::Write("[EDITOR] - Making bottom separator...");
    Aether::Rectangle* bottomSeparator = new Aether::Rectangle(
        topPanel->x(),
        BOTTOM_SEPARATOR_Y,
        topPanel->w(),
        1
    );

    topSeparator->setColour(SEPARATOR_COLOUR);
    bottomSeparator->setColour(SEPARATOR_COLOUR);

    // align everything
    Logger::Write("[EDITOR] - Aligning elements...");
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
    Logger::Write("[EDITOR] - Making copyright text...");
    Aether::TextBlock* copyrightAndDisclaimer = new Aether::TextBlock(
        logoSeparator->x(),
        sidebarFrame->h(),
        "RUNE v"+RUNE_VERSION+"\n© 2026 tderop",
        12,
        240
    );

    copyrightAndDisclaimer->setY(sidebarFrame->h()-copyrightAndDisclaimer->textureHeight()-TEXT_Y_OFFSET);
    copyrightAndDisclaimer->setColour(MUTED_TEXT_COLOR);

    // Separator
    Aether::Rectangle* copyrightSeparator = new Aether::Rectangle(
        logoSeparator->x(), copyrightAndDisclaimer->y()-12,
        logoSeparatorWidth, 2
    );

    copyrightSeparator->setColour(SEPARATOR_COLOUR);

    // build tab buttons
    Logger::Write("[EDITOR] - Making tab button container...");
    Aether::Container* tabButtonList = new Aether::Container(
        logoSeparator->x(),
        logoSeparator->y()+36,
        logoSeparator->w(),
        96 //,
        // Aether::Padding::None
    );
    tabButtonList->onCursorMove([](){
        Rune::Audio::playSound("cursor");
    });

    // tabButtonList->setScrollBarColour(SEPARATOR_COLOUR);
    // tabButtonList->setShowScrollBar(false);

    // create tab buttons
    Logger::Write("[EDITOR] - Making tab buttons...");
    this->tabLabelSelIndicator = new Aether::Rectangle(0, 0, 3, 1);
    this->tabLabelSelIndicator->setColour(TOP_R);

    int tabIdx = 0;

    for (std::string tabName : Rune::Editor::Tabs){
        // make a button
        Aether::ListButton* tabButton;
        tabButton = new Aether::ListButton(
            tabName,
            [this, tabName](){
                Logger::Write(tabName+" was selected");

                Rune::Audio::playSound("select");
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
    Logger::Write("[EDITOR] - Positioning select indicator and showing tab...");

    // adjust height of list accordingly
    Logger::Write("[EDITOR] - Adjusting height of tab button container...");
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

    editorScroll->setScrollBarColour(SEPARATOR_COLOUR);

    // the control bar
    Aether::ControlBar* controlBar = new Aether::ControlBar();
    controlBar->addControl(Aether::Button::PLUS, "Save");
    controlBar->addControl(Aether::Button::MINUS, "Exit");
    this->controlBar = controlBar;

    // confirmation message box
    Aether::MessageBox* confirmBox = new Aether::MessageBox();
    Aether::MessageBox* messageBox = new Aether::MessageBox();

    confirmBox->setBodySize(770, 320);
    confirmBox->setRectangleColour(BACKGROUND_COLOUR);

    messageBox->setBodySize(770, 320);
    messageBox->setRectangleColour(BACKGROUND_COLOUR);

    confirmBox->setLineColour(SEPARATOR_COLOUR);
    messageBox->setLineColour(SEPARATOR_COLOUR);

    // modified Aether source so I can access left and right border buttons
    confirmBox->addLeftButton("Yes", [](){});
    confirmBox->addRightButton("No", [](){});

    // Exiting the editor
    this->onButtonPress(Aether::Button::MINUS, [this, confirmBox](){
        confirmBox->setString("Are you sure you want to exit?");
        confirmBox->leftButton()->onPress([this, confirmBox](){
            confirmBox->close();
            this->app->showScreen("Menu");
        });

        confirmBox->rightButton()->onPress([confirmBox](){
            confirmBox->close();
        });

        this->app->window()->addOverlay(confirmBox);
        confirmBox->setFocused(confirmBox->leftButton());
    });

    messageBox->addTopButton("OK", [messageBox](){
        messageBox->close();
    });

    // handle compiling the save when saving
    this->onButtonPress(Aether::Button::PLUS, 
        [this, confirmBox, messageBox]{

        Aether::Window* window = this->app->window();
        Logger::Write("[EDITOR] - Plus pressed, requesting save...");

        confirmBox->setString("Save changes? This will modify this save slot.");
        confirmBox->leftButton()->onPress([this, window, confirmBox, messageBox](){
            // remove the confirm box and show loading
            confirmBox->close();
            auto loadingMsg = this->app->showLoading("Packing values...", true);

            // encode the save file
            Rune::Save::RawFile rawFile;
            Rune::Save::Encode(&rawFile, this->saveFile, [this](size_t cur, size_t max){
                if (max == 0) { return; }
                if (cur % 1280 != 0) { return; }
                this->app->setLoadingProgress(static_cast<float>(cur)/max);
            });

            // compile the save file
            loadingMsg->setString("Storing backup...");
            this->app->setLoadingProgress(0);

            const std::string& path = rawFile.path;
            Result rc = Rune::Save::CompileTo(&rawFile, rawFile.path, 
                [this, loadingMsg, path](size_t cur, size_t max){
                    if (max == 0) { return; }
                    if (cur % 640 != 0) { return; }

                    loadingMsg->setString("Writing to "+path+"...");
                    this->app->setLoadingProgress(static_cast<float>(cur)/max);
                }
            );

            loadingMsg->close();

            if (R_SUCCEEDED(rc)){
                messageBox->setString("Saved! Backups are in the sdcard:/switch/RUNE directory.");
            } else {
                messageBox->setString("An error ocurred when compiling edited save file.");
            }

            window->addOverlay(messageBox);
            messageBox->setFocused(messageBox->topButton());
        });

        confirmBox->rightButton()->onPress([confirmBox]{
            confirmBox->close();
        });

        window->addOverlay(confirmBox);
        confirmBox->setFocused(confirmBox->leftButton());
    });

    // add all elements
    Logger::Write("[EDITOR] - Adding elements...");
    this->addElement(background);
    this->addElement(controlBar);

    this->addElement(topPanel);
    this->addElement(sidebarFrame);

    this->addElement(logoText);
    this->addElement(logoSeparator);
    this->addElement(logoImg);

    this->addElement(tabButtonList);
    this->addElement(this->tabLabelSelIndicator);

    this->addElement(topSeparator);
    this->addElement(bottomSeparator);

    this->addElement(copyrightAndDisclaimer);
    this->addElement(copyrightSeparator);

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
    
    Logger::Write("[EDITOR] - Screen built successfully");
}

void EditorScreen::showTab(std::string tabName){
    // if new tab is old tab, ignore
    if (this->selectedTab == tabName){
        Logger::Write("[EDITOR] - Already selected tab '"+tabName+"'! Ignoring...");
        return;
    }

    Logger::Write("[EDITOR] - Cleaning temporary elements...");
    this->pageTrove->clear();

    Logger::Write("[EDITOR] - Resetting selected menu, setting selected tab, and clearing menu labels...");
    this->selectedMenu = 0;
    this->selectedTab = tabName;
    this->menuLabels.clear();

    EditorTab tabMeta = Rune::Editor::TabData.at(tabName);
    
    int totalMenus = tabMeta.menus.size();
    int totalTextureLength = 0;

    Logger::Write("[EDITOR] - Initialisaing local menuLabels...");
    std::vector<Aether::BorderButton*> menuLabels;
    int idx = 0;
    int textPadding = TEXT_X_OFFSET*3;

    Logger::Write("[EDITOR] - Building menu labels...");
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

    Logger::Write("[EDITOR] - Middle Index: "+std::to_string(middleIdx));
    Logger::Write("[EDITOR] - Calculating X offset...");

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

    Logger::Write("[EDITOR] - Repositioning menu labels...");
    for (Aether::BorderButton* menuLabel : menuLabels){
        menuLabel->setXY(
            menuLabel->x()+xOffset,
            menuLabelY
        );

        menuLabel->setTextColour(TEXT_COLOUR);
        menuLabel->setBorderColour(Aether::Colour{0, 0, 0, 0});
        
        Logger::Write("[EDITOR] - Adding element to trove...");
        this->pageTrove->addElement(menuLabel);
        this->menuLabels.push_back(menuLabel);

        this->addElement(menuLabel);
    }

    Logger::Write("[EDITOR] - Successfully repositioned all menu labels.");

    this->showMenu(0);

    // find the tab index
    size_t tabIdx = 0;
    for (const std::string& tab : Rune::Editor::Tabs){
        if (tab == tabName){
            positionTabSelectIndicator(this->tabLabelSelIndicator, tabButtons.at(tabIdx));
        }

        tabIdx++;
    }

    Logger::Write("[EDITOR] - Tab shown successfully!");
    Logger::Write("==========================================================");
}

void EditorScreen::showMenu(int menuIdx){
    // wrap around if R pressed at end of tabs
    if (menuIdx > (int)this->menuLabels.size()-1){
        menuIdx = 0;
    }

    // wrap around if L pressed at start of tabs
    if (menuIdx < 0){
        menuIdx = (this->menuLabels.size())-1;
    }

    Logger::Write("[EDITOR] - Preparing to show menu at index "+std::to_string(menuIdx));
    this->selectedMenu = menuIdx;

    if (this->menuLabelSelIndicator == nullptr){
        Logger::Write("[EDITOR] - Creating menu label select indicator...");
        this->menuLabelSelIndicator = new Aether::Rectangle(0, 0, 60, 2);
        this->menuLabelSelIndicator->setColour(TOP_R);

        this->addElement(menuLabelSelIndicator);
        Logger::Write("[EDITOR] - Menu label select indicator created!");
    }

    Logger::Write("[EDITOR] - Finding selected button...");
    Aether::BorderButton* selectedBtn = this->menuLabels.at(menuIdx);

    // reposition the select indicator
    menuLabelSelIndicator->setXY(
        selectedBtn->x()+(int)(selectedBtn->w()/2.0)-(int)(this->menuLabelSelIndicator->w()/2.0),
        selectedBtn->y()+selectedBtn->h()-(this->menuLabelSelIndicator->h())-TEXT_X_OFFSET
    );

    // recolor text
    Logger::Write("[EDITOR] - Recolouring text...");
    for (Aether::BorderButton* otherBtn : this->menuLabels){
        if (otherBtn != selectedBtn){
            otherBtn->setTextColour(TEXT_COLOUR);
        }
        else{
            otherBtn->setTextColour(TOP_R);
        }
    }

    // get more menu data
    Logger::Write("[EDITOR] - Getting menu data...");

    EditorTab& thisTabData = Rune::Editor::TabData.at(this->selectedTab);
    const std::string& menuName = thisTabData.menus.at(menuIdx);
    Logger::Write("[EDITOR] - Got menu name and selected tab data: '"+menuName+"'");

    // Get this menu's builder
    auto buildPair = thisTabData.menuBuilders.find(menuName);
    std::string builderName=""; // Initialises to empty string without initialiser but just doing it because I'm a consistency goblin

    // If it exists, set the builder name
    if (buildPair != thisTabData.menuBuilders.end()){
        builderName = buildPair->second;
    }

    // Get this menu's entries
    auto entryPair = thisTabData.menuEntries.find(menuName);
    std::vector<std::string> menuEntries={};

    // If it exists, set the menu's entries
    if (entryPair != thisTabData.menuEntries.end()){
        menuEntries = entryPair->second;
    }

    // Clean the old menu
    this->cleanMenu();

    // Build the new menu
    EditorMenu* menu = new EditorMenu(menuName, this->app->window(), this);

    // Set the save file
    menu->setSaveFile(this->saveFile);

    // Set the scroll for menus that may need it (entries)
    menu->setList(this->editorScroll);

    // Set the save entries (for entries)
    menu->setDataNames(menuEntries);

    // Set the popupList (for entries)
    menu->setPopupList(this->popupList);

    // Set the widgetOverlay (for entries, AGAIN)
    menu->setWidgetOverlay(this->widgetOverlay);

    // Set the controlBar
    menu->setControlBar(this->controlBar);

    // Build the menu
    menu->build(builderName);
    this->editorScroll->setSelectable((builderName!="Inventory"));
    
    // Set current menu
    this->currentMenu = menu;

    Logger::Write("[EDITOR] - Menu shown successfully!");
    Logger::Write("----------------------------------");
}

Aether::PopupList* EditorScreen::getPopupList(){
    return this->popupList;
}

void EditorScreen::editSaveSlot(Rune::Save::Slot* saveSlot){
    // Delete current menu
    this->cleanMenu();

    if (saveSlot == nullptr){
        // saveSlot is a pointer for slotCopy
        this->saveSlot = nullptr;

        // Free save file memory
        delete this->saveFile;
        delete this->captionFile;

        // Create new empty files to be populated
        this->saveFile = new Rune::Save::File();
        this->captionFile = new Rune::Save::File();

        // Empty selected tab string
        this->selectedTab = "";
        return;
    }

    // Save a copy of the slot
    this->slotCopy = *saveSlot;
    this->saveSlot = &this->slotCopy;

    // Get the raw save file
    Rune::Save::RawFile rawSaveFile;
    std::string saveFileDataPath = saveSlot->gameDataPath;
    Result rc = Rune::Save::GetSaveRawFile(&rawSaveFile, saveFileDataPath);
    if (R_FAILED(rc)){
        Logger::Write("[EDITOR] - Failed to get raw save file for slot "+std::to_string(saveSlot->index)+"!");
        return;
    }

    // Decode the raw save file
    Logger::Write("[EDITOR] - Decoding raw save file...");
    rc = Rune::Save::Decode(this->saveFile, rawSaveFile, *runeMetadata, 
        [this](size_t cur, size_t max){
            if (cur % 640 != 0) { return; }
            this->app->setLoadingProgress(static_cast<float>(cur)/max);
        }
    );

    if (R_FAILED(rc)){
        Logger::Write("[EDITOR] - Failed to decode save file for slot "+std::to_string(saveSlot->index)+"!");
        return;
    }

    Logger::Write("[EDITOR] - Raw save file decoded successfully!");

    // Get the raw caption file
    Logger::Write("[EDITOR] - Getting raw caption file...");
    Rune::Save::RawFile rawCaptionFile;
    std::string captionFileDataPath = saveSlot->captionDataPath;
    rc = Rune::Save::GetSaveRawFile(&rawCaptionFile, captionFileDataPath);
    if (R_FAILED(rc)){
        Logger::Write("[EDITOR] - Failed to get raw caption file for slot "+std::to_string(saveSlot->index)+"!");
        return;
    }

    // Decode the raw caption file
    Logger::Write("[EDITOR] - Decoding raw caption file...");
    rc = Rune::Save::Decode(this->captionFile, rawCaptionFile, *runeMetadata);
    if (R_FAILED(rc)){
        Logger::Write("[EDITOR] - Failed to decode caption file for slot "+std::to_string(saveSlot->index)+"!");
        return;
    }

    Logger::Write("[EDITOR] - Raw caption file decoded successfully!");

    // Load the caption image
    // Get the last tab button for alignment
    Aether::ListButton* lastTabBtn = this->tabButtons.at(this->tabButtons.size()-1);

    // The Y offset of the caption image relative to the final tab button
    int captionImgOffsetY = 48;

    Aether::Image* captionImg = new Aether::Image(
        lastTabBtn->x(),
        lastTabBtn->y()+lastTabBtn->h()+captionImgOffsetY,
        this->saveSlot->captionImgPath,
        Aether::Render::Wait
    );

    // Scale the image
    captionImg->setScaleDimensions(
        static_cast<float>(lastTabBtn->w()) * CAPTION_IMAGE_SCALE,
        ((static_cast<float>(lastTabBtn->w()) / 16.0f) * 9.0f) * CAPTION_IMAGE_SCALE
    );

    // Reposition the image 
    captionImg->setX(
        lastTabBtn->x() + lastTabBtn->w()/2 - captionImg->scaleWidth()/2
    );

    // Render the image
    captionImg->renderSync();
    
    // Add the image to the screen and the screen trove (clears when screen is unloaded)
    this->screenTrove->addElement(captionImg);
    this->addElement(captionImg);

    // Get the caption timestamp
    std::string timestamp = formatUnixTimestamp(
        std::get<int32_t>(Rune::Save::GetEntryFromDataName(
            *this->captionFile, LAST_SAVE_TIME_LOWER_KEY)->value // lovely
        )
    );

    bool isAutosave = std::get<bool>(
        Rune::Save::GetEntryFromDataName(*this->captionFile, IS_AUTOSAVE_KEY)->value
    );

    std::string locationId = std::get<std::string>(
        Rune::Save::GetEntryFromDataName(*this->captionFile, SAVE_LOCATION_KEY)->value
    );

    // Make the caption timestamp text
    Aether::TextBlock* captionImgTimestamp = new Aether::TextBlock(
        captionImg->x(), 0,
        "Save Time: "+timestamp+
        "\nSave Type: "+((isAutosave) ? "Autosave" : "Manual")+
        "\nLocation: "+Rune::Save::GetReadableLocationName(locationId),
        14,
        240
    );

    // Reposition the timestamp text
    captionImgTimestamp->setXY(
        captionImg->x(),
        captionImg->y()+captionImg->scaleHeight()+12
    );

    // Set the text colour
    captionImgTimestamp->setColour(Aether::Colour{255, 255, 255, 200});

    // Add the text to the screen and its trove
    this->screenTrove->addElement(captionImgTimestamp);
    this->addElement(captionImgTimestamp);

    // Add autosave lavel
    if (isAutosave){
        Aether::Text* autosaveText = new Aether::Text(
            0,
            0,
            "Autosave",
            12
        );

        int autosaveOffsetX = 6, autosaveOffsetY = 3;

        // Reposition the autosave text
        autosaveText->setXY(
            captionImg->x() + (captionImg->scaleWidth()) - (autosaveText->textureWidth()) - autosaveOffsetX,
            captionImg->y() + (captionImg->scaleHeight()) - (autosaveText->textureHeight()) - autosaveOffsetY
        );

        // The background rect
        Aether::Rectangle* autosaveRect = new Aether::Rectangle(
            autosaveText->x() - autosaveOffsetX,
            autosaveText->y() - autosaveOffsetY,
            autosaveText->textureWidth() + autosaveOffsetX*2,
            autosaveText->textureHeight() + autosaveOffsetY*2
        );

        // Make the label darker than the background
        autosaveRect->setColour(Aether::Colour{0, 0, 0, 150});

        // Add background to the screen
        this->screenTrove->addElement(autosaveRect);
        this->addElement(autosaveRect);

        // Then add autosave text
        this->screenTrove->addElement(autosaveText);
        this->addElement(autosaveText);
    }
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

    // Show the first tab
    this->showTab(Rune::Editor::Tabs.at(0));
}

void EditorScreen::onUnload(){
    // disconnect input listeners
    InputSystem& inputService = InputSystem::getService();

    inputService.unbind("NextMenu");
    inputService.unbind("PrevMenu");
    inputService.unbind("ExitEditor");

    // clear trove
    this->pageTrove->clear();
    this->screenTrove->clear();

    // Edit null
    this->editSaveSlot(nullptr);

    Logger::Write("============== [EDITOR] - Screen successfully unloaded. ==============");
}

void EditorScreen::update(unsigned int dt){
    Aether::Screen::update(dt);

    if (this->currentMenu != nullptr){
        if (this->currentMenu->reloadQueued()){
            this->currentMenu->reload();
        }
    }
}