#include <Widgets/WidgetBuilder.hpp>
#include <Widgets/EditorWidget.hpp>
#include <Save/Save.hpp>
#include <Events/Signal.hpp>
#include <Application/Keyboard.hpp>

/**
 * @brief Constructor for signed32 values.
 * Will apply to s32
 */
void Rune::WidgetBuilder::Int(EditorWidget* widget){
    Rune::Save::Entry* saveEntry  = widget->getSaveEntry();
    
    log("[WIDGET] - Creating int widget.");

    // Create the button
    widget->addEditButton(getReadableSaveKeyName(saveEntry->dataName), Rune::Save::ValueToString(saveEntry->value, false));
    widget->addDescription(getSaveKeyDescription(saveEntry->dataName));

    // Set its onPress
    widget->getEditButton()->onPress([widget]() mutable{
        WidgetOverlay* editOverlay = widget->getWidgetOverlay();
        Rune::Save::Entry* saveEntry = widget->getSaveEntry(); // get fresh reference

        // get min/max/init values
        Rune::Save::MetaEntry* metaEntry = widget->getMetaEntry();
        editOverlay->setTitleLabel("Edit Value - "+getReadableSaveKeyName(saveEntry->dataName));
        
        // build popup interaction option
        Aether::Spinner* numSpinner = new Aether::Spinner(0, 0, 180);
        numSpinner->setXY(
            editOverlay->panel()->x() + editOverlay->panel()->w()/2 - numSpinner->w()/2,
            editOverlay->panel()->y() + editOverlay->panel()->h()/2 - numSpinner->h()/2 - 12
        );

        numSpinner->setMax(std::get<int>(metaEntry->maxValue));
        numSpinner->setMin(std::get<int>(metaEntry->minValue));
        numSpinner->setValue(std::get<int>(saveEntry->value));

        numSpinner->setLabel(getReadableSaveKeyName(saveEntry->dataName));
        numSpinner->onPress([widget, editOverlay, numSpinner, metaEntry](){
            // get num input
            float outFloat;
            Result rc = Rune::Keyboard::getNumberInput(&outFloat, "Enter a number...", 
                std::get<int>(metaEntry->minValue), std::get<int>(metaEntry->maxValue));

            if (R_SUCCEEDED(rc)){
                // update value
                numSpinner->setValue((int)outFloat);
                // editOverlay->closeWidgetOverlay(true);
            }
        });

        editOverlay->addTempElement(numSpinner);

        editOverlay->onCloseSignal->connectWithId([numSpinner, widget](size_t id, bool isSave){
            widget->getWidgetOverlay()->onCloseSignal->disconnect(id);

            if (isSave){
                // update value
                log("[WidgetOverlay::onCloseSignal] - WidgetOverlay closed; updating value to "
                    +std::to_string(numSpinner->value()));

                int value = numSpinner->value();
                widget->getEditButton()->setValue(std::to_string(value));
                widget->getSaveEntry()->value = value;
            }
        });

        widget->getWindow()->addOverlay(editOverlay);
    });
}
