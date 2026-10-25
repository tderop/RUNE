#include <Aether/Aether.hpp>
#include <Editor/WidgetOverlay.hpp>
#include <Config.hpp>

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

    this->clearTrove();
    this->close();
}

Aether::Rectangle* WidgetOverlay::panel(){
    return this->_panel;
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
}

WidgetOverlay::~WidgetOverlay(){
    this->onCloseSignal->clear();
    
    delete this->onCloseSignal;
    delete this->_trove;
}