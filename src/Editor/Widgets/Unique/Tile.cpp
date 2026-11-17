#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"

#include <Aether/Aether.hpp>

#include <Application/Application.hpp>
#include <Application/Keyboard.hpp>
#include <Application/Audio.hpp>

#include <Editor/Menus/Inventory.hpp>
#include <Editor/Widgets/Tile.hpp>

#include <Utils/Logger.hpp>
#include <Utils/Utils.hpp>
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
        void Tile::updateThumbnail(){
            inventoryItem.thumbPath = 
                GetThumbPathFromInventoryItem(inventoryItem);
            const std::string& path = inventoryItem.thumbPath;

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
                        default:
                            // If there is an image, show it
                            if (this->tileThumb != nullptr && tileThumb->hidden()){
                                tileThumb->setHidden(false);
                            }

                            // Stub out the onSelect (so the edit icon does not show when highlighted)
                            tileBtn->onHighlight([](bool){});

                            // If there is an edit icon (that shows for empty tiles), hide it
                            if (this->tileEditImg != nullptr && !tileEditImg->hidden()){
                                tileEditImg->setHidden(true);
                            }

                        case (TileStyle::NoLabel): {
                            this->tileLabelText->setFontSize(18);
                            this->tileLabelText->setString(str);
                            this->tileLabelText->setXY(
                                tileBtn->x()+14,
                                tileBtn->bottom()
                                -this->tileLabelText->textureHeight()-12);
                        
                            this->tileLabelText->setHidden(!GetActorValue<bool>(inventoryItem.actorName, 
                                "CanStack").value_or(false));
                            this->tileLabelRect->setHidden(true);
                            break;
                        }

                        case (TileStyle::Label): {
                            this->tileLabelText->setFontSize(16);
                            this->tileLabelText->setString(str);
                            this->tileLabelText->setXY(
                                this->tileLabelRect->centreX()
                                -this->tileLabelText->textureWidth()/2,
                                this->tileLabelRect->centreY()
                                -this->tileLabelText->textureHeight()/2);

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

                case (TileType::Empty): {
                    this->tileLabelRect->setHidden(true);
                    this->tileLabelText->setHidden(true);

                    // Make the edit image that shows on selection
                    Aether::Image* tileEditImg = new Aether::Image(0, 0,
                        ASSET_PATHS.at("edit"), Aether::Render::Wait);
                    tileEditImg->setScaleDimensions(tileBtn->w()*0.65, tileBtn->h()*0.65);
                    tileEditImg->setXY(tileBtn->centreX()-tileEditImg->scaleWidth()/2, 
                    tileBtn->centreY()-tileEditImg->scaleHeight()/2);
                    tileEditImg->renderAsync();
                    tileEditImg->setHidden(!tileBtn->highlighted());
                    tileEditImg->setColour(Aether::Colour{255, 255, 255, 90});

                    tileBtn->addElement(tileEditImg);

                    // Highlight bind
                    tileBtn->onHighlight([this](bool h){
                        if (!this->tileEditImg){
                            this->tileEditImg = new Aether::Image(0, 0,
                                ASSET_PATHS.at("edit"), Aether::Render::Wait);
                            this->tileEditImg->setScaleDimensions(tileBtn->w()*0.65, tileBtn->h()*0.65);
                            this->tileEditImg->centreAtElement(this->tileBtn);
                            this->tileEditImg->renderAsync();
                            this->tileEditImg->setHidden(!tileBtn->highlighted());
                            this->tileEditImg->setColour(Aether::Colour{255, 255, 255, 90});
                        }

                        this->tileEditImg->setHidden(!h);
                    });

                    // Check if there is an image, and if there is, hide it
                    if (this->tileThumb != nullptr && !this->tileThumb->hidden()){
                        this->tileThumb->setHidden(true);
                    }

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
            }
            else {
                this->ovlOpen = false;

                // nullify property editors
                this->visiblePropertyEditors.clear();
                this->ovlPrimaryValueStepper = nullptr;
                this->ovlSecondaryFlagDropdown = nullptr;
                this->ovlSecondaryValueStepper = nullptr;
                this->ovlArmorStarStepper = nullptr;
                this->ovlImg = nullptr;
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

            this->ovlImgText = new Aether::Text(0, 0, DUMMY_NAME,
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
                Logger::Important("[TILE] - Swap button pressed.");
                this->onSwap();
            });

            // -------------
            
            cursorX = ovlSwapBtn->right()+btnSpacing;

            // Add reset button if weapon, bow or shield
            switch (inventoryItem.category){
                case Category::Weapons:
                case Category::Bows:
                case Category::Shields: {
                    this->ovlResetBtn = new Aether::BorderButton(
                        0, 0, btnWidth, 42,
                        3, "", 1, [](){}
                    );
                    Aether::Image* ovlResetImg = new Aether::Image(
                        0, 0, ASSET_PATHS.at("reset"),
                        Aether::Render::Wait
                    );
                    Aether::Text* ovlResetText = new Aether::Text(
                        0, 0, "Reset", 18
                    );

                    ovlResetImg->setScaleDimensions(24, 24);
                    ovlResetImg->renderAsync();

                    ovlResetBtn->setXY(cursorX, cursorY);
                    ovlResetImg->setXY(cursorX+32, ovlResetBtn->centreY()-ovlResetImg->scaleHeight()/2);
                    ovlResetText->setXY(ovlResetImg->right()+12, ovlResetBtn->centreY()-ovlResetText->textureHeight()/2);

                    ovlResetBtn->setBorderColour(LIGHT_SEPARATOR_COLOUR);
                    ovlResetBtn->setFillColour(LIGHT_BACKGROUND_COLOUR);

                    this->ovl->addTempElement(ovlResetBtn);
                    ovlResetBtn->addElement(ovlResetImg);
                    ovlResetImg->addElement(ovlResetText);

                    ovlResetBtn->onPress([this](){
                        this->onReset();
                    });

                    cursorX = ovlResetBtn->right()+btnSpacing;
                    break;
                }

                default:
                    break;
            }

            // -------------

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

            inventoryItem.name = GetNameFromActorName(inventoryItem.actorName);
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

            if (this->ovlImg != nullptr){
                this->ovl->removeTempElement(this->ovlImg);
            }

            // Update thumbnail path
            inventoryItem.thumbPath = GetThumbPathFromInventoryItem(inventoryItem);

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
                if (editor != nullptr){
                    this->ovl->removeTempElement(editor);
                }
            }

            this->ovlPrimaryValueStepper = nullptr;
            this->ovlSecondaryFlagDropdown = nullptr;
            this->ovlSecondaryValueStepper = nullptr;
            this->ovlArmorStarStepper = nullptr;

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
                    ovlPrimaryValueStepper->setDefaultValue(
                        GetMaxDurability(inventoryItem.actorName));
                    cursorY += ovlPrimaryValueStepper->h()+PROPERTY_Y_SPACING;

                    this->ovlSecondaryFlagDropdown = new Aether::Dropdown(cursorX,
                        cursorY, "Modifier", 180, 180);
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
                    ovlSecondaryValueStepper->onValueChange([this](){
                        Audio::playSound("cursor");

                        // Update my copy on change
                        this->inventoryItem.valueSp = this->ovlSecondaryValueStepper->value();
                    });
                    ovlSecondaryValueStepper->onCursorMove([](){
                        Audio::playSound("cursor");
                    });
                    visiblePropertyEditors.push_back(ovlSecondaryValueStepper);
                    this->ovl->addTempElement(ovlSecondaryValueStepper);
                    this->ovl->addTempElement(ovlSecondaryFlagDropdown);

                    // Hide Modifier Val. stepper if value is not needed
                    ovlSecondaryFlagDropdown->onValueChange([this](){
                        if (this->ovlSecondaryFlagDropdown == nullptr) { return; }
                        Audio::playSound("decide");

                        auto modOpt = this->ovlSecondaryFlagDropdown->getValueData<Modifier>();
                        bool needsEditor = false;

                        if (modOpt.has_value()){
                            Modifier currentModifier = modOpt.value();
                            needsEditor = ModifierNeedsValueEditor(currentModifier);

                            // Update my copy on change
                            this->inventoryItem.flagSp = static_cast<int32_t>(currentModifier);
                            this->inventoryItem.modifier = currentModifier;
                        }
                        
                        // Hide the value stepper, and reset it to 1
                        this->ovlSecondaryValueStepper->setHidden(!needsEditor);
                        if (!needsEditor){ this->ovlSecondaryValueStepper->setValue(1); }
                    });

                    ovlSecondaryFlagDropdown->setValue(
                        ModifierName(static_cast<Modifier>(inventoryItem.flagSp)));

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
                        // Set value to 1 by default (0 slots may cause instability)
                        if (inventoryItem.value == 0){
                            inventoryItem.value = 1;
                        }
                        ovlPrimaryValueStepper->setMin(inventoryItem.value);
                        ovlPrimaryValueStepper->setMax(inventoryItem.value);
                        ovlPrimaryValueStepper->setValue(inventoryItem.value);
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
                        Audio::playSound("cursor");
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

                        // Update my copy on change
                        this->inventoryItem.actorName = newActor;
                        *starCount = newStarCount;

                        this->ovlArmorStars->setString("");
                        for (int s = 0; s < *starCount-1; s++){ 
                            this->ovlArmorStars->setString(
                                this->ovlArmorStars->string()+STAR); 
                        }
                    });

                    ovlArmorStarStepper->onCursorMove([](){
                        Audio::playSound("cursor");
                    });

                    this->ovl->addTempElement(ovlArmorStarStepper);
                    this->visiblePropertyEditors.push_back(
                        ovlArmorStarStepper);
                    break;
                }

                default:
                    break;
            }

            if (!ovlPrimaryValueStepper->hidden()){
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

                ovlPrimaryValueStepper->onValueChange([this](){
                    Audio::playSound("cursor");

                    // Update my copy on change
                    int newPrimaryValue = this->ovlPrimaryValueStepper->value();
                    if (this->inventoryItem.category == Category::Weapons ||
                        this->inventoryItem.category == Category::Bows || 
                        this->inventoryItem.category == Category::Shields){
                        
                        newPrimaryValue *= kDurabilityMultiplier;
                    }
                    this->inventoryItem.value = newPrimaryValue;

                    if (this->inventoryItem.category == Category::Armor){
                        this->updateOvlImg();
                    }
                });
                ovlPrimaryValueStepper->onCursorMove([](){
                    Audio::playSound("cursor");
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
            this->app = Application::getApplication();
            this->window = this->app->window();
            this->saveFile = this->menu->saveFile();
            this->saveEntry = this->widget->getSaveEntry();
            this->originalInventoryItem = std::any_cast<std::shared_ptr<Item>>(widgetMeta.at(0));
            this->inventoryItem = *this->originalInventoryItem; // Editable copy

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

            // auto debugTxt = new Aether::Text(
            //     tileBtn->x(), tileBtn->y(),
            //     std::to_string(inventoryItem.index), 32
            // );
            // tileBtn->addElement(debugTxt);

            // Set the tile elements
            this->updateThumbnail();
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
            if (this->ovl == nullptr){
                return;
            }
            // Audio::playSound("select");

            this->inventoryItem.actorName = "";

            this->inventoryItem.value = 0;
            this->inventoryItem.valueSp = 0;
            this->inventoryItem.flagSp = 0;

            this->updateOvlImg();
            this->updateOvlText();
            this->updatePropertyEditors();
        }

        void Tile::onSwap(){
            Logger::Important("[TILE] - Running onSwap()...");
            if (this->ovl == nullptr){
                Logger::Important("[TILE] - ovl is nullptr!");
                return;
            }

            Logger::Important("[TILE] - Playing menu open sound...");
            Audio::playSound("open_menu");

            // Disable exiting the overlay temporarily
            Logger::Important("[TILE] - Disabling exiting the overlay...");
            this->ovl->setExitEnabled(false);

            // Show loading message
            Logger::Important("Getting the application...");
            Application* app = Application::getApplication();

            Aether::MessageBox* loadingMsg = app->showLoading("Loading items...", true);
            loadingMsg->onButtonPress(Aether::Button::B, [](){}); // Stub exiting the menu
            
            Logger::Importantf("[TILE] - Category: %d", static_cast<int>(inventoryItem.category));

            // Get a list of available actor names
            std::vector<std::string> otherActorNames 
                = CategoryActors.find(inventoryItem.category)->second;

            Logger::Importantf("[TILE] - Found other actor names");

            // Make a main containter
            Aether::Overlay* swapOvl = new Aether::Overlay();

            // Make a dark background rect
            Aether::Rectangle* swapBg = new Aether::Rectangle(0, 0, WINDOW_RESOLUTION_X, WINDOW_RESOLUTION_Y);
            swapBg->setColour(Aether::Colour{0, 0, 0, 150});
            swapOvl->addElement(swapBg);

            // Show other controls
            Aether::ControlBar* ctrlBar = new Aether::ControlBar();
            ctrlBar->addControl(Aether::Button::B, "Cancel");
            ctrlBar->addControl(Aether::Button::X, "Search");
            swapOvl->addElement(ctrlBar);

            // Make a central list frame
            Aether::Rectangle* listFrame = new Aether::Rectangle(0, 0, 960, 540, 6);
            listFrame->setColour(BACKGROUND_COLOUR);
            listFrame->setXY(
                swapBg->centreX()-listFrame->w()/2,
                swapBg->centreY()-listFrame->h()/2);
            swapOvl->addElement(listFrame);

            // Add instructional text
            Aether::Text* hintText = new Aether::Text(0, 0, "Select an Item", 32);
            hintText->setXY(
                listFrame->centreX()-hintText->textureWidth()/2,
                listFrame->y()-hintText->textureHeight()-24);
            hintText->setColour(TEXT_COLOUR);
            swapOvl->addElement(hintText);

            // Make the list
            Aether::List* list = new Aether::List(
                listFrame->x(), listFrame->y(),
                listFrame->w(), listFrame->h(),
                Aether::Padding::FitScrollbar
            );
            list->setScrollBarColour(LIGHT_SEPARATOR_COLOUR);
            list->onCursorMove([](){
                Audio::playSound("cursor");
            });
            swapOvl->addElement(list);

            // Build the options
            Aether::ListButton* firstOptn = nullptr;
            auto listBtns = std::make_shared<std::unordered_map<std::string, std::pair<std::string, Aether::ListButton*>>>();

            // Incremented when a list button is generated
            size_t genProgress = 0;

            for (const std::string& otherActor : otherActorNames){
                std::string fullName = GetNameFromActorName(otherActor);
                Aether::ListButton* actorOptn = new Aether::ListButton(
                    fullName, [this, swapOvl, otherActor](){
                        // Audio::playSound("select");
                        this->inventoryItem.actorName = otherActor;

                        this->updateOvlImg();
                        this->updateOvlText();
                        this->updatePropertyEditors();

                        this->ovl->setExitEnabled(true);
                        this->window->removeOverlay(swapOvl);

                        this->ovl->focusElement(this->ovlSwapBtn);
                    }
                );
                actorOptn->setLineColour(LIGHT_SEPARATOR_COLOUR);
                // actorOptn->setTextColour(TOP_R);

                list->addElement(actorOptn);
                
                toLower(fullName);
                (*listBtns)[otherActor] = {fullName, actorOptn};

                if (!firstOptn){
                    firstOptn = actorOptn;
                    firstOptn->setY(list->y()+12);
                }


                // Update loading bar
                genProgress++;
                if (genProgress % 12 == 0){
                    app->setLoadingProgress(static_cast<float>(genProgress)
                        /(otherActorNames.size()-1));
                }
            }

            swapOvl->onButtonPress(Aether::Button::X, [listBtns, list](){
                std::string out;
                if (R_SUCCEEDED(Keyboard::getStringInput(
                    &out, "Enter a search query..."))){
                    // lower the search query
                    toLower(out);

                    // Filter buttons
                    size_t posOffset=0;
                    for (const auto& [actorName, nameBtn] : *listBtns){
                        const auto& [name, btn] = nameBtn;

                        if (name.find(out) == std::string::npos){
                            btn->setHidden(true); // Hide buttons that don't match
                        } else {
                            Logger::Importantf("Showing: %s", name.c_str());
                            btn->setHidden(false); // Show buttons that do
                            btn->setY(list->y()+12+(posOffset*btn->h()));

                            if (posOffset == 0){
                                list->setFocused(btn);
                            }

                            posOffset+=1;
                        }
                    }
                }
            });

            swapOvl->onButtonPress(Aether::Button::B, [this, swapOvl](){
                Audio::playSound("cancel");

                this->ovl->setExitEnabled(true);
                this->window->removeOverlay(swapOvl);
                this->ovl->focusElement(this->ovlSwapBtn);
            });

            swapOvl->setFocused(list);
            list->setFocused(firstOptn);

            // Show the select overlay and close the loading overlay
            this->window->addOverlay(swapOvl);
            loadingMsg->close();
        }

        void Tile::onPress(){
            // Assign widget overlay
            if (this->ovl == nullptr){
                this->ovl = this->widget->getWidgetOverlay();
            }
            
            // Run saving/resetting methods on close
            this->ovl->onCloseSignal->connectWithId([this](size_t connId, bool isSave){
                this->onClose(isSave);
                this->ovl->onCloseSignal->disconnect(connId);
            });
            
            // Build the core overlay elements and open the overlay
            this->buildOvl();

            this->setOvlState(true);
            this->updateOvlText();
            this->updateOvlImg();
            this->updatePropertyEditors();

            this->window->addOverlay(this->ovl);
        }

        void Tile::onClose(bool isSave){
            if (isSave){
                // Update the tile type depending on its occupied state
                inventoryItem.type = (inventoryItem.actorName.empty()) ? TileType::Empty : TileType::Standard;

                // Update the item in the Pouch
                *originalInventoryItem = this->inventoryItem;

                // Save changes made to the pouch

                // Update my tile
                this->updateThumbnail();
                this->updateBuffStar();
                this->updateTileFormat(inventoryItem.type, 
                    inventoryItem.style, FormatValue(inventoryItem));
            }
            else {
                // Nothing was changed, no reload needed
                // Audio::playSound("cancel");
                this->inventoryItem = *originalInventoryItem;
            }

            this->setOvlState(false);
        }

        Tile::~Tile(){
            // Do whatever
        }

        void Tile::onReset(){
            // Audio::playSound("select");

            switch (inventoryItem.category){
                case Category::Weapons:
                case Category::Bows:
                case Category::Shields: {
                    // Reset durability
                    uint32_t maxDurability = GetMaxDurability(
                        inventoryItem.actorName)*kDurabilityMultiplier; // bruh
                    inventoryItem.value = maxDurability;

                    // Remove modifier
                    inventoryItem.modifier = Modifier::None;
                    inventoryItem.flagSp = 0;
                    inventoryItem.valueSp = 0;
                    break;
                }

                default:
                    break;
            }

            this->updatePropertyEditors();
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