#include <Screens/Editor.hpp>

void EditorScreen::cleanMenu(){
    // clear the current menu
    if (this->currentMenu != nullptr){
        delete this->currentMenu;
        this->currentMenu = nullptr;
    }

    // clear all remaining scroll elements
    this->editorScroll->removeAllElements();
}

// lol this file is so small since I moved to the new menu framework
// let's hope it works first try with no crashes at all!