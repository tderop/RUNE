#include <Widgets/WidgetBuilder.hpp>
#include <Widgets/EditorWidget.hpp>
#include <Widgets/WidgetOverlay.hpp>

#include <Application/Keyboard.hpp>

#include <Save/Save.hpp>
#include <Config.hpp>

#include <Utils/Utils.hpp>

#include <string>

const std::string KEY_PREFIX = "Button_";
std::vector<std::string> VECTOR_LETTERS = {"X", "Y", "Z", "A"};

void Rune::WidgetBuilder::Vector(EditorWidget* widget){
    Rune::Save::Entry* saveEntry = widget->getSaveEntry();
    widget->addEditButton(
        getReadableSaveKeyName(saveEntry->dataName),
        Rune::Save::ValueToString(saveEntry->value)
    );
    widget->addDescription(
        getSaveKeyDescription(saveEntry->dataName)
    );
    widget->getEditButton()->onPress([widget](){
        WidgetOverlay* ovl           = widget->getWidgetOverlay();
        Rune::Save::Entry* saveEntry = widget->getSaveEntry();

        std::string readableName  = getReadableSaveKeyName(saveEntry->dataName);
        std::string readableValue = Rune::Save::ValueToString(saveEntry->value);
        ovl->setTitleLabel(readableName);

        ovl->addList();

        // build the number buttons
        auto value = std::get<std::vector<float>>(saveEntry->value);
        size_t vectorSize = value.size();

        std::vector<Aether::ListOption*> buttons;

        for (size_t i = 0; i < vectorSize; i++){
            Aether::ListOption* editBtn = new Aether::ListOption(
                VECTOR_LETTERS.at(i),
                Rune::Save::ValueToString(value.at(i)),
                [widget, i](){
                    float outFloat;

                    log("[Rune::WidgetBuilder::Vector] - Getting metadata...");
                    Rune::Save::MetaEntry* metaEntry = widget->getMetaEntry();

                    log("[Rune::WidgetBuilder::Vector] - Got metadata, getting bounds...");
                    log("[Rune::WidgetBuilder::Vector] - minValue: '"
                        +Rune::Save::ValueToString(metaEntry->minValue)+"' | maxValue: '"
                        +Rune::Save::ValueToString(metaEntry->maxValue)+"' | initValue: '"
                        +Rune::Save::ValueToString(metaEntry->initValue)+"'");

                    const auto* minValues = std::get_if<std::vector<float>>(&metaEntry->minValue);
                    const auto* maxValues = std::get_if<std::vector<float>>(&metaEntry->maxValue);

                    if (!minValues || !maxValues ||
                        i >= minValues->size() || i >= maxValues->size()){

                        log("[Rune::WidgetBuilder::Vector] - Invalid component bounds in metadata.");
                        return;
                    }

                    float minValue = minValues->at(i);
                    float maxValue = maxValues->at(i);

                    log("[Rune::WidgetBuilder::Vector] - Got bounds, showing keyboard...");
                    Result rc = Rune::Keyboard::getNumberInput(&outFloat, "Enter a number...", minValue, maxValue);

                    if (R_SUCCEEDED(rc)){
                        log("[Rune::WidgetBuilder::Vector] - Success, setting value");

                        AetherElement buttonElm;
                        rc = widget->getElementAtKey(buttonElm, KEY_PREFIX+std::to_string(i));
                        Aether::ListOption* button = std::get<Aether::ListOption*>(buttonElm);
                        button->setValue(std::to_string(outFloat));
                    }
                }
            );

            editBtn->setLineColour(SEPARATOR_COLOUR);
            editBtn->setValueColour(TOP_R);

            editBtn->setValue(std::to_string(value.at(i)));
            buttons.push_back(editBtn);

            widget->addElementWithKey(editBtn, KEY_PREFIX+std::to_string(i));
            ovl->list()->addElement(editBtn);
        }

        ovl->onCloseSignal->connectWithId([widget](size_t id, bool isSave){
            widget->getWidgetOverlay()->onCloseSignal->disconnect(id);

            if (isSave){
                Rune::Save::Entry* saveEntry = widget->getSaveEntry();
                auto oldValue = std::get<std::vector<float>>(saveEntry->value);

                // get the vector size dynamically
                int vectorSize = oldValue.size();
                std::vector<float> newValue;

                // get all buttons and build new value
                for (int i = 0; i < vectorSize; i++){
                    AetherElement buttonElm;
                    Result rc = widget->getElementAtKey(buttonElm, KEY_PREFIX+std::to_string(i));

                    Aether::ListOption* editBtn = std::get<Aether::ListOption*>(buttonElm);
                    newValue.push_back(std::stof(editBtn->value()));
                }

                log("[Rune::WidgetBuilder::String] - Inner edit button value: '"+Rune::Save::ValueToString(newValue)+"'");

                widget->getEditButton()->setValue(Rune::Save::ValueToString(newValue));
                widget->getSaveEntry()->value = newValue;
            }
        });
        widget->getWindow()->addOverlay(ovl);
    });
}