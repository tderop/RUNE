#include <Editor/WidgetBuilder.hpp>
#include <Editor/EditorWidget.hpp>
#include <Editor/WidgetOverlay.hpp>

#include <Application/Keyboard.hpp>

#include <Save/Save.hpp>
#include <Config.hpp>

#include <Utils/Utils.hpp>
#include <Utils/Logger.hpp>

#include <string>

std::vector<std::string> VECTOR_LETTERS = {"X", "Y", "Z", "A"};

void Rune::WidgetBuilder::Vector(EditorWidget* widget){
    Rune::Save::Entry* saveEntry = widget->getSaveEntry();
    widget->addEditButton(
        Rune::Save::GetReadableKeyName(saveEntry->dataName),
        Rune::Save::ValueToString(saveEntry->value)
    );
    widget->addDescription(
        Rune::Save::GetKeyDescription(saveEntry->dataName)
    );
    widget->getEditButton()->onPress([widget](){
        WidgetOverlay* ovl           = widget->getWidgetOverlay();
        Rune::Save::Entry* saveEntry = widget->getSaveEntry();

        std::string readableName  = Rune::Save::GetReadableKeyName(saveEntry->dataName);
        std::string readableValue = Rune::Save::ValueToString(saveEntry->value);
        ovl->setTitleLabel("Edit Value - "+readableName);
        ovl->setDeveloperNote(Rune::Save::GetKeyDevNote(saveEntry->dataName));

        ovl->addList();

        // build the number buttons
        auto value = std::get<std::vector<float>>(saveEntry->value);
        size_t vectorSize = value.size();

        std::vector<Aether::ListOption*> buttons;

        for (size_t i = 0; i < vectorSize; i++){
            Aether::ListOption* editBtn = new Aether::ListOption(
                VECTOR_LETTERS.at(i),
                Rune::Save::ValueToString(value.at(i)),
                [](){}
            );

            editBtn->onPress(
                [widget, editBtn, i](){
                    float outFloat;

                    Logger::Write("[Rune::WidgetBuilder::Vector] - Getting metadata...");
                    Rune::Save::MetaEntry* metaEntry = widget->getMetaEntry();

                    Logger::Write("[Rune::WidgetBuilder::Vector] - Got metadata, getting bounds...");
                    Logger::Write("[Rune::WidgetBuilder::Vector] - minValue: '"
                        +Rune::Save::ValueToString(metaEntry->minValue)+"' | maxValue: '"
                        +Rune::Save::ValueToString(metaEntry->maxValue)+"' | initValue: '"
                        +Rune::Save::ValueToString(metaEntry->initValue)+"'");

                    const auto* minValues = std::get_if<std::vector<float>>(&metaEntry->minValue);
                    const auto* maxValues = std::get_if<std::vector<float>>(&metaEntry->maxValue);

                    if (!minValues || !maxValues ||
                        i >= minValues->size() || i >= maxValues->size()){

                        Logger::Write("[Rune::WidgetBuilder::Vector] - Invalid component bounds in metadata.");
                        return;
                    }

                    float minValue = minValues->at(i);
                    float maxValue = maxValues->at(i);

                    Logger::Write("[Rune::WidgetBuilder::Vector] - Got bounds, showing keyboard...");
                    Result rc = Rune::Keyboard::getNumberInput(&outFloat, "Enter a number...", minValue, maxValue, true);

                    if (R_SUCCEEDED(rc)){
                        Logger::Write("[Rune::WidgetBuilder::Vector] - Success, setting value");
                        editBtn->setValue(std::to_string(outFloat));
                    }
                }
            );

            editBtn->setLineColour(SEPARATOR_COLOUR);
            editBtn->setValueColour(TOP_R);

            editBtn->setValue(std::to_string(value.at(i)));
            buttons.push_back(editBtn);

            ovl->list()->addElement(editBtn);
        }

        ovl->onCloseSignal->connectWithId([widget, buttons](size_t id, bool isSave){
            widget->getWidgetOverlay()->onCloseSignal->disconnect(id);

            if (isSave){
                Rune::Save::Entry* saveEntry = widget->getSaveEntry();
                auto oldValue = std::get<std::vector<float>>(saveEntry->value);

                // get the vector size dynamically
                int vectorSize = oldValue.size();
                std::vector<float> newValue;

                // get all buttons and build new value
                for (Aether::ListOption* editBtn : buttons){
                    newValue.push_back(std::stof(editBtn->value()));
                }

                Logger::Write("[Rune::WidgetBuilder::String] - Inner edit button value: '"+Rune::Save::ValueToString(newValue)+"'");

                widget->getEditButton()->setValue(Rune::Save::ValueToString(newValue));
                widget->getSaveEntry()->value = newValue;
            }
        });
        widget->getWindow()->addOverlay(ovl);

        // focus the first button
        ovl->setFocused(ovl->list());
        ovl->list()->setFocused(buttons.at(0));
    });
}