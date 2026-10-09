#include <WidgetBuilder.hpp>
#include <Config.hpp>
#include <Utils/Utils.hpp>

// yum
EditorWidget::EditorWidget(Rune::Save::Entry* saveEntry, Aether::PopupList* popupList, Aether::Window* window) 
    : saveEntry(saveEntry), popupList(popupList), window(window){
    
    // init
    this->_isLoaded = false;
    this->trove = new Trove();
    log("[EditorWidget] - Trove created!");

    if (saveEntry == nullptr){
        log("[!! EDITOR WIDGET !!] - Cannot build widget for a missing save entry.");
        return;
    }

    // find the widget builder for this datatype
    auto it = WIDGET_BUILDERS.find(saveEntry->dataType);

    // don't build widget if type doesn't exist
    if (it == WIDGET_BUILDERS.end()){
        return;
    }

    auto widgetBuilder = it->second;
    widgetBuilder(this);

    this->_isLoaded = true;
}

void EditorWidget::clearTrove(){
    if (!this->_isLoaded){
        return;
    }

    log("[EditorWidget "+this->saveEntry->dataName+"] - Clearing trove.");
    this->trove->clear();
}

std::vector<AetherElement> EditorWidget::getAllElements(){
    return this->elements;
}

void EditorWidget::addAllElements(Aether::Element*  parentElement){
    for (AetherElement elm : this->elements){
        Aether::Element* toAdd = std::get<Aether::Element*>(elm);
        parentElement->addElement(toAdd);
        toAdd->setParent(parentElement);
    }

    // clear queued elements
    // this->elements.clear();
}

bool EditorWidget::isLoaded(){
    return this->_isLoaded;
}

EditorWidget::~EditorWidget(){
    this->trove->clear();
    delete this->trove;
}

void EditorWidget::addToElements(AetherElement element){
    log("[EditorWidget] - Adding element to vector...");
    this->elements.push_back(element);

    log("[EditorWidget] - Adding element to trove...");
    this->trove->addElement(element); // why?
}

void EditorWidget::setEditButton(AetherElement editButton){
    log("[EditorWidget] - Setting edit button...");
    this->editButton = editButton;

    log("[EditorWidget] - Adding edit button to elements...");
    this->addToElements(editButton);
}

AetherElement EditorWidget::getEditButton(){
    return this->editButton;
}

void EditorWidget::setPopupList(Aether::PopupList* popupList){
    this->popupList = popupList;
}

Aether::PopupList* EditorWidget::getPopupList(){
    return this->popupList;
}

Rune::Save::Entry* EditorWidget::getSaveEntry(){
    return this->saveEntry;
}

Aether::Window* EditorWidget::getWindow(){
    return this->window;
}

void EditorWidget::addPopupListEntry(const std::string& entryString, std::function<void()> callback, bool showTicked){
    this->popupList->addEntry(
        entryString,
        callback,
        showTicked
    );

    // this->popupListEntries.push_back(entryString);
}

void EditorWidget::removePopupListEntries(){
    this->popupList->removeEntries();
}

std::string getReadableSaveKeyName(std::string& saveKey){
    if (auto it = READABLE_SAVE_KEYS.find(saveKey); it != READABLE_SAVE_KEYS.end()){ // no contains :pensive:
        return it->second;
    }

    // default to input key if no translation exists
    return saveKey;
}