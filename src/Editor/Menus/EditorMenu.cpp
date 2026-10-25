#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>

EditorMenu::EditorMenu(std::string menuName, Aether::Window* window){
    // init
    this->_menuName = menuName;
    this->_window = window;
    this->trove = new Trove();

    // Doesn't matter if no menu builder, just make empty object
}

EditorMenu::~EditorMenu(){
    // Clear all owned widgets
    for (EditorWidget* widget : this->widgets){
        delete widget; // now clears this widgets' trove and deletes it automatically!
    }

    // Clear my own trove
    this->trove->clear();
    delete this->trove;
}

void EditorMenu::build(std::string builderName){
    // If builder name is blank, default to entry
    if (builderName.empty()){
        builderName = "Entry";
    }

    // Find menu builder
    auto it = MENU_BUILDERS.find(builderName);
    if (it != MENU_BUILDERS.end()){
        // Get and run the builder
        MenuBuilderFunc funcPtr = it->second;
        funcPtr(this);
    }
}

void EditorMenu::addWidget(EditorWidget* widget){
    // Store the widget for deletion later
    this->widgets.push_back(widget);
}

void EditorMenu::addElement(AetherElement element){
    this->trove->addElement(element);
}

// Return menuName
std::string EditorMenu::menuName(){
    return this->_menuName;
}

// Set dataNames
void EditorMenu::setDataNames(std::vector<std::string> dataNames){
    this->_dataNames = dataNames;
}

// Return dataNames
std::vector<std::string> EditorMenu::dataNames(){
    return this->_dataNames;
}

// Set list
void EditorMenu::setList(Aether::List* list){
    this->_list = list;
}

// Return list
Aether::List* EditorMenu::list(){
    return this->_list;
}

void EditorMenu::setSaveFile(Rune::Save::File* saveFile){
    this->_saveFile = saveFile;
}

Rune::Save::File* EditorMenu::saveFile(){
    return this->_saveFile;
}

Aether::Window* EditorMenu::window(){
    return this->_window;
}

void EditorMenu::setPopupList(Aether::PopupList* popupList){
    this->_popupList = popupList;
}

Aether::PopupList* EditorMenu::popupList(){
    return this->_popupList;
}

void EditorMenu::setWidgetOverlay(WidgetOverlay* widgetOverlay){
    this->_widgetOverlay = widgetOverlay;
}

WidgetOverlay* EditorMenu::widgetOverlay(){
    return this->_widgetOverlay;
}