#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"

#include <Aether/Aether.hpp>

#include <Application/Keyboard.hpp>

#include <Editor/Tile.hpp>

#include <Utils/Logger.hpp>
#include <Config.hpp>

#include <vector>
#include <any>
#include <map>
#include <memory>

#define PROPERTY_Y_SPACING 11
#define OVERLAY_X_SPACING 16
#define ARMOR_STAR_SIZE 22
#define BUFF_STAR_SIZE 18

#define STAR "★"
#define ENHANCED_COLOUR Aether::Colour{255, 255, 0, 255}
#define DUMMY_NAME "Unassigned"

namespace Rune {
    namespace Inventory {
        void Tile::updateThumbnail(const std::string &path){
            if (!path.empty()){
                // Get the thumbnail texture key
                int thumbSize = static_cast<int>(kTileSize*kThumbScale);
                std::string textureKey = path+"|"+std::to_string(thumbSize)+
                    "x"+std::to_string(thumbSize);

                // Check if there is a cached texture
                auto cachedTexture = IconTextureCache
                    ->get<Aether::TextureHandle>(textureKey);

                // Delete the old image
                if (this->tileThumb != nullptr 
                    && this->tileThumb->parent()){

                    this->tileThumb->parent()
                        ->removeElement(this->tileThumb);
                    this->tileThumb = nullptr;
                }

                // Make new image
                if (cachedTexture != std::nullopt){
                    this->tileThumb = new Aether::Image(
                        0, 0, cachedTexture.value(), 
                        Aether::Render::Wait);
                } else {
                    this->tileThumb = new Aether::Image(
                        0, 0, path, 
                        Aether::Render::Wait);
                }

                // Set the thumbnail's size and reposition it
                this->tileThumb
                    ->setScaleDimensions(thumbSize, thumbSize);
                this->tileThumb->setXY(
                    this->tileBtn->centreX()-this->tileThumb->scaleWidth()/2,
                    this->tileBtn->centreY()-this->tileThumb->scaleHeight()/2
                );

                // Add the thumbnail to the tile button
                this->tileBtn->addElement(this->tileThumb);

                // Render the thumbnail
                this->tileThumb->renderAsync();
            }
            else {
                tileBtn->setString(this->inventoryItem.name);
                tileBtn->setTextColour(TEXT_COLOUR);
            }
        }

        void Tile::updateBuffStar(){
            // Create a new buff star if one does not exist
            if (this->buffStar == nullptr){
                this->buffStar = new Aether::Image(
                    this->tileBtn->x()+8, this->tileBtn->y()+8,
                    ASSET_PATHS.at("star_alt"), 
                    Aether::Render::Wait);
                this->buffStar->setScaleDimensions(
                    BUFF_STAR_SIZE, BUFF_STAR_SIZE);
                this->buffStar->renderAsync();
                this->tileBtn->addElement(this->buffStar);
            }

            if (this->inventoryItem.modifier != Modifier::None){
                this->buffStar->setHidden(false);
                if (IsModifierEnhanced(this->inventoryItem.modifier)){
                    buffStar->setColour(ENHANCED_COLOUR);
                }
                else {
                    buffStar->setColour(Aether::Colour{255, 255,255,255});
                }
            }
            else {
                this->buffStar->setHidden(true);
            }
        }

        void Tile::updateTileFormat(TileType type, TileStyle style, std::string str){
            if (!this->tileLabelRect){
                this->tileLabelRect = new Aether::Rectangle(
                    0, 0, kTileSize/3, kTileSize/5, 3);
                this->tileLabelRect->setColour(MUTED_BACKGROUND_COLOUR);
                this->tileLabelRect->setXY(
                    tileBtn->right()-this->tileLabelRect->w()/2-kTilePadding, 
                    tileBtn->bottom()-this->tileLabelRect->h()/2-6);
            }

            if (!this->tileLabelText){
                this->tileLabelText = new Aether::Text(
                    0, 0, str, 18);
            }

            switch (type){
                case (TileType::Standard): {
                    switch (style){
                        case (TileStyle::NoLabel): {
                            this->tileLabelText->setFontSize(18);
                            this->tileLabelText->setXY(
                                tileBtn->x()+14,
                                tileBtn->bottom()
                                -this->tileLabelText->textureHeight()-12);
                        
                            this->tileLabelText->setHidden(false);
                            this->tileLabelRect->setHidden(true);
                            break;
                        }

                        case (TileStyle::Label): {
                            this->tileLabelText->setFontSize(16);
                            this->tileLabelText->setXY(
                                this->tileLabelRect->centreX()
                                -this->tileLabelText->textureWidth()/2,
                                this->tileLabelRect->centreY()
                                -this->tileLabelText->textureHeight()/2
                            );

                            this->tileLabelText->setHidden(false);
                            this->tileLabelRect->setHidden(false);
                            break;
                        }
                    }

                    break;
                }

                case (TileType::Add): {
                    this->tileLabelRect->setHidden(true);
                    this->tileLabelText->setHidden(true);

                    tileBtn->setString("+");
                    tileBtn->onPress([this](){
                        auto slotKeyIt = kSlotKeys
                            .find(inventoryItem.category);
                        if (slotKeyIt == kSlotKeys.end()){
                            return;
                        }

                        std::string slotKey = slotKeyIt->second;
                        Save::Entry* slotEntry = Save::GetEntryFromDataName(
                            *this->saveFile,
                            slotKey);

                        auto maxSlotsIt = kMaxSlots.find(
                            inventoryItem.category);
                        int maxSlots = -1;

                        if (maxSlotsIt != kMaxSlots.end()){
                            maxSlots = maxSlotsIt->second;
                        }

                        int32_t newSlotCount = std::get<int32_t>(
                            slotEntry->value)+1;
                        if (newSlotCount > maxSlots && maxSlots >= 0){
                            newSlotCount = maxSlots;
                        }
                        
                        slotEntry->value = newSlotCount;
                        this->menu->requestReload();
                    });
                    break;
                }  

                default:
                    this->tileLabelRect->setHidden(true);
                    this->tileLabelText->setHidden(true);
                    break;
            }
        }

        void Tile::setOvlState(bool open){
            if (open){
                this->ovlOpen = true;
                this->window->addOverlay(this->ovl);
            }
            else {
                this->ovlOpen = false;
                // this->ovl->closeWidgetOverlay(isSave);

                // nullify property editors
                this->visiblePropertyEditors.clear();
                this->ovlPrimaryValueStepper = nullptr;
                this->ovlSecondaryFlagDropdown = nullptr;
                this->ovlSecondaryValueStepper = nullptr;
                this->ovlArmorStarStepper = nullptr;
            }
        }

        void Tile::buildOvl(){
            this->ovlImgFrame = new Aether::Rectangle(60, 0,
                280, 280, 8);
            ovlImgFrame->setY(
                (this->ovl->panel()->centreY())
                -(ovlImgFrame->h()/2));
            ovlImgFrame->setColour(MUTED_BACKGROUND_COLOUR);
            this->ovl->addTempElement(ovlImgFrame);

            this->ovlImgText = new Aether::Text(0, 0, "Unassigned",
                20);
            ovlImgText->setXY(ovlImgFrame->centreX()-ovlImgText->textureWidth()/2,
                ovlImgFrame->bottom()-ovlImgText->textureHeight()-16);
            this->ovl->addTempElement(ovlImgText);

            this->ovlArmorStars = new Aether::Text(0, 0, STAR,
                 ARMOR_STAR_SIZE);
            ovlArmorStars->setXY(ovlImgFrame->x()+12, 
            ovlImgFrame->y()+ovlArmorStars->textureHeight()/2);
            ovlArmorStars->setString("");
            this->ovl->addTempElement(ovlArmorStars);

            Aether::Rectangle* sepA = new Aether::Rectangle(
                ovlImgFrame->right()+OVERLAY_X_SPACING,
                ovlImgFrame->y(), 1, ovlImgFrame->h());
            sepA->setColour(LIGHT_SEPARATOR_COLOUR);
            this->ovl->addTempElement(sepA);
            
            int cursorX=sepA->right()+OVERLAY_X_SPACING;
            Aether::Text* propertiesText = new Aether::Text(
                cursorX, sepA->y(), 
                "Item Properties", 22);
            propertiesText->setColour(TEXT_COLOUR);
            this->ovl->addTempElement(propertiesText);

            this->ovlPropertiesBox = new Aether::Rectangle(
                cursorX, propertiesText->bottom()+PROPERTY_Y_SPACING,
                496,189, 6);
            ovlPropertiesBox->setColour(MUTED_BACKGROUND_COLOUR);
            this->ovl->addTempElement(ovlPropertiesBox);

            this->ovlNoEditText = new Aether::Text(0, 0, 
                    "No editable properties!", 28);
            ovlNoEditText->setColour(MUTED_TEXT_COLOR);
            ovlNoEditText->setXY(ovlPropertiesBox->centreX()
                -ovlNoEditText->textureWidth()/2,
                    ovlPropertiesBox->centreY()
                -ovlNoEditText->textureHeight()/2
            );
            ovlNoEditText->setHidden(true);
            ovl->addTempElement(ovlNoEditText);

            cursorX=ovlPropertiesBox->right()+OVERLAY_X_SPACING;
            Aether::Rectangle* sepB = new Aether::Rectangle(
                cursorX, sepA->y(), sepA->w(), sepA->h());
            sepB->setColour(LIGHT_SEPARATOR_COLOUR);
            this->ovl->addTempElement(sepB);

            int btnWidth = 148;
            int btnSpacing = (ovlPropertiesBox->w()-(btnWidth*3))/2;

            cursorX = ovlPropertiesBox->x();
            int cursorY = ovlPropertiesBox->bottom()+12;

            this->ovlSwapBtn = new Aether::BorderButton(
                0, 0, btnWidth, 42, 
                3, "", 1, [](){});
            Aether::Image* ovlSwapImg = new Aether::Image(
                0, 0, ASSET_PATHS.at("swap"), 
                Aether::Render::Wait);
            Aether::Text* ovlSwapText = new Aether::Text(
                0, 0, "Swap",  18);

            ovlSwapImg->setScaleDimensions(24, 24);
            ovlSwapImg->renderAsync();
            
            ovlSwapBtn->setXY(cursorX, cursorY);
            ovlSwapImg->setXY(cursorX+32, ovlSwapBtn->centreY()-ovlSwapImg->scaleHeight()/2);
            ovlSwapText->setXY(ovlSwapImg->right()+12, ovlSwapBtn->centreY()-ovlSwapText->textureHeight()/2);
            
            ovlSwapBtn->setBorderColour(LIGHT_SEPARATOR_COLOUR);
            ovlSwapBtn->setFillColour(LIGHT_BACKGROUND_COLOUR);

            this->ovl->addTempElement(ovlSwapBtn);
            ovlSwapBtn->addElement(ovlSwapImg);
            ovlSwapBtn->addElement(ovlSwapText);

            ovlSwapBtn->onPress([this](){
                this->onSwap();
            });

            // -------------

            cursorX = ovlSwapBtn->right()+btnSpacing;
            this->ovlDeleteBtn = new Aether::BorderButton(
                0, 0, btnWidth, 42, 
                3, "", 1, [](){});
            Aether::Image* ovlDeleteImg = new Aether::Image(
                0, 0, ASSET_PATHS.at("bin"), 
                Aether::Render::Wait);
            Aether::Text* ovlDeleteText = new Aether::Text(
                0, 0, "Clear",  18);

            ovlDeleteImg->setScaleDimensions(24, 24);
            ovlDeleteImg->renderAsync();
            
            ovlDeleteBtn->setXY(cursorX, cursorY);
            ovlDeleteImg->setXY(cursorX+32, ovlDeleteBtn->centreY()-ovlDeleteImg->scaleHeight()/2);
            ovlDeleteText->setXY(ovlDeleteImg->right()+12, ovlDeleteBtn->centreY()-ovlDeleteText->textureHeight()/2);
            
            ovlDeleteBtn->setBorderColour(Aether::Colour{96, 0, 0, 150});
            ovlDeleteBtn->setFillColour(Aether::Colour{180, 0, 0, 255});

            this->ovl->addTempElement(ovlDeleteBtn);
            ovlDeleteBtn->addElement(ovlDeleteImg);
            ovlDeleteBtn->addElement(ovlDeleteText);

            ovlDeleteBtn->onPress([this](){
                this->onDelete();
            });
        }

        void Tile::updateOvlText(){
            // Safeguard
            if (!this->ovlOpen){
                return;
            }

            std::string itemName = (!inventoryItem.name.empty()) 
                ? inventoryItem.name : DUMMY_NAME;

            ovl->setTitleLabel("Edit Item - "+itemName);
            ovlImgText->setString(itemName);
            ovlImgText->setX(ovlImgFrame->centreX()-ovlImgText->textureWidth()/2);

            auto devNoteIt = kCategoryDevNotes.find(inventoryItem.category);
            if (devNoteIt != kCategoryDevNotes.end()){
                ovl->setDeveloperNoteWrap(324);
                ovl->setDeveloperNote(devNoteIt->second);
                ovl->setDeveloperNoteX(this->ovlPropertiesBox->right()
                    +2*OVERLAY_X_SPACING);
            }

            if (this->visiblePropertyEditors.empty()){
                this->ovlNoEditText->setHidden(false);
            }
        }

        void Tile::updateOvlImg(){
            if (!this->ovlOpen){
                return;
            }

            if (this->ovlImg != nullptr 
                && this->ovlImg->parent()){
                this->ovlImg->parent()
                ->removeElement(this->ovlImg);
            }

            // Update thumbnail path
            inventoryItem.thumbPath = Inventory
                ::GetThumbPathFromInventoryItem(inventoryItem);

            // Make a new thumbnail and reposition it
            this->ovlImg = new Aether::Image(0, 0, 
                inventoryItem.thumbPath, Aether::Render::Wait);
            ovlImg->setScaleDimensions(210, 210);
            ovlImg->setXY(ovlImgFrame->centreX()-ovlImg->scaleWidth()/2,
                ovlImgFrame->y()+10);
            ovlImg->renderAsync();
            this->ovl->addTempElement(ovlImg);
        }

        void Tile::updatePropertyEditors(){
            if (!ovlOpen){ return; }

            // Clear existing editors
            for (auto& editor : std::vector<Aether::Element*>{
                ovlPrimaryValueStepper, ovlSecondaryFlagDropdown,
                ovlSecondaryValueStepper, ovlArmorStarStepper
            }){
                if (editor != nullptr && editor->parent()){
                    editor->parent()->removeElement(editor);
                    editor = nullptr;
                }
            }

            int cursorX = ovlPropertiesBox->x()+16;
            int cursorY = ovlPropertiesBox->y()+12;

            this->ovlPrimaryValueStepper = new Aether::Stepper(
                cursorX, cursorY, "Primary Val.");

            switch (inventoryItem.category){
                case Category::Weapons:
                case Category::Bows:
                case Category::Shields: {
                    ovlPrimaryValueStepper->setString("Durability");
                    ovlPrimaryValueStepper->setMax(
                        __INT32_MAX__/kDurabilityMultiplier);
                    ovlPrimaryValueStepper->setValue(
                        inventoryItem.value/kDurabilityMultiplier);
                    cursorY += ovlPrimaryValueStepper->h()+PROPERTY_Y_SPACING;

                    this->ovlSecondaryFlagDropdown = new Aether::Dropdown(cursorX,
                        cursorY, "Modifier");
                    ovlSecondaryFlagDropdown->setValue(
                        ModifierName((Modifier)inventoryItem.flagSp));
                    
                    std::map<std::string, std::any> buffMap;
                    const auto& categoryModifiers = kCategoryModifiers
                        .find(inventoryItem.category);
                    if (categoryModifiers != kCategoryModifiers.end()){
                        for (const Modifier& modifier : categoryModifiers->second){
                            buffMap[Inventory::ModifierName(modifier,
                                categoryModifiers->first)] = std::make_any<Modifier>
                                (modifier);
                        }
                    }
                    ovlSecondaryFlagDropdown->setMap(buffMap);
                    visiblePropertyEditors.push_back(ovlSecondaryFlagDropdown);
                    cursorY += ovlSecondaryFlagDropdown->h()+PROPERTY_Y_SPACING;

                    this->ovlSecondaryValueStepper = new Aether::Stepper(cursorX,
                        cursorY, "Modifier Val.");
                    ovlSecondaryValueStepper->setMax(__UINT16_MAX__);
                    ovlSecondaryValueStepper->setValue(inventoryItem.valueSp);
                    visiblePropertyEditors.push_back(ovlSecondaryValueStepper);
                    this->ovl->addTempElement(ovlSecondaryValueStepper);
                    this->ovl->addTempElement(ovlSecondaryFlagDropdown);

                    // Show keyboard for manual input on press
                    ovlSecondaryValueStepper->onPress([this](){
                        float out;
                        std::string prompt = "Enter a number... ("
                            +std::to_string(this->ovlSecondaryValueStepper->min())
                            +" → "+std::to_string(
                                this->ovlSecondaryValueStepper->max())+")";
                        
                        // I just felt like doing this to break the pattern
                        // oh also the reason everything is so narrow is cuz I have
                        // this file and the old tile.cpp open at the same time to
                        // copy logic
                        if (R_SUCCEEDED(Keyboard::getNumberInput(&out, prompt, 
                            this->ovlSecondaryValueStepper->min(), 
                            this->ovlSecondaryValueStepper->max(),
                            false,
                            (this->ovlSecondaryValueStepper->min() < 0)))){

                                this->ovlSecondaryValueStepper->setValue(
                                    static_cast<int>(out));
                            }
                    });

                    break;
                }

                case Category::Arrows:
                case Category::Materials:
                case Category::Food:
                case Category::KeyItems: {
                    if (GetActorValue<bool>(inventoryItem.actorName, "CanStack").value_or(false)){
                        ovlPrimaryValueStepper->setString("Quantity");
                        ovlPrimaryValueStepper->setMax(kMaxInventoryQuantity);
                        ovlPrimaryValueStepper->setValue(inventoryItem.value);
                    } else {
                        ovlPrimaryValueStepper->setMin(inventoryItem.value);
                        ovlPrimaryValueStepper->setMax(inventoryItem.value);
                        ovlPrimaryValueStepper->setHidden(true);
                    }

                    break;
                }

                case Category::Armor: {
                    if (IsArmorDyeable(inventoryItem.actorName)){
                        ovlPrimaryValueStepper->setString("Dye Colour");
                        ovlPrimaryValueStepper->setValue(inventoryItem.value);
                        ovlPrimaryValueStepper->setMax(kMaxDyeColour);
                        cursorY += ovlPrimaryValueStepper->h()+PROPERTY_Y_SPACING;
                    }
                    else {
                        ovlPrimaryValueStepper->setMinMax(-1, -1);
                        ovlPrimaryValueStepper->setHidden(true);
                    }

                    if (!IsArmorUpgradeable(inventoryItem.actorName)) { 
                        break; 
                    }
                    
                    auto starCount = std::make_shared<int>(
                        GetArmorStarNum(inventoryItem.actorName));
                    if (*starCount <= 0){ break; }

                    this->ovlArmorStarStepper = new Aether::Stepper(cursorX, 
                        cursorY, "Stars");
                    ovlArmorStarStepper->setMinMax(0, 4);

                    for (int s = 0; s < *starCount-1; s++){ 
                        this->ovlArmorStars->setString(
                            this->ovlArmorStars->string()+STAR); 
                    }
                    ovlArmorStarStepper->setValue(*starCount-1);

                    // Change stars and visual when stepper changes
                    ovlArmorStarStepper->onValueChange([this, starCount](){
                        int newStarCount = this->ovlArmorStarStepper->value()+1;
                        int diff = newStarCount-*starCount;
                        if (diff == 0) { return; }

                        std::string newActor;
                        if (diff > 0){
                            newActor = GetNextArmorRankActorName(
                                this->inventoryItem.actorName);
                        }
                        else {
                            newActor = GetPrevArmorRankActorName(
                                this->inventoryItem.actorName);
                        }

                        if (newActor.empty()){ return; }
                        this->inventoryItem.actorName = newActor;
                        *starCount = newStarCount;

                        this->ovlArmorStars->setString("");
                        for (int s = 0; s < *starCount-1; s++){ 
                            this->ovlArmorStars->setString(
                                this->ovlArmorStars->string()+STAR); 
                        }
                    });

                    this->ovl->addTempElement(ovlArmorStarStepper);
                    this->visiblePropertyEditors.push_back(
                        ovlArmorStarStepper);
                    break;
                }

                default:
                    break;
            }

            if (ovlPrimaryValueStepper->isVisible()){
                // Add to screen if visible
                this->visiblePropertyEditors
                    .push_back(ovlPrimaryValueStepper);
                this->ovl->addTempElement(ovlPrimaryValueStepper);

                // Allow manual input
                ovlPrimaryValueStepper->onPress([this](){
                    float out;
                    std::string prompt = "Enter a number... ("
                        +std::to_string(this->ovlPrimaryValueStepper->min())
                        +" → "+std::to_string(
                            this->ovlPrimaryValueStepper->max())+")";
                    
                    if (R_SUCCEEDED(Keyboard::getNumberInput(&out, prompt, 
                        this->ovlPrimaryValueStepper->min(), 
                        this->ovlPrimaryValueStepper->max(), 
                        false,
                        (this->ovlPrimaryValueStepper->min() < 0)))){

                            this->ovlPrimaryValueStepper->setValue(
                                static_cast<int>(out));
                        }
                });
            }

            this->ovlNoEditText->setHidden(
                !visiblePropertyEditors.empty());
        }

        Tile::Tile(EditorWidget* widget){
            // Unpack and validate meta
            std::vector<std::any> widgetMeta = widget->getMeta();
            if (widgetMeta.empty()){
                Logger::Important("Failed to make Tile widget: "
                    "widget meta was empty!");
                return;
            }

            // Set properties
            this->widgetMeta = widgetMeta;
            this->widget = widget;
            this->menu = widget->getMenu();
            this->window = this->menu->window();
            this->saveFile = this->menu->saveFile();
            this->saveEntry = this->widget->getSaveEntry();
            this->originalInventoryItem = std::any_cast<Item>(widgetMeta.at(0));
            this->inventoryItem = this->originalInventoryItem; // Editable copy

            // // Build the tile button // // 
            int buttonX = this->inventoryItem.x;
            int buttonY = this->inventoryItem.y;
            int buttonH = this->inventoryItem.h;
            int buttonW = this->inventoryItem.w;

            // The text that is displayed on the little label in the bottom 
            // right/left of the tile
            std::string labelText = std::any_cast<std::string>
                (widgetMeta.at(1));
            
            this->tileBtn = std::any_cast<Aether::BorderButton*>
                (widgetMeta.at(2));
            this->tileBtn->setXYWH(buttonX, buttonY, buttonW, buttonH);
            this->tileBtn->setBorderColour(EDITOR_SIDEBAR_COLOUR);

            auto debugTxt = new Aether::Text(
                tileBtn->x(), tileBtn->y(),
                std::to_string(inventoryItem.index), 32
            );
            tileBtn->addElement(debugTxt);

            // Set the tile elements
            this->updateThumbnail(inventoryItem.thumbPath);
            this->updateBuffStar();
            // // Tile button built // //

            this->tileBtn->onPress([this](){
                this->onPress();
            });
            this->updateTileFormat(inventoryItem.type, 
                inventoryItem.style, labelText);

            this->widget->addToElements(tileBtn);
            this->widget->addToElements(this->tileLabelRect);
            this->widget->addToElements(this->tileLabelText);
        }

        void Tile::onDelete(){
            
        }

        void Tile::onSwap(){

        }

        void Tile::onPress(){
            // Assign widget overlay
            if (this->ovl == nullptr){
                this->ovl = this->widget->getWidgetOverlay();
            }
            
            if (this->connId == 0){
                this->connId = this->ovl->onCloseSignal->connect(
                    [this](bool isSave){
                    this->onClose(isSave);
                }).getId();
            }
            
            // Build the core overlay elements and open the overlay
            this->buildOvl();
            this->setOvlState(true);

            this->updateOvlText();
            this->updateOvlImg();
            this->updatePropertyEditors();
        }

        void Tile::onClose(bool isSave){
            if (isSave){
                // ActorName changed
                Save::Entry* porchItemEntry = Save::GetEntryFromDataName(*this->saveFile, kInventoryKey);
                auto& porchItem = std::get<std::vector<std::string>>(porchItemEntry->value);
                
                if (inventoryItem.index >= porchItem.size()){
                    Logger::Importantf("(PorchItem) Error: Index %d is OOB", inventoryItem.index);
                } else {
                    std::string oldName = porchItem.at(inventoryItem.index);
                    porchItem[inventoryItem.index] = "Weapon_Sword_070";
                    Logger::Importantf("Saved ActorName Update (%s -> %s)", oldName.c_str(), inventoryItem.actorName.c_str());
                }

                // PorchItem_Value1 changed
                Save::Entry* primaryValueEntry = Save::GetEntryFromDataName(*this->saveFile, kInventoryValueKey);
                auto& primaryValue = std::get<std::vector<int32_t>>(primaryValueEntry->value);
                int newPrimaryValue=this->ovlPrimaryValueStepper->value();

                if (inventoryItem.category == Category::Weapons ||
                    inventoryItem.category == Category::Bows || 
                    inventoryItem.category == Category::Shields){
                    
                    newPrimaryValue *= kDurabilityMultiplier;
                }

                if (inventoryItem.index >= primaryValue.size()){
                    Logger::Importantf("(Value1) Error: Index %d is OOB", inventoryItem.index);
                } else {
                    primaryValue[inventoryItem.index] = newPrimaryValue;
                    Logger::Important("Saved Value1 Update.");
                }

                // PorchItem_FlagSp changed
                if (this->ovlSecondaryFlagDropdown != nullptr){
                    Save::Entry* secondaryFlagEntry = Save
                        ::GetEntryFromDataName(*this->saveFile, 
                        inventoryItem.flagSpKey);

                    auto& secondaryFlags = std::get<std::vector<int32_t>>(
                        secondaryFlagEntry->value);
                    int32_t oldFlag = secondaryFlags.at(
                        this->inventoryItem.localIndex);

                    if (inventoryItem.localIndex >= secondaryFlags.size()){
                        Logger::Importantf("(FlagSp) Error: Index %d is OOB", inventoryItem.localIndex);
                    } else {
                        secondaryFlags[this->inventoryItem.localIndex]
                            = static_cast<int32_t>(
                                this->ovlSecondaryFlagDropdown->getValueData
                                <Modifier>().value_or((Modifier)oldFlag)
                            );
                        Logger::Important("Saved FlagSp Update.");
                    }
                }

                // PorchItem_ValueSp changed
                if (this->ovlSecondaryValueStepper != nullptr){
                    Save::Entry* secondaryValueEntry = Save
                        ::GetEntryFromDataName(*this->saveFile, 
                        inventoryItem.valueSpKey);
                    auto& secondaryValues = std::get<std::vector<int32_t>>(
                        secondaryValueEntry->value);
                        
                    if (inventoryItem.localIndex >= secondaryValues.size()){
                        Logger::Importantf("(ValueSp) Error: Index %d is OOB", inventoryItem.localIndex);
                    } else {
                        secondaryValues[this->inventoryItem.localIndex] 
                            = this->ovlSecondaryValueStepper->value();
                        Logger::Important("Saved ValueSp Update.");
                    }
                }

                // Request for the menu to reload to generate new tile
                this->menu->requestReload();
                Logger::Important("Requested Menu Reload.");
            }
            else {
                // Nothing was changed, no reload needed
                this->inventoryItem = originalInventoryItem;
            }

            this->setOvlState(false);
        }

        Tile::~Tile(){
            // Clean connection to close signal
            if (this->ovl != nullptr && this->connId != 0){
                this->ovl->onCloseSignal->disconnect(this->connId);
                this->connId = 0;
            }
        }

    } // namespace Inventory

    void WidgetBuilder::Tile(EditorWidget* widget){
        Inventory::Tile* tile = new Inventory::Tile(widget);
        widget->getMenu()->giveTask([tile](){
            delete tile;
        });
    }
} // namespace Rune

#pragma GCC diagnostic pop