#include <vector>
#include <string>

#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>
#include <Utils/Logger.hpp>

const std::string PADDING_KEY = "<PAD>";

EditorWidget* Rune::MenuBuilder::AddWidgetFromSaveKey(EditorMenu* menu, const std::string& saveKey, bool renderAsync, std::vector<std::any> meta, Aether::Element* parentOverride){
    // Get the save entry
    Rune::Save::Entry* saveEntry = Rune::Save
    ::GetEntryFromDataName(*(menu->saveFile()), saveKey);

    if (saveEntry == nullptr){
        // Failed to get save entry!
        return nullptr;
    }

    // Create the editor widget
    EditorWidget* widget = new EditorWidget(saveEntry, menu, renderAsync, meta);

    widget->setPopupList(menu->popupList());
    widget->setWidgetOverlay(menu->widgetOverlay());

    // This section replaces the old EditorScreen::makeWidget()
    
    // Take ownership of the widget
    menu->addWidget(widget);

    // If the widget is not loaded, ignore
    if (!widget->isLoaded()){
        return nullptr;
    }

    // Add all the elements from the widget to the list
    if (parentOverride==nullptr){
        widget->addAllElements(menu->list()); 
    } else {
        widget->addAllElements(parentOverride);
    }

    return widget;
}

void Rune::MenuBuilder::Entry(EditorMenu* menu){
    // Get the entry names used by this menu
    std::vector<std::string> menuEntries = menu->dataNames();

    // No entires, return
    if (menuEntries.empty()){
        Logger::Write("[Rune::MenuBuilder::Entry] - No menu entries provided for menu "+menu->menuName()+"!");
        return;
    }

    // No save file, return
    if (menu->saveFile() == nullptr){
        Logger::Write("[Rune::MenuBuilder::Entry] - No save file provided!");
        return;
    }

    // No list, return
    if (menu->list() == nullptr){
        Logger::Write("[Rune::MenuBuilder::Entry] - No Editor Aether::List provided!");
        return;
    }

    // No popupList, return (probably getting removed later)
    if (menu->popupList() == nullptr){
        Logger::Write("[Rune::MenuBuilder::Entry] - No Editor Aether::PopupList provided!");
        return;
    }   

    // No widgetOverlay, return
    if (menu->widgetOverlay() == nullptr){
        Logger::Write("[Rune::MenuBuilder::Entry] - No Editor WidgetOverlay provided!");
        return;
    }  

    // A non-copying loop through all of the entries in this menu
    for (const std::string& saveKey : menuEntries){
        // Replacing the old EditorScreen::getWidgetFromSaveKey()
        
        // Handle special cases
        if (saveKey == PADDING_KEY){
            Aether::ListSeparator* pad = new Aether::ListSeparator();
            
            // Hand the element to the menu
            menu->addToTrove(pad);

            // Show the element in the list
            menu->list()->addElement(pad);

            continue; // Proceed to next key
        }

        Rune::MenuBuilder::AddWidgetFromSaveKey(menu, saveKey);
    }
}