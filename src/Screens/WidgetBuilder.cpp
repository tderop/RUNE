#include <WidgetBuilder.hpp>
#include <Config.hpp>
#include <Utils/Utils.hpp>

// yum
EditorWidget::EditorWidget(Rune::Save::Entry* saveEntry, Aether::PopupList* popupList, Aether::Window* window) 
    : editButton(nullptr), trove(nullptr), saveEntry(saveEntry), popupList(popupList), window(window),
      _isLoaded(false){
    
    // init
    if (saveEntry == nullptr){
        log("[!! EDITOR WIDGET !!] - Cannot build widget for a missing save entry.");
        return;
    }

    this->trove = new Trove();
    log("[EditorWidget] - Trove created!");

    // find the widget builder for this datatype
    auto it = WIDGET_BUILDERS.find(saveEntry->dataType);

    // don't build widget if type doesn't exist
    if (it == WIDGET_BUILDERS.end()){
        return;
    }

    auto widgetBuilder = it->second;
    widgetBuilder(this);

    this->_isLoaded = true;
    log("[WIDGET "+saveEntry->dataName+"] - Completed widget build function. Widget now successfully loaded.");
}

void EditorWidget::clearTrove(){
    if (!this->_isLoaded){
        return;
    }

    log("[WIDGET "+this->saveEntry->dataName+"] - Clearing trove.");
    this->trove->clear();
}

std::vector<AetherElement> EditorWidget::getAllElements(){
    return this->elements;
}

void EditorWidget::addAllElements(Aether::Element*  parentElement){
    log("[WIDGET "+this->saveEntry->dataName+"] - Adding all elements to screen...");
    for (AetherElement elm : this->elements){
        std::visit([parentElement](auto&& arg){
            Aether::Element* toAdd = arg;
            parentElement->addElement(toAdd);
            toAdd->setParent(parentElement);
        }, elm);
    }

    log("[WIDGET "+this->saveEntry->dataName+"] - Successfully added all widget elements!");
    // clear queued elements
    // this->elements.clear();
}

bool EditorWidget::isLoaded(){
    return this->_isLoaded;
}

EditorWidget::~EditorWidget(){
    if (this->trove != nullptr){
        this->trove->clear();
        delete this->trove;
    }
}

void EditorWidget::addToElements(AetherElement element){
    log("[EditorWidget] - Adding element to vector...");
    this->elements.push_back(element);

    log("[EditorWidget] - Adding element to trove...");
    this->trove->addElement(element); // why?
}

void EditorWidget::setEditButton(Aether::ListOption* editButton){
    log("[EditorWidget] - Setting edit button...");
    this->editButton = editButton;

    log("[EditorWidget] - Adding edit button to elements...");
    this->addToElements(AetherElement(editButton));
}

Aether::ListOption* EditorWidget::getEditButton(){
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