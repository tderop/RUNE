#include <Aether/Aether.hpp>
#include <Aether/utils/Utils.hpp>

#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>
#include <Editor/EditorWidget.hpp>

#include <Utils/Utils.hpp>
#include <Utils/Timer.hpp>
#include <Utils/Logger.hpp>
#include <Save/Save.hpp>
#include <Json/Json.hpp>
#include <Config.hpp>
#include <Input/Input.hpp>

#include <Editor/Menus/Inventory.hpp>
#include <Application/Application.hpp>
#include <Application/Audio.hpp>
#include <Editor/Menus/Inventory/Pouch.hpp>

#include <string>
#include <vector>

#define OPTIONS_BUTTON Aether::Button::NO_BUTTON
#define OPTIONS_FRAME_W 320
#define OPTIONS_FRAME_H 360

#define PREV_PAGE Aether::Button::RSTICK_LEFT
#define NEXT_PAGE Aether::Button::RSTICK_RIGHT

#define PAGE_PROMPT_SPACING 24

void Rune::MenuBuilder::Inventory(EditorMenu* menu){
    // Disable right stick navigation
    Aether::SetRightStickNavigation(false);

    // Make a new pouch (parse the inventory save data) if one hasn't been made yet
    if (Inventory::CurrentPouch == nullptr){
        Inventory::CurrentPouch = new Inventory::Pouch(menu->saveFile());
    }

    // Get the items for this category
    Inventory::Category category = Inventory::GetCategoryFromMenuName(menu->menuName());

    // Default to hardcoded max
    size_t maxSlots = GetMaxSlotsFromCategory(category);
    
    // Check max unlocked slots in save data
    auto maxIt = Inventory::kSlotKeys.find(category);
    if (maxIt != Inventory::kSlotKeys.end()){
        maxSlots = static_cast<size_t>(std::get<int>(Save
            ::GetEntryFromDataName(*menu->saveFile(), 
            maxIt->second)->value));
    }
    Inventory::PouchItems* inventoryItems = new Inventory::PouchItems(
        Inventory::CurrentPouch->getItemsByCategory(
            category, maxSlots));

    // Add arrows if this is a bows page
    if (category == Inventory::Category::Bows){
        Inventory::PouchItems arrows = Inventory::CurrentPouch->getItemsByCategory(Inventory::Category::Arrows, 
            Inventory::kMaxArrowSlots);
        inventoryItems->insert(inventoryItems->end(), arrows.begin(), arrows.end());
    }

    // Calculate max pages
    int maxPage = static_cast<float>(inventoryItems->size()-1) / static_cast<float>(Rune::Inventory::kTilesPerPage);
    auto currentPage = std::make_shared<int>(0);

    // The dots showing which page is currently being viewed
    auto ellipses = std::make_shared<std::vector<Aether::Ellipse*>>();

    // Do not show navigation dot when there is only one page
    if (maxPage > 0){
        for (int i = 0; i <= maxPage; ++i){
            // Create a new circle
            Aether::Ellipse* menuDot = new Aether::Ellipse(
                0,
                menu->body()->y()+Rune::Inventory::kYStartOffset - 24, 
                Rune::Inventory::kDotWidth
            );

            // Disable it by default (will be enabled if applicable by passing ellipse vector into Rune::Inventory::ShowPage)
            menuDot->setColour(Rune::Inventory::kDisabledEllipse);
            
            // The body owns the dot
            menu->body()->addElement(menuDot);

            // Store the new circle
            ellipses->push_back(menuDot);
        }
    }

    // Calculate values for dot repositioning
    float centerPosition = (menu->body()->x() + Rune::Inventory::kXStartOffset + (Rune::Inventory::kGridWidth/2.0f));
    int dotStride = Rune::Inventory::kDotWidth + Rune::Inventory::kDotPadding;
    int dotStart = centerPosition - (((ellipses->size())-1)*(dotStride))/2.0f;

    // Reposition menu dots
    Aether::Ellipse* lastDot = nullptr;
    for (int i = 0; i < static_cast<int>(ellipses->size()); ++i){
        Aether::Ellipse* menuDot = ellipses->at(i);
        menuDot->setX(dotStart + (i*dotStride));

        lastDot = menuDot;
    }

    // Show the prompt to change page if multiple pages are in this menu
    if (lastDot != nullptr){
        Aether::Text* pagePrompt = new Aether::Text(
            0, 0,
            Aether::Utils::buttonToCharacter(Aether::Button::RSTICK),
            20
        );

        pagePrompt->setColour(MUTED_TEXT_COLOR);
        pagePrompt->setX(lastDot->right()+PAGE_PROMPT_SPACING);
        pagePrompt->centreAtY(lastDot->centreY());
        menu->body()->addElement(pagePrompt);
    }

    // Options overlay
    menu->controlBar()->addControl(OPTIONS_BUTTON, "Options");
    Aether::Overlay* optnOvl = new Aether::Overlay();

    // Aether::Rectangle* bgFrame = new Aether::Rectangle(
    //     0, 0, WINDOW_RESOLUTION_X, WINDOW_RESOLUTION_Y, 0
    // );
    Aether::Rectangle* optnFrame = new Aether::Rectangle(
        0, 0, 320, 360, 6);
    Aether::Text* optnText = new Aether::Text(0, 0, 
        menu->menuName()+" - Options", 32);

    // bgFrame->setColour(Aether::Colour{0, 0, 0, });
    optnFrame->setColour(BACKGROUND_COLOUR);    
    optnText->setColour(TEXT_COLOUR);

    optnFrame->centreAtXY(WINDOW_RESOLUTION_X/2, WINDOW_RESOLUTION_Y/2);
    optnText->centreAtXY(optnFrame->centreX(),
        optnFrame->y()-56);

    // optnOvl->addElement(bgFrame);
    optnOvl->addElement(optnFrame);
    optnOvl->addElement(optnText);
    
    menu->screen()->onButtonPress(OPTIONS_BUTTON, [optnOvl, optnFrame](){
        Application* app = Rune::Application::getApplication();
        app->window()->addOverlay(optnOvl);
        
        // Reset position before size tween
        optnFrame->setWH(1, 1);
        optnFrame->centreAtXY(WINDOW_RESOLUTION_X/2, WINDOW_RESOLUTION_Y/2);

        // Tween in
        Aether::Tween sizeTween(optnFrame, .33, Aether::EasingStyle::Cubic, 
            Aether::EasingDirection::Out, {
                .Size = {{320, 360}}
        });
        sizeTween.play();
    });

    optnOvl->onButtonPress(OPTIONS_BUTTON, [](){}); // stub
    optnOvl->onButtonPress(Aether::Button::B, [optnOvl, optnFrame](){
        Aether::Tween t(optnFrame, .33,Aether::EasingStyle::Cubic, 
            Aether::EasingDirection::Out, {
                .Size = {{1, 1}}
        });
        t.play();

        optnOvl->close();
    });

    // Show the first page
    Timer::start();
    Rune::Inventory::ShowPage(menu, inventoryItems, *currentPage, *ellipses);
    Timer::end();

    // Go to the next page when ZR is pressed
    menu->screen()->onButtonPress(NEXT_PAGE, [menu, inventoryItems, currentPage, maxPage, ellipses](){
        if (maxPage == 0){
            return;
        }
        Audio::playSound("page_slide");

        // Change current page, wrap around if at max page
        if (*currentPage >= maxPage){
            *currentPage = 0;
        } else {
            *currentPage += 1;
        }

        Rune::Inventory::ShowPage(menu, inventoryItems, static_cast<size_t>(*currentPage), *ellipses);
    });

    // Go to the previous page when ZL is pressed
    menu->screen()->onButtonPress(PREV_PAGE, [menu, inventoryItems, currentPage, maxPage, ellipses](){
        if (maxPage == 0){
            return;
        }
        Audio::playSound("page_slide");

        if (*currentPage <= 0){
            *currentPage = maxPage;
        } else {
            *currentPage -= 1;
        }

        Rune::Inventory::ShowPage(menu, inventoryItems, static_cast<size_t>(*currentPage), *ellipses);
    });

    // This method is run when the menu is cleared
    menu->giveTask([menu, ellipses, optnOvl, inventoryItems](){
        // Delete heap allocations
        delete inventoryItems;

        // Enable right stick navigation
        // Aether::SetRightStickNavigation(true);

        // remove the options control
        menu->controlBar()->removeControl(OPTIONS_BUTTON);
        menu->window()->removeOverlay(optnOvl);

        // Reset the blank pouch tiles
        if (Inventory::CurrentPouch != nullptr){
            Inventory::CurrentPouch->resetBlankSlots();
        }

        // Re-enable right stick navigation
        Aether::SetRightStickNavigation(true);
    });

    return;
}