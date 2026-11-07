#include <Aether/Aether.hpp>
#include <Editor/WidgetOverlay.hpp>
#include <Config.hpp>

const int DEV_NOTE_WRAP_WIDTH = 480;

void WidgetOverlay::clearTrove(){
    this->_list = nullptr;
    this->_trove->clear();
}

void WidgetOverlay::addTempElement(Aether::Element* element){
    this->_trove->addElement(element);
    this->addElement(element);
}

void WidgetOverlay::addList(){
    Aether::List* list = new Aether::List(30, 322, 1220, 326);
    this->addTempElement(list);
    this->_list = list;
}

Aether::List* WidgetOverlay::list(){
    return this->_list;
}

void WidgetOverlay::setTitleLabel(const std::string &title){
    if (this->_titleLabel == nullptr){
        return;
    }

    this->_titleLabel->setString(title);
}

void WidgetOverlay::closeWidgetOverlay(bool isSave){
    this->onCloseSignal->fire(isSave);

    this->setDeveloperNote("");

    this->_devNoteText->setXY(36, this->_panel->y()+72+36);
    this->_devNoteText->setWrapWidth(DEV_NOTE_WRAP_WIDTH);
    this->_devNoteRect->setX(this->_devNoteText->x());

    this->clearTrove();
    this->close();
}

Aether::Rectangle* WidgetOverlay::panel(){
    return this->_panel;
}

void WidgetOverlay::setHintVisible(bool hintVisible){
    this->_hintVisible = hintVisible;
    this->_devNoteRect->setHidden(!this->_hintVisible);
    this->_devNoteText->setHidden(!this->_hintVisible);
}

bool WidgetOverlay::hintVisible(){
    return this->_hintVisible;
}

void WidgetOverlay::setDeveloperNote(const std::string& note){
    if (note.empty()){
        // Hide the info button and the hint text&separator if no note 
        this->_infoButton->setHidden(true);
        this->setHintVisible(false);

        return;
    } 
    else {
        // Show the info button since there is now a hint available
        this->_infoButton->setHidden(false);
    }

    // Set the text
    this->_devNoteText->setString(note);

    // Reposition the separator
    this->_devNoteRect->setY(this->_devNoteText->y()+this->_devNoteText->textureHeight()+8);
}

WidgetOverlay::WidgetOverlay(const std::string &title){
    this->_trove = new Trove();
    this->onCloseSignal = new Signal<bool>();

    // the main panel
    Aether::Rectangle* panel = new Aether::Rectangle(
        0,
        250,
        OVL_PANEL_WIDTH,
        OVL_PANEL_HEIGHT
    );

    panel->setColour(BACKGROUND_COLOUR);
    this->addElement(panel);
    this->_panel = panel;

    // the title
    Aether::Text* titleLabel = new Aether::Text(
        72,
        278,
        title,
        26
    );

    this->_titleLabel = titleLabel;
    this->addElement(titleLabel);

    // the info button
    this->_infoButton = new Aether::HelpButton(
        this->_titleLabel->x() - 30 - 12, this->_titleLabel->y() - 2,
        30, [](){}
    );

    // hide by default
    this->_infoButton->setColour(Aether::Colour{255, 255, 255, 150});
    this->_infoButton->setHidden(true);

    this->addElement(this->_infoButton);

    // dev text and separator rect
    this->_devNoteText = new Aether::TextBlock(
        36,
        this->_panel->y()+72+36,
        "",
        14,
        480
    );

    this->_devNoteRect = new Aether::Rectangle(
        36, 0,
        320, 2
    );

    this->_devNoteText->setColour(MUTED_TEXT_COLOR);
    this->_devNoteRect->setColour(SEPARATOR_COLOUR);
    this->_devNoteText->setHidden(true);
    this->_devNoteRect->setHidden(true);
    this->addElement(this->_devNoteRect);
    this->addElement(this->_devNoteText);

    // the top separator
    Aether::Rectangle* topSeparator = new Aether::Rectangle(
        30,
        322,
        1220,
        1
    );

    topSeparator->setColour(SEPARATOR_COLOUR);
    this->addElement(topSeparator);

    // bottom separator
    Aether::Rectangle* bottomSeparator = new Aether::Rectangle(
        30,
        648,
        1220,
        1
    );

    bottomSeparator->setColour(SEPARATOR_COLOUR);
    this->addElement(bottomSeparator);

    // control bar
    Aether::ControlBar* controlBar = new Aether::ControlBar();
    controlBar->addControl(Aether::Button::PLUS, "Save");
    controlBar->addControl(Aether::Button::B, "Cancel");

    this->addElement(controlBar);

    /**
     * Handle saving changes
     */
    this->onButtonPress(Aether::Button::PLUS, [this](){
        this->closeWidgetOverlay(true);
    });

    /**
     * Just close the menu when B is pressed
     */
    this->onButtonPress(Aether::Button::B, [this](){
        this->closeWidgetOverlay(false);
    });

    /**
     * Show/hide hint when question mark is pressed
     */
    this->_infoButton->onPress([this](){
        this->setHintVisible(!this->hintVisible());
    });
}

void WidgetOverlay::setDeveloperNoteX(int x){
    this->_devNoteText->setX(x);
    this->_devNoteRect->setX(x);
}

void WidgetOverlay::setDeveloperNoteWrap(int w){
    this->_devNoteText->setWrapWidth(w);
}

WidgetOverlay::~WidgetOverlay(){
    this->onCloseSignal->clear();
    
    delete this->onCloseSignal;
    delete this->_trove;
}