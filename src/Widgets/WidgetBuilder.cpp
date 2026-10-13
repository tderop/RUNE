#include <Widgets/EditorWidget.hpp>
#include <Widgets/WidgetBuilder.hpp>

#include <Save/Save.hpp>

namespace Rune{
    /**
     * @brief Constructor for boolean values.
     * Will apply to bool & revival bool
     */
    void WidgetBuilder::Bool(EditorWidget* widget){
        // build the list option
        Aether::ListOption* boolButton = nullptr;
        Rune::Save::Entry* saveEntry = widget->getSaveEntry();

        log("[WIDGET " + saveEntry->dataName + "] - Creating bool widget...");

        boolButton = new Aether::ListOption(
            getReadableSaveKeyName(saveEntry->dataName),
            Rune::Save::ValueToString(saveEntry->value, false),
            [widget, saveEntry, boolButton]()
            {
                log("[WIDGET " + saveEntry->dataName + "] - Bool widget clicked!");

                // get the popup list
                log("[WIDGET " + saveEntry->dataName + "] - Getting popup list...");
                Aether::PopupList *popupList = widget->getPopupList();
            
                // clear its entries
                // widget->removePopupListEntries();
                log("[WIDGET " + saveEntry->dataName + "] - Removing all popup list entries...");
                popupList->removeEntries();

                // update its title
                log("[WIDGET " + saveEntry->dataName + "] - Getting readable save key name...");
                std::string readableSaveKey = getReadableSaveKeyName(saveEntry->dataName);
                popupList->setTitleLabel("Edit Value - " + readableSaveKey); // kinda looks ugly`

                // add true/false options
                log("[WIDGET " + saveEntry->dataName + "] - Adding popup list entries...");
                bool boolSaveValue = std::get<bool>(saveEntry->value);

                widget->addPopupListEntry(std::string("true"), [widget, saveEntry]() { // set entry to true
                    Aether::ListOption* editBtn = widget->getEditButton();
                    if (editBtn == nullptr){
                        log("[WIDGET " + saveEntry->dataName + "] - No edit button!");
                        return;
                    }

                    saveEntry->value = true;
                    editBtn->setValue("true");
                    
                }, (boolSaveValue == true));

                widget->addPopupListEntry(std::string("false"), [widget, saveEntry]() { // set entry to false
                    Aether::ListOption* editBtn = widget->getEditButton();

                    if (editBtn == nullptr){
                        log("[WIDGET " + saveEntry->dataName + "] - No edit button!");
                        return;
                    }

                    saveEntry->value = false;
                    editBtn->setValue("false");
                }, (boolSaveValue == false));

                // show the popup list
                log("[WIDGET " + saveEntry->dataName + "] - Showing popup list...");
                widget->getWindow()->addOverlay(popupList);

                log("[WIDGET " + saveEntry->dataName + "] - Popup list shown successfully!");
                log("*******************************************************");
            }
        );

        log("[WIDGET " + saveEntry->dataName + "] - Setting edit button...");

        boolButton->setValueColour(TOP_R);
        boolButton->setLineColour(SEPARATOR_COLOUR);
        
        widget->setEditButton(boolButton);

        // list comment description
        widget->addDescription(getSaveKeyDescription(saveEntry->dataName));

        log("[WIDGET " + saveEntry->dataName + "] - Created bool widget successfully!");
        return;
    }

    /**
     * @brief Constructor for signed32 values.
     * Will apply to s32
     */
    void WidgetBuilder::Int(EditorWidget* widget){
        // data
        Aether::ListOption* intButton = nullptr;
        Rune::Save::Entry* saveEntry  = widget->getSaveEntry();
        
        log("[WIDGET] - Creating int widget.");

        // Create the button
        intButton = new Aether::ListOption(
            getReadableSaveKeyName(saveEntry->dataName),
            Rune::Save::ValueToString(saveEntry->value, false),
            [widget](){
                WidgetOverlay* editOverlay = widget->getWidgetOverlay();
                Rune::Save::Entry* saveEntry = widget->getSaveEntry(); // get fresh reference

                // get min/max/init values
                Rune::Save::MetaEntry* metaEntry = widget->getMetaEntry();
                editOverlay->setTitleLabel("Edit Value - "+getReadableSaveKeyName(saveEntry->dataName));
                
                // build popup interaction option
                Aether::Spinner* numSpinner = new Aether::Spinner(0, 0, 180);
                numSpinner->setXY(
                    editOverlay->panel()->x() + editOverlay->panel()->w()/2 - numSpinner->w()/2,
                    editOverlay->panel()->y() + editOverlay->panel()->h()/2 - numSpinner->h()/2
                );

                numSpinner->setMax(std::get<int>(metaEntry->maxValue));
                numSpinner->setMin(std::get<int>(metaEntry->minValue));
                numSpinner->setValue(std::get<int>(saveEntry->value));
                // numSpinner->setW(240);

                editOverlay->addTempElement(numSpinner);
                widget->getWindow()->addOverlay(editOverlay);
            }
        );

        intButton->setValueColour(TOP_R);
        intButton->setLineColour(SEPARATOR_COLOUR);

        widget->setEditButton(intButton);
        widget->addDescription(getSaveKeyDescription(saveEntry->dataName));

        return;
    }
} // namespace Rune