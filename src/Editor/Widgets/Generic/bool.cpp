#include <Editor/WidgetBuilder.hpp>
#include <Editor/EditorWidget.hpp>
#include <Save/Save.hpp>
#include <Events/Signal.hpp>
#include <Utils/Logger.hpp>

/**
 * @brief Constructor for boolean values.
 * Will apply to bool & revival bool
 */
void Rune::WidgetBuilder::Bool(EditorWidget* widget){
    Rune::Save::Entry* saveEntry = widget->getSaveEntry();

    // Make btn
    widget->addEditButton(Rune::Save::GetReadableKeyName(saveEntry->dataName), Rune::Save::ValueToString(saveEntry->value, false));
    widget->addDescription(Rune::Save::GetKeyDescription(saveEntry->dataName));

    // set click callback
    widget->getEditButton()->onPress([widget, saveEntry](){
        Logger::Write("[WIDGET " + saveEntry->dataName + "] - Bool widget clicked!");

        // get the popup list
        Logger::Write("[WIDGET " + saveEntry->dataName + "] - Getting popup list...");
        Aether::PopupList *popupList = widget->getPopupList();
    
        // clear its entries
        // widget->removePopupListEntries();
        Logger::Write("[WIDGET " + saveEntry->dataName + "] - Removing all popup list entries...");
        popupList->removeEntries();

        // update its title
        Logger::Write("[WIDGET " + saveEntry->dataName + "] - Getting readable save key name...");
        std::string readableSaveKey = Rune::Save::GetReadableKeyName(saveEntry->dataName);
        popupList->setTitleLabel("Edit Value - " + readableSaveKey); // kinda looks ugly`

        // add true/false options
        Logger::Write("[WIDGET " + saveEntry->dataName + "] - Adding popup list entries...");
        bool boolSaveValue = std::get<bool>(saveEntry->value);

        widget->addPopupListEntry(std::string("true"), [widget, saveEntry]() { // set entry to true
            Aether::ListOption* editBtn = widget->getEditButton();
            if (editBtn == nullptr){
                Logger::Write("[WIDGET " + saveEntry->dataName + "] - No edit button!");
                return;
            }

            saveEntry->value = true;
            editBtn->setValue("true");
            
        }, (boolSaveValue == true));

        widget->addPopupListEntry(std::string("false"), [widget, saveEntry]() { // set entry to false
            Aether::ListOption* editBtn = widget->getEditButton();

            if (editBtn == nullptr){
                Logger::Write("[WIDGET " + saveEntry->dataName + "] - No edit button!");
                return;
            }

            saveEntry->value = false;
            editBtn->setValue("false");
        }, (boolSaveValue == false));

        // show the popup list
        Logger::Write("[WIDGET " + saveEntry->dataName + "] - Showing popup list...");
        widget->getWindow()->addOverlay(popupList);

        Logger::Write("[WIDGET " + saveEntry->dataName + "] - Popup list shown successfully!");
        Logger::Write("*******************************************************");
    });

    Logger::Write("[WIDGET " + saveEntry->dataName + "] - Created bool widget successfully!");
}