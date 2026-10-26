#include <Config.hpp>
#include <Utils/Utils.hpp>
#include <Json/Json.hpp>

#include <Editor/EditorWidget.hpp>
#include <Editor/WidgetBuilder.hpp>
#include <Editor/EditorMenu.hpp>

#include <any>

#include <switch.h>

const int MAX_VALUE_LENGTH = 28; // characters
// yum
EditorWidget::EditorWidget(Rune::Save::Entry* saveEntry, EditorMenu* menu, bool renderAsync, std::vector<std::any> meta) 
    : saveEntry(saveEntry), menu(menu), meta(meta){
    
    // init
    this->renderAsync = renderAsync;
    this->window = this->menu->window();

    if (saveEntry == nullptr){
        log("[!! EDITOR WIDGET !!] - Cannot build widget for a missing save entry.");
        return;
    }

    this->trove = new Trove();
    log("[EditorWidget] - Trove created!");

    // find the widget builder for this datatype
    log("[EditorWidget] - Finding widget builder for dataType '"
        +std::string(Rune::Save::DataTypeName(saveEntry->dataType))+
    "'");

    // find the meta entry
    log("[EditorWidget] - Getting meta entry for '"+saveEntry->dataName+"'...");

    Rune::Save::MetaEntry metaEntry;
    Result rc = Rune::Save::GetMetaEntry(*runeMetadata, saveEntry->id, &metaEntry);
    if (R_FAILED(rc)){
        log("[EditorWidget] - Failed to get meta entry for '"+saveEntry->dataName+"'!");
        return; // failed to get meta entry
    }

    // Search for the widget builder
    // First, check editor override
    log("[EditorWidget] - Initialising widget builder func...");
    WidgetBuilderFunc widgetBuilder = nullptr;

    if (!metaEntry.editor.empty()){
        log("[EditorWidget] - Meta entry shows unique editor! Searching for editor...");
        auto it = UNIQUE_WIDGET_BUILDERS.find(metaEntry.editor);
        if (it != UNIQUE_WIDGET_BUILDERS.end()){
            // builder found
            log("[EditorWidget] - Builder found! Assigning function.");
            widgetBuilder = it->second;
        }

        log("[EditorWidget] - Unique builder not found.");
    }

    // Fallback to generic editor if no unique editor was found
    if (widgetBuilder == nullptr){
        auto it = WIDGET_BUILDERS.find(saveEntry->dataType);

        // don't build widget if type doesn't exist
        if (it == WIDGET_BUILDERS.end()){
            log("[EditorWidget] - Builder not found for '"
                +std::string(Rune::Save::DataTypeName(saveEntry->dataType))+
            "'!");
            return;
        }

        log("[EditorWidget] - Widget builder found for dataType '"
            +std::string(Rune::Save::DataTypeName(saveEntry->dataType))+
        "'!");
        
        log("[EditorWidget] - Assigning function value...");
        widgetBuilder = it->second;

        log("[EditorWidget] - Function value assigned!");
    }

    this->metaEntry = metaEntry;
    log("[EditorWidget] - Found and set meta entry for '"+saveEntry->dataName+"'!");
    log("[EditorWidget] - Running widget builder for '"+saveEntry->dataName+"'...");
    (*widgetBuilder)(this);

    this->_isLoaded = true;
    log("[WIDGET "+saveEntry->dataName+"] - Completed widget build function. Widget now successfully loaded.");
}

void EditorWidget::clearTrove(){
    if (!this->_isLoaded){
        return;
    }

    if (this->trove == nullptr){
        return;
    }

    log("[WIDGET "+this->saveEntry->dataName+"] - Clearing trove.");
    this->trove->clear();

    // clear keyed elements
    this->keyedElements.clear();
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
    // Clear trove and delete it
    this->clearTrove();
    delete this->trove;
}

void EditorWidget::addToElements(AetherElement element){
    log("[EditorWidget] - Adding element to vector...");
    this->elements.push_back(element);

    log("[EditorWidget] - Adding element to trove...");
    this->trove->addElement(element); // why?
}

void EditorWidget::addDescription(const std::string &description){
    if (!description.empty()){
        Aether::ListComment* entryComment = new Aether::ListComment(description);
        entryComment->setTextColour(MUTED_TEXT_COLOR);
        this->addToElements(entryComment);
    }
}

void EditorWidget::addEditButton(const std::string &name, std::string value){
    Aether::ListOption* editBtn = new Aether::ListOption(name, value, [](){}, MAX_VALUE_LENGTH);
    editBtn->setValueColour(TOP_R);
    editBtn->setLineColour(SEPARATOR_COLOUR);
    
    this->setEditButton(editBtn);
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

void EditorWidget::setWidgetOverlay(WidgetOverlay* widgetOverlay){
    this->widgetOverlay = widgetOverlay;
}

WidgetOverlay* EditorWidget::getWidgetOverlay(){
    return this->widgetOverlay;
}

Rune::Save::Entry* EditorWidget::getSaveEntry(){
    return this->saveEntry;
}

Rune::Save::MetaEntry* EditorWidget::getMetaEntry(){
    return &this->metaEntry;
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

bool EditorWidget::isRenderAsync(){
    return this->renderAsync;
}

EditorMenu* EditorWidget::getMenu(){
    return this->menu;
}

std::vector<std::any> EditorWidget::getMeta(){
    return this->meta;
}