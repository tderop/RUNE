#include <Screens/Editor.hpp>

void EditorScreen::makeWidget(const std::string &saveKey){
    EditorWidget* widget = this->getWidgetFromSaveKey(saveKey);
    if (widget == nullptr){
        return;
    }

    this->tempWidgets.push_back(widget);

    if (!widget->isLoaded()){
        return; // ignore if failed to load
    }
    
    widget->addAllElements(this->editorScroll);
}

void EditorScreen::cleanEditorWidgets(){
    for (EditorWidget* widget : this->tempWidgets){
        log("[EDITOR] - Deleting widget...");
        widget->clearTrove();
        delete widget;
    }

    this->tempWidgets.clear();
    this->editorScroll->removeAllElements();
}

/**
 * @brief Create an EditorWidget.
 */
EditorWidget* EditorScreen::getWidgetFromSaveKey(const std::string& saveKey){
    // get the save entry
    log("[EDITOR] - Getting entry from data name '"+saveKey+"'...");
    if (this->saveFile == nullptr){
        log("[EDITOR] - Save file is nullptr! Ignoring request.");
        return nullptr;
    }

    // handle padding
    if (saveKey == "<PAD>"){
        Aether::ListSeparator* pad = new Aether::ListSeparator();
        this->editorScroll->addElement(pad);
        return nullptr;
    }

    Rune::Save::Entry* saveEntry = Rune::Save::GetEntryFromDataName((*this->saveFile), saveKey);
    if (saveEntry == nullptr){
        log("[EDITOR] - Save entry '"+saveKey+"' was not found. Ignoring request.");
        return nullptr;
    }

    // create the editor widget
    log("[EDITOR] - Constructing new editor widget...");
    // create widget overlay
   
    EditorWidget* widget = new EditorWidget(saveEntry, app->window());

    widget->setPopupList(this->popupList);
    widget->setWidgetOverlay(this->widgetOverlay);

    log("[EDITOR] - Widget for '"+saveKey+"' successfully built. isLoaded? "+std::to_string(widget->isLoaded()));
    return widget;
}