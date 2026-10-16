#include <Widgets/WidgetBuilder.hpp>
#include <Widgets/EditorWidget.hpp>
#include <Save/Save.hpp>
#include <Events/Signal.hpp>
#include <Application/Keyboard.hpp>
#include <Config.hpp>

#include <string>

/**
 * @brief Constructor for float32 values.
 * Will apply to f32
 */
void Rune::WidgetBuilder::Float(EditorWidget* widget){
    Rune::Save::Entry* saveEntry  = widget->getSaveEntry();
    
    log("[WIDGET] - Creating float widget.");

    // Create the button
    widget->addEditButton(getReadableSaveKeyName(saveEntry->dataName), Rune::Save::ValueToString(saveEntry->value, false));
    widget->addDescription(getSaveKeyDescription(saveEntry->dataName));

    // Set its onPress
    widget->getEditButton()->onPress([widget]() mutable{
        WidgetOverlay* editOverlay = widget->getWidgetOverlay();
        Rune::Save::Entry* saveEntry = widget->getSaveEntry(); // get fresh reference

        // get min/max/init values
        editOverlay->setTitleLabel("Edit Value - "+getReadableSaveKeyName(saveEntry->dataName));
        
        // build list option
        Aether::ListOption* editBtn = new Aether::ListOption(
            getReadableSaveKeyName(saveEntry->dataName), 
            Rune::Save::ValueToString(saveEntry->value, false),
            [](){}
        );

        editBtn->setLineColour(SEPARATOR_COLOUR);
        editBtn->setValueColour(TOP_R);

        editBtn->onPress(
            [widget, editBtn](){
                float outFloat;
                Result rc = Rune::Keyboard::getNumberInput(
                    &outFloat, "Enter a decimal value...", 
                    std::get<float>(widget->getMetaEntry()->minValue),
                    std::get<float>(widget->getMetaEntry()->maxValue)
                );

                if (R_SUCCEEDED(rc)){
                    editBtn->setValue(std::to_string(outFloat));
                } else {
                    log("[WidgetBuilder::Float] - An error ocurred when taking number value input: "+std::to_string(rc));
                }
            }
        );
        
        // give list and add option
        editOverlay->addList();
        editOverlay->list()->addElement(editBtn);

        // save on close
        editOverlay->onCloseSignal->connectWithId([widget, editBtn](size_t id, bool isSave){
            widget->getWidgetOverlay()->onCloseSignal->disconnect(id);

            if (isSave){
                // update value
                log("[WidgetOverlay::onCloseSignal] - WidgetOverlay closed; updating value to "
                    +editBtn->value());

                float value = std::stof(editBtn->value());

                widget->getEditButton()->setValue(std::to_string(value));
                widget->getSaveEntry()->value = value;
            }
        });

        // show overlay
        widget->getWindow()->addOverlay(editOverlay);
    });
}
