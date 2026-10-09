#include <switch.h>
#include <vector>

#include <Screens/Menu.hpp>
#include <Config.hpp>
#include <Save/Save.hpp>
#include <Utils/Utils.hpp>

#include <Screens/Editor.hpp>


int SAVE_ICON_SIZE = 16; // 16x16 pixels for image icons (star, hard mode)

/**
 * @brief The Menu constructor
 */
Menu::Menu(Aether::Window* window) : window(window){}
Menu::~Menu(){}

/**
 * @brief Create a Scrollable listing all save slots
 */
void Menu::showSaveSelect(){
    // new scrollable
    Aether::List* scrollable = new Aether::List(
        WINDOW_RESOLUTION_X/2,
        TOP_SEPARATOR_Y+1,
        1200,
        BOTTOM_SEPARATOR_Y-TOP_SEPARATOR_Y-2
    );

    this->saveSlotScroll = scrollable;

    // centre
    scrollable->setX(scrollable->x()-scrollable->w()/2);
    scrollable->setY(TOP_SEPARATOR_Y+1);
    scrollable->setH(
        BOTTOM_SEPARATOR_Y-TOP_SEPARATOR_Y-2
    );

    scrollable->setCatchup(11);
    scrollable->setScrollBarColour(SEPARATOR_COLOUR);
    scrollable->setShowScrollBar(true);

    // NEXT SESSION: FIND OUT HOW TO MAKE LISTS SCROLL WITH CONTROLLER NAVIGATION
    // check NXTM source
    scrollable->setWrapAround(true);

    // get save slots
    std::vector<Rune::Save::Slot> runeSaveSlots;
    Rune::Save::GetSaveSlots(&runeSaveSlots);

    if (runeSaveSlots.size() == 0){
        log("[MENU] - No save slots found!");
        return;
    }

    int i = 0;
    Aether::ListButton* firstButton = nullptr;

    // // show save version
    // little bro set up github NOW 
    // github.com/tderop/RUNE.git

    // get first caption file
    Rune::Save::RawFile rawFirstCaption;
    Rune::Save::Slot firstSaveSlot;

    firstSaveSlot = runeSaveSlots.at(0);
    Result gotRawCaption = Rune::Save::GetSaveRawFile(&rawFirstCaption, firstSaveSlot.captionDataPath);

    if (R_FAILED(gotRawCaption)){
        log("[INIT] - Failed to get save version! (1)");
        return;
    }

    // decode save version
    std::string saveVersion = Rune::Save::GetVersionString(
        Rune::Save::Version(rawFirstCaption.header.version)
    );

    Aether::Text* saveVersionText = new Aether::Text(
        0,
        0,
        "v"+saveVersion,
        24
    );

    // reposition
    saveVersionText->setX(
        MAX_SEPARATOR_X-saveVersionText->textureWidth()-(TEXT_X_OFFSET*2) // bro so much random padding
    );

    saveVersionText->setY(
        TOP_SEPARATOR_Y/2 - (saveVersionText->textureHeight()/2)
    );

    // recolour
    saveVersionText->setColour(MUTED_TEXT_COLOR);

    for (Rune::Save::Slot saveSlot : runeSaveSlots){
        // get save data
        Rune::Save::RawFile rawCaptionFile;
        Result rc = Rune::Save::GetSaveRawFile(&rawCaptionFile, saveSlot.captionDataPath);
        if (R_FAILED(rc)){
            log("[MENU] - Failed to get raw caption file for save slot "+std::to_string(saveSlot.index));
            continue;
        }

        // decode save data
        Rune::Save::File captionFile;
        rc = Rune::Save::Decode(&captionFile, rawCaptionFile, *runeMetadata);
        if (R_FAILED(rc)){
            log("[MENU] - Failed to decode caption file for save slot "+std::to_string(saveSlot.index));
        }

        // get relevant info

        // create a new list button
        Aether::ListButton* slotButton = new Aether::ListButton(
            "",
            [saveSlot, this]() mutable{   
                log("[MENU] - Save slot "+std::to_string(saveSlot.index)+" was selected.");
                
                // show loading
                Aether::MessageBox* loadingMessage = new Aether::MessageBox();
                Aether::Text* loadingMessageText = new Aether::Text(
                    0,
                    0,
                    "Decoding Save...",
                    32
                );

                loadingMessageText->setXY(
                    loadingMessage->x()+(loadingMessage->w()/2)-(loadingMessageText->textureWidth()/2),
                    loadingMessage->y()+(loadingMessage->h()/2)-(loadingMessageText->textureHeight()/2)
                );

                loadingMessage->setRectangleColour(MUTED_BACKGROUND_COLOUR);
                loadingMessage->setLineColour(Aether::Colour{0, 0, 0, 0});
                loadingMessageText->setColour(TEXT_COLOUR);

                this->addElement(loadingMessage);
                this->addElement(loadingMessageText);

                // get the save file
                Rune::Save::RawFile rawSaveFile;
                std::string dataPath = saveSlot.gameDataPath;
                // dataPath = "save:/option.sav";
                Result rc = Rune::Save::GetSaveRawFile(&rawSaveFile, dataPath);
                
                if (R_FAILED(rc)){
                    log("[MENU] - Failed to get raw save file for slot "+std::to_string(saveSlot.index)+"!");
                    return;
                }

                // decode the raw   
                Rune::Save::File decodedSave;
                rc = Rune::Save::Decode(&decodedSave, rawSaveFile, *runeMetadata);

                // transition to editor
                // make an application mgr to deal with this, currently if a new 
                // editor is created, then so is another menu when returning
                // so this just leads to infinite memory leakage
                log("[MENU] - Making editor screen...");
                Aether::Screen* editor = new Editor(this->window, decodedSave);

                log("[MENU] - Showing editor screen...");
                this->window->showScreen(editor);
            }
        );


        if (firstButton == nullptr){
            firstButton = slotButton;
        }

        // slotButton->setX(scrollable->x());
        slotButton->setH(WINDOW_RESOLUTION_Y/6);

        scrollable->addElement(slotButton);

        // small padding for the comment
        Aether::ListSeparator* commentPadding = new Aether::ListSeparator(16);
        scrollable->addElement(commentPadding);
        
        // create a comment
        Aether::ListHeading* slotComment = new Aether::ListHeading(
            "Save slot index: "+std::to_string(saveSlot.index)
        );
        scrollable->addElement(slotComment);
        slotComment->setRectColour(MUTED_TEXT_COLOR);
        slotComment->setTextColour(MUTED_TEXT_COLOR);

        // create a list separator
        Aether::ListSeparator* bottomPadding = new Aether::ListSeparator();
        scrollable->addElement(bottomPadding);

        // make save slot image
        Aether::Image* captionImg = new Aether::Image(
            slotButton->x(),
            slotButton->y(),
            saveSlot.captionImgPath,
            Aether::Render::Wait
        );

        // re-scale it
        int size_offset = 14;
        size_t height = slotButton->h()-size_offset; // -10 for size offset

        captionImg->setScaleDimensions(height*CAPTION_IMG_ASPECT_RATIO, height);

        // render the image after setting dimensions
        captionImg->renderSync();

        // move
        captionImg->setXY(
            captionImg->x()+size_offset/2,
            captionImg->y()+size_offset/2
        );

        // add image to slot button
        slotButton->addElement(captionImg);

        // get save version
        

        // location text
        Rune::Save::Entry* saveLocationEntry = Rune::Save::GetEntryFromDataName(captionFile, SAVE_LOCATION_KEY);
        if (std::get<std::string>(saveLocationEntry->value).empty()){ // default to map district if no location
                saveLocationEntry = Rune::Save::GetEntryFromDataName(captionFile, SAVE_DISTRICT_KEY);
        }

        if (saveLocationEntry != nullptr){
            // make location text
            // get localised/translated readable name
            // for now use raw value

            Aether::Text* locationText = new Aether::Text(
                slotButton->x()+(size_offset)+captionImg->w()+TEXT_X_OFFSET,
                slotButton->y()+(slotButton->h()/2),
                Rune::Save::ValueToString(saveLocationEntry->value, false),
                20
            );

            locationText->setY(
                locationText->y()-(locationText->textureHeight()/2)
            );

            locationText->setColour(TEXT_COLOUR);
            slotButton->addElement(locationText);
        }

        // form timestamp
        // Rune::Save::Entry* lastSaveTime_UpperEntry = Rune::Save::GetEntryFromDataName(captionFile, LAST_SAVE_TIME_UPPER_KEY); // seemingly meaningless value, tested dates beyond 2038 and 2106 and did not change, perhaps unimplemented rollover value
        Rune::Save::Entry* lastSaveTime_LowerEntry = Rune::Save::GetEntryFromDataName(captionFile, LAST_SAVE_TIME_LOWER_KEY);
        Aether::Text* timeText = nullptr;

        if (lastSaveTime_LowerEntry != nullptr){
            // make time text
            int32_t lowerTimestamp = std::get<int32_t>(lastSaveTime_LowerEntry->value);
            std::string timestampString = formatUnixTimestamp(lowerTimestamp);

            timeText = new Aether::Text(
                slotButton->x()+(size_offset)+captionImg->w()+TEXT_X_OFFSET,
                slotButton->y(),
                timestampString,
                14
            );

            timeText->setY(
                timeText->y()+TEXT_Y_OFFSET
            );

            timeText->setColour(TEXT_COLOUR);
            slotButton->addElement(timeText);
        }

        // game clear star
        Rune::Save::Entry* gameClearEntry = Rune::Save::GetEntryFromDataName(captionFile, GAME_CLEAR_KEY);
        Aether::Image* starIcon = nullptr;

        if (gameClearEntry != nullptr && timeText != nullptr){
            if (std::get<bool>(gameClearEntry->value)){
                // log("Testing RomFS...");

                // FILE* testFile = fopen("romfs:/assets/star.png", "rb");

                // if (testFile == nullptr) {
                //     log("FAILED to open star.png");
                // } else {
                //     log("Successfully opened star.png!");
                //     fclose(testFile);
                // }

                // make star
                // log("Making star...");

                starIcon = new Aether::Image(
                    0,
                    0,
                    ASSET_PATHS.at("star"),
                    Aether::Render::Wait
                );

                // log("Star constructed!");

                // scale
                starIcon->setScaleDimensions(SAVE_ICON_SIZE, SAVE_ICON_SIZE);

                // render
                starIcon->renderSync();
                // log("Star rendered!");

                // reposition
                starIcon->setXY(
                    timeText->x()+timeText->textureWidth()+TEXT_X_OFFSET,
                    timeText->y()
                );

                // parent
                slotButton->addElement(starIcon);

                // log("Star created and added!");
            }
        }

        // master mode icon
        Rune::Save::Entry* isMasterModeEntry = Rune::Save::GetEntryFromDataName(captionFile, IS_MASTER_MODE_KEY);
        if (timeText != nullptr && isMasterModeEntry != nullptr && std::get<bool>(isMasterModeEntry->value)){
            // icon
            Aether::Image* masterModeIcon = new Aether::Image(
                0,
                0,
                ASSET_PATHS.at("triforce"),
                Aether::Render::Wait
            );

            // scale
            masterModeIcon->setScaleDimensions(SAVE_ICON_SIZE, SAVE_ICON_SIZE);

            // render
            masterModeIcon->renderSync();

            // reposition
            // different positions dependent on the existence of the game complete icon
            if (starIcon != nullptr){
                masterModeIcon->setXY(
                    timeText->x()+timeText->textureWidth()+(starIcon->textureWidth()*1.5)+TEXT_X_OFFSET,
                    timeText->y()
                );
            } else {
                masterModeIcon->setXY( // no icon, no need to account for star
                    timeText->x()+timeText->textureWidth()+TEXT_X_OFFSET,
                    timeText->y()
                );
            }
            
            // yes this logic is very cursed for a normal grid/list layout but I'm not adding more
            // than two icons and I will stand with this to the end

            slotButton->addElement(masterModeIcon);
        }

        // autosave icon
        Rune::Save::Entry* isAutosaveEntry = Rune::Save::GetEntryFromDataName(captionFile, IS_AUTOSAVE_KEY);
        if (isAutosaveEntry != nullptr && std::get<bool>(isAutosaveEntry->value)){
            // text
            Aether::Text* autosaveText = new Aether::Text(
                0,
                0,
                "Autosave",
                14
            );

            // reposition
            autosaveText->setXY(
                slotButton->x()+slotButton->w()-autosaveText->textureWidth()-TEXT_X_OFFSET,
                slotButton->y()+slotButton->h()-autosaveText->textureHeight()-(TEXT_Y_OFFSET/2)
            );

            // recolour
            autosaveText->setColour(MUTED_TEXT_COLOR);

            // rectangle
            Aether::Rectangle* autosaveLabel = new Aether::Rectangle(
                autosaveText->x()-TEXT_X_OFFSET,
                autosaveText->y()-TEXT_Y_OFFSET/2,
                autosaveText->textureWidth()+TEXT_X_OFFSET*2,
                autosaveText->textureHeight()+TEXT_Y_OFFSET*1
            );

            autosaveLabel->setColour(MUTED_BACKGROUND_COLOUR);

            slotButton->addElement(autosaveLabel);
            slotButton->addElement(autosaveText);
        }
        i++;
    }

    this->addElement(scrollable);
    this->setFocused(scrollable);

    this->addElement(saveVersionText);

    if (firstButton != nullptr){
        scrollable->setFocused(firstButton);
    }

    // finally fixed not scrolling with controller, for some reason had to manually call update
    // log("canScroll="+std::to_string(scrollable->canScroll())+
    // " isTouch="+std::to_string(scrollable->isTouch)+
    // " maxScroll="+std::to_string(scrollable->maxScrollPos())+
    // " focusedNull="+std::to_string(scrollable->focused()==nullptr));
}

/**
 * @brief Add all key elements to the Menu screen.
 */
void Menu::addScreenElements(){
    // add the background
    Aether::Rectangle* background = new Aether::Rectangle(
        0,
        0,
        WINDOW_RESOLUTION_X,
        WINDOW_RESOLUTION_Y
    );
    background->setColour(BACKGROUND_COLOUR);

    // add the top and bottom separators
    Aether::Rectangle* topSeparator = new Aether::Rectangle(
        MIN_SEPARATOR_X,
        TOP_SEPARATOR_Y,
        MAX_SEPARATOR_X - MIN_SEPARATOR_X,
        1
    );
    topSeparator->setColour(SEPARATOR_COLOUR);

    Aether::Rectangle* bottomSeparator = new Aether::Rectangle(
        MIN_SEPARATOR_X,
        BOTTOM_SEPARATOR_Y,
        MAX_SEPARATOR_X - MIN_SEPARATOR_X,
        1
    );
    bottomSeparator->setColour(SEPARATOR_COLOUR);

    // make the control bar
    Aether::ControlBar* controlBar = new Aether::ControlBar();
    controlBar->addControl(Aether::Button::A, "Select");
    controlBar->addControl(Aether::Button::PLUS, "Exit");
    controlBar->setEnabledColour(TEXT_COLOUR);
    controlBar->setDisabledColour(MUTED_TEXT_COLOR);

    // make the title
    Aether::Text* screenTitle = new Aether::Text(
        MIN_SEPARATOR_X+36,
        0,
        "Save Select",
        32
    );
    
    screenTitle->setY(
        TOP_SEPARATOR_Y/2 - (screenTitle->textureHeight()/2)
    );

    screenTitle->setColour(TEXT_COLOUR);

    // title icon
    Aether::Image* titleIcon = new Aether::Image(
        0,
        0,
        ASSET_PATHS.at("check"),
        Aether::Render::Wait
    );

    titleIcon->setScaleDimensions(32, 32);
    titleIcon->renderSync();

    titleIcon->setXY(
        MIN_SEPARATOR_X+8,
        screenTitle->y()
    );

    // lol
    screenTitle->setX(
        titleIcon->x()+titleIcon->textureWidth()+12
    );

    // make the caption
    Aether::Text* captionText = new Aether::Text(
        640,
        74,
        "Breath of the Wild Save Editor for Nintendo Switch",
        12
    );
    captionText->setX(captionText->x()-captionText->w()/2);
    captionText->setColour(MUTED_TEXT_COLOR);

    // show all elements (in z-index order)
    this->addElement(background);
    this->addElement(controlBar);
    this->addElement(screenTitle);
    this->addElement(titleIcon);
    // this->addElement(captionText);
    this->addElement(topSeparator);
    this->addElement(bottomSeparator);

    showSaveSelect();
}

void Menu::onLoad(){
    this->addScreenElements();
}

/**
 * @brief Clean up all elements on unload to prevent memory leaks.
 */
void Menu::onUnload(){
    this->removeAllElements();
}

// do vector decoding