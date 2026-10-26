#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>

#include <Utils/Utils.hpp>
#include <Save/Save.hpp>

#include <string>

// CONFIG

// All of the items in Link's pouch are merged into one massive array of max length 420. This is going to be a little annoying.
const std::string INVENTORY_KEY = "PorchItem";

void Rune::MenuBuilder::Inventory(EditorMenu* menu){
    return;
}