#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>
#include <Utils/Logger.hpp>

#include <functional>

EditorMenu::EditorMenu(std::string menuName, Aether::Window* window, Aether::Screen* screen){
    // init
    this->_menuName = menuName;
    this->_window = window;
    this->_screen = screen;

    this->_trove = new Trove();

    // default reload func - just recreate
    this->onReload([this](){
        Logger::Write("=============== [RELOADING: "+_menuName+"...] ===============", true);

        this->onDelete();
        this->_trove = new Trove();

        // if list is not null, reset it to generate new body
        if (this->_list != nullptr){
            this->setList(this->_list);
        }

        // rebuild
        if (!this->builder.empty()){
            this->build(this->builder);
        }
    });

    // Doesn't matter if no menu builder, just make empty object
}

void EditorMenu::onDelete(){
    this->clearAllWidgets();

    // Clear my own trove
    this->_trove->clear();
    delete this->_trove;

    // reset garbage pointers
    this->_trove = nullptr;
    this->_body = nullptr;
}

EditorMenu::~EditorMenu(){
    this->onDelete();
}

void EditorMenu::build(std::string builderName){
    // If builder name is blank, default to entry
    if (builderName.empty()){
        builderName = "Entry";
    }

    this->builder = builderName;

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
    this->_trove->addElement(element);
}

void EditorMenu::giveTask(std::function<void()> task){
    this->_trove->giveTask(task);
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
    if (this->_body == nullptr){
        this->_body = new Aether::Container(
            list->x(),
            list->y(),
            list->w(),
            list->h()
        );

        this->_screen->addElement(_body);
        this->addElement(_body);
    }

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

Aether::Container* EditorMenu::body(){
    return this->_body;
}

void EditorMenu::clearAllWidgets(){
    // Clear all owned widgets
    for (EditorWidget* widget : this->widgets){
        delete widget; // now clears this widgets' trove and deletes it automatically!
    }

    this->widgets.clear();
}

void EditorMenu::onReload(std::function<void()> f){
    this->_onReload = f;
}

void EditorMenu::requestReload(){
    this->_reloadQueued = true;
}

void EditorMenu::reload(){
    this->_reloadQueued = false;
    if (this->_onReload){
        this->_onReload();
    }
}

bool EditorMenu::reloadQueued(){
    return this->_reloadQueued;
}
// Trove* EditorMenu::trove(){
//     return this->_trove;
// }