#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>

#include <Save/Save.hpp>
#include <string>

const int ENTRIES_PER_PAGE = 50;

void Rune::MenuBuilder::AllEntries(EditorMenu* menu){
    // Iterate through all entries and add a widget for every one
    // This gonna cause INSANE lag

    size_t totalEntries = 0;
    for (const auto& entry : menu->saveFile()->entries){
        if (entry.dataName.find("Porch") != std::string::npos){
            log(
                "["+entry.dataName+"] - "+Rune::Save::ValueToString(entry.value), 
                false
            );
        }

        if (entry.dataType == Rune::Save::RuneRevivalBool 
            || entry.dataType == Rune::Save::RuneRevivalS32){

            continue;
        }

        if (entry.dataType != Rune::Save::RuneString64 && 
            entry.dataType != Rune::Save::RuneString32 &&
            entry.dataType != Rune::Save::RuneString256){
            continue;
        }

        totalEntries++;
        Rune::MenuBuilder::AddWidgetFromSaveKey(menu, entry.dataName, true);

        if (totalEntries >= ENTRIES_PER_PAGE){
            break;
        }
    }

    // Wow new framework OP only 3 lines of code
}