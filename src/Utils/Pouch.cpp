#include <Editor/Menus/Inventory/Pouch.hpp>

namespace Rune {
    namespace Inventory {
        std::string Pouch::makeCacheKey(Category category, const std::string& id){
            return std::to_string(static_cast<int32_t>(category))+id;
        }

        std::vector<int32_t> Pouch::getOrAdd(Category category, const std::string& id, const std::string& vkey){
            const std::string& cacheKey = this->makeCacheKey(category, id);
            auto cached = this->entryCache_.get<std::vector<int32_t>>(cacheKey);

            if (!cached.has_value()){
                auto entryVec = std::get<std::vector<int32_t>>
                (Rune::Save::GetEntryFromDataName(*this->saveFile_, 
                    vkey)->value);
                
                this->entryCache_.put(cacheKey, entryVec);
                cached = entryVec;
            }

            return cached.value();
        }

        Pouch::Pouch(Save::File* saveFile) : saveFile_(saveFile) {
            // Get all relevant save entries
            auto porchItem = std::get<std::vector<std::string>>(
                Save::GetEntryFromDataName(*saveFile, kInventoryKey)->value);
            auto primaryValue = std::get<std::vector<int32_t>>(
                Save::GetEntryFromDataName(*saveFile, kInventoryValueKey)->value);
            
            // Iterate through and store items
            Category prevCategory = Category::Unknown;
            std::unordered_map<Category, size_t> indexMap;

            for (size_t index = 0; index < porchItem.size(); ++index){
                // The item that will be filled
                auto item = std::make_shared<Item>();

                item->actorName = porchItem.at(index);
                item->name = GetNameFromActorName(item->actorName);
                item->value = primaryValue.at(index); // primary value and main inventory are index-linked
                item->category = (item->actorName.empty()) // an empty actor name indicates an empty slot in this model
                    ? Category::Unknown : GetCategoryFromActorName(item->actorName);

                // The item's position in its category
                item->localIndex = (item->category != Category::Unknown) ? indexMap[item->category]++ : 0; // Default to 0 index if unknown category

                // Set the previous item category for checks
                prevCategory = item->category;
                
                // Equipment specific values
                if (IsCategoryEquipment(item->category)){
                    // flagSp/valueSp
                    item->flagSpKey = GetFlagSpKeyFromCategory(item->category);
                    item->valueSpKey = GetValueSpKeyFromCategory(item->category);
                    std::vector<int32_t> flagSp  = this->getOrAdd(item->category, "flagSp", item->flagSpKey);
                    std::vector<int32_t> valueSp = this->getOrAdd(item->category, "valueSp", item->valueSpKey);
                    item->flagSp  = flagSp.at(item->localIndex);
                    item->valueSp = valueSp.at(item->localIndex);

                    // modifier
                    item->modifier = static_cast<Modifier>(item->flagSp);
                }

                // Set tile properties
                auto it = kLabelTileStyles.find(item->category);
                item->style = (it == kLabelTileStyles.end()) ? TileStyle::NoLabel : it->second;
                item->type  = (item->actorName.empty()) ? TileType::Empty : TileType::Standard;

                // Store the inventory item
                this->items_.push_back(item);
            }
        }

        bool Pouch::hasItem(std::string_view actorName){
            auto it = std::find_if(this->items_.begin(), this->items_.end(), 
                [&](std::shared_ptr<Item> i){
                    return (i->name == actorName);
                });
            
            return (it != this->items_.end());
        }

        PouchItems Pouch::getItems(){
            return this->items_;
        }

        PouchItems Pouch::getItemsByCategory(Category category, size_t slots){
            PouchItems filteredPouch;

            size_t added = 0;
            size_t cursor = 0;

            // Get all the occupied slots
            for (const auto& item : this->items_){
                if (item->category == category){
                    filteredPouch.push_back(item);
                    added++;
                }

                cursor++;
                if (added >= slots){ break; }
            }

            // Returns nullptr if none found
            std::function<std::shared_ptr<Item>()> findNextEmpty = [&]()->std::shared_ptr<Item>{
                std::shared_ptr<Item> nextEmpty = nullptr;

                // Look for the next empty item
                auto it = std::find_if(this->items_.begin(), this->items_.end(), 
                    [](std::shared_ptr<Item> i){
                        return (i->name.empty() && i->category == Category::Unknown);
                    });

                // If it was found, set it
                if (it != this->items_.end()){
                    nextEmpty = *it;
                }

                return nextEmpty;
            };

            // Fill the empty slots
            int slotsLeft = slots - added;
            if (slotsLeft < 0){ slotsLeft = 0; }

            for (size_t i = 0; i < static_cast<size_t>(slotsLeft); ++i){
                auto next = findNextEmpty();
                if (next == nullptr){ 
                    Logger::Importantf("Could not find any more safe slots! Added %d slots.", added);
                    break; 
                } // No next safe found, stop

                // Overwrite the category
                next->category = category;

                // Overwrite the style properties
                auto it = kLabelTileStyles.find(next->category);
                next->style = (it == kLabelTileStyles.end()) ? TileStyle::NoLabel : it->second;
                next->type  = TileType::Empty;

                filteredPouch.push_back(next);
                added++;
            }

            // Check the max slots for this inventory
            size_t max = GetMaxSlotsFromCategory(category);

            // If we've made less slots than the maximum, and this category is upgradeable 
            // add an add slot to increase inventory size
            if (IsCategoryEquipment(category) && added < max){
                auto addSlot = std::make_shared<Item>();
                addSlot->type = TileType::Add;
                addSlot->category = category;
                
                filteredPouch.push_back(addSlot);
                added++;
            }

            return filteredPouch;
        }

        void Pouch::save(){
            auto& porchItem = std::get<std::vector<std::string>>(
                Save::GetEntryFromDataName(*this->saveFile_, kInventoryKey)->value);
            auto& primaryValue = std::get<std::vector<int32_t>>(
                Save::GetEntryFromDataName(*this->saveFile_, kInventoryValueKey)->value);

            std::unordered_map<Category, size_t> indexMap;

            size_t index = 0;
            size_t maxIndex = porchItem.size() - 1;

            for (const auto& item : this->items_){
                // Skip empty items
                if (item->actorName.empty()){ continue; }

                // Save actorName and primary value
                porchItem[index] = item->actorName;
                primaryValue[index] = item->value;

                // Correct bad values
                if (!item->actorName.empty() && IsCategoryEquipment(item->category)){
                    item->flagSpKey = GetFlagSpKeyFromCategory(item->category);
                    item->valueSpKey = GetValueSpKeyFromCategory(item->category);
                }

                // Save valueSp and flagSp
                if (IsCategoryEquipment(item->category) && !item->flagSpKey.empty() 
                    && !item->valueSpKey.empty() && !item->actorName.empty()){

                    auto& flagSp = std::get<std::vector<int32_t>>
                        (Save::GetEntryFromDataName(*this->saveFile_, 
                            item->flagSpKey)->value);

                    auto& valueSp = std::get<std::vector<int32_t>>
                        (Save::GetEntryFromDataName(*this->saveFile_, 
                            item->valueSpKey)->value);

                    size_t localIndex = indexMap[item->category]++;
                    flagSp[localIndex] = item->flagSp;
                    valueSp[localIndex] = item->valueSp;
                }

                index++;
            }
        }

        void Pouch::resetBlankSlots(){
            for (const auto& item : this->items_){
                if (!item->actorName.empty()){
                    continue;
                }

                item->category = Category::Unknown;
            }
        }

        Pouch::~Pouch(){}
    }
}