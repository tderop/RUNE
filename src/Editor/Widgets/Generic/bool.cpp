#include <Editor/WidgetBuilder.hpp>
#include <Editor/EditorWidget.hpp>
#include <Save/Save.hpp>
#include <Events/Signal.hpp>
#include <Application/Audio.hpp>
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
        Rune::Audio::playSound("select");
        Logger::Write("[WIDGET " + saveEntry->dataName + "] - Bool widget clicked!");

        // Check current value
        Logger::Write("[WIDGET " + saveEntry->dataName + "] - Adding popup list entries...");
        bool boolSaveValue = std::get<bool>(saveEntry->value);

        // Invert and update
        saveEntry->value=!boolSaveValue;
        widget->getEditButton()->setValue(Rune::Save::ValueToString(saveEntry->value));
    });

    Logger::Write("[WIDGET " + saveEntry->dataName + "] - Created bool widget successfully!");
}