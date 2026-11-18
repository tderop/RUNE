#include <Editor/WidgetBuilder.hpp>
#include <Editor/EditorWidget.hpp>
#include <Save/Save.hpp>
#include <Events/Signal.hpp>
#include <Application/Keyboard.hpp>
#include <Application/Audio.hpp>
#include <Utils/Logger.hpp>

/**
 * @brief Constructor for signed32 values.
 * Will apply to s32
 */
void Rune::WidgetBuilder::Int(EditorWidget* widget){
    Rune::Save::Entry* saveEntry  = widget->getSaveEntry();
    
    Logger::Write("[WIDGET] - Creating int widget.");

    // Create the button
    widget->addEditButton(Rune::Save::GetReadableKeyName(saveEntry->dataName), Rune::Save::ValueToString(saveEntry->value, false));
    widget->addDescription(Rune::Save::GetKeyDescription(saveEntry->dataName));

    // Set its onPress
    widget->getEditButton()->onPress([widget]() mutable{
        WidgetOverlay* editOverlay = widget->getWidgetOverlay();
        Rune::Save::Entry* saveEntry = widget->getSaveEntry(); // get fresh reference

        // get min/max/init values
        Rune::Save::MetaEntry* metaEntry = widget->getMetaEntry();
        editOverlay->setTitleLabel("Edit Value - "+Rune::Save::GetReadableKeyName(saveEntry->dataName));
        editOverlay->setDeveloperNote(Rune::Save::GetKeyDevNote(saveEntry->dataName));
        
        // build popup interaction option
        Aether::Spinner* numSpinner = new Aether::Spinner(0, 0, 264);
        numSpinner->setXY(
            editOverlay->panel()->x() + editOverlay->panel()->w()/2 - numSpinner->w()/2,
            editOverlay->panel()->y() + editOverlay->panel()->h()/2 - numSpinner->h()/2 - 12
        );

        int max = std::get<int>(metaEntry->maxValue);
        int min = std::get<int>(metaEntry->minValue);
        
        numSpinner->setMax(max);
        numSpinner->setMin(min);
        numSpinner->setValue(std::get<int>(saveEntry->value));
        numSpinner->setWrapAround(true);

        numSpinner->setLabel(Rune::Save::GetReadableKeyName(saveEntry->dataName));
        numSpinner->onPress([numSpinner, min, max](){
            // get num input
            float outFloat;

            Result rc = Rune::Keyboard::getNumberInput(&outFloat,
                "Enter a number... ("+Rune::Save::ValueToString(min)+" → "+Rune::Save::ValueToString(max)+")", 
                min, max);

            if (R_SUCCEEDED(rc)){
                // update value
                numSpinner->setValue((int)outFloat);
                // editOverlay->closeWidgetOverlay(true);
            }
        });
        numSpinner->onValueChange([](){
            Rune::Audio::playSound("cursor");
        });

        editOverlay->addTempElement(numSpinner);

        editOverlay->onCloseSignal->connectWithId([numSpinner, widget](size_t id, bool isSave){
            widget->getWidgetOverlay()->onCloseSignal->disconnect(id);

            if (isSave){
                // update value
                Logger::Write("[WidgetOverlay::onCloseSignal] - WidgetOverlay closed; updating value to "
                    +std::to_string(numSpinner->value()));

                int value = numSpinner->value();
                widget->getEditButton()->setValue(Rune::Save::ValueToString(value));
                widget->getSaveEntry()->value = value;
            }
        });

        widget->getWindow()->addOverlay(editOverlay);
    });
}
