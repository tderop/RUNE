#include <Editor/WidgetBuilder.hpp>
#include <Editor/EditorWidget.hpp>
#include <Editor/WidgetOverlay.hpp>

#include <Application/Keyboard.hpp>

#include <Save/Save.hpp>
#include <Utils/Logger.hpp>
#include <Config.hpp>

#include <string>

// I'll just ignore string size here and deal with it during compilation
// // Maybe it would be nice QoL to show the max length of the string though
void Rune::WidgetBuilder::String(EditorWidget* widget){
    Rune::Save::Entry* saveEntry = widget->getSaveEntry();
    widget->addEditButton(
        Rune::Save::GetReadableKeyName(saveEntry->dataName),
        Rune::Save::ValueToString(saveEntry->value, false)
    );
    widget->addDescription(
        Rune::Save::GetKeyDescription(saveEntry->dataName)
    );

    widget->getEditButton()->onPress([widget]() mutable{
        WidgetOverlay* ovl           = widget->getWidgetOverlay();
        Rune::Save::Entry* saveEntry = widget->getSaveEntry();

        std::string readableName  = Rune::Save::GetReadableKeyName(saveEntry->dataName);
        std::string readableValue = Rune::Save::ValueToString(saveEntry->value);
        ovl->setTitleLabel("Edit Value - " + readableName);
        ovl->setDeveloperNote(Rune::Save::GetKeyDevNote(saveEntry->dataName));
        
        Aether::ListOption* editBtn = new Aether::ListOption(readableName, readableValue, [](){});

        editBtn->setLineColour(SEPARATOR_COLOUR); editBtn->setValueColour(TOP_R);
        editBtn->onPress([widget, editBtn](){
            std::string outStr;
            Result rc = Rune::Keyboard::getStringInput(&outStr, "Enter text...");
            if (R_SUCCEEDED(rc)){
                editBtn->setValue(outStr);
            }
        });

        ovl->addList();
        ovl->list()->addElement(editBtn);

        // focus the first button
        ovl->panel()->setFocused(ovl->list());
        ovl->list()->setFocused(editBtn);

        ovl->onCloseSignal->connectWithId([widget, editBtn](size_t id, bool isSave){
            widget->getWidgetOverlay()->onCloseSignal->disconnect(id);

            if (isSave){
                std::string value = editBtn->value();
                Logger::Write("[Rune::WidgetBuilder::String] - Inner edit button value: '"+value+"'");
                
                widget->getEditButton()->setValue(value);
                widget->getSaveEntry()->value = value;
            }
        });
        widget->getWindow()->addOverlay(ovl);
    });
}