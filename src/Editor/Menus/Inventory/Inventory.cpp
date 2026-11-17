#include <Aether/Aether.hpp>

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
#include <Editor/Menus/Inventory/Pouch.hpp>

#include <string>
#include <vector>

#define OPTIONS_BUTTON Aether::Button::X
#define OPTIONS_FRAME_W 320
#define OPTIONS_FRAME_H 360

void Rune::MenuBuilder::Inventory(EditorMenu* menu){
    // Make a new pouch (parse the inventory save data) if one hasn't been made yet
    if (Inventory::CurrentPouch == nullptr){
        Logger::Importantf("[INVENTORY] - Creating new pouch.");
        Inventory::CurrentPouch = new Inventory::Pouch(menu->saveFile());
    }

    // Get the items for this category
    Logger::Importantf("[INVENTORY] - Getting items in this category.");
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
    for (int i = 0; i < static_cast<int>(ellipses->size()); ++i){
        Aether::Ellipse* menuDot = ellipses->at(i);
        menuDot->setX(dotStart + (i*dotStride));
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

    // Logger::Importantf("Time taken to generate initial %s page: %.2fms", menu->menuName().c_str(), Timer::get()/1E+6);

    // Bind navigation actions
    InputSystem& inputService = InputSystem::getService();
    inputService.bind("NextPage", InputObject(InputType::Gamepad, HidNpadButton_ZR));
    inputService.bind("PrevPage", InputObject(InputType::Gamepad, HidNpadButton_ZL));

    // Go to the next page when ZR is pressed
    inputService.connect("NextPage", [menu, inventoryItems, currentPage, maxPage, ellipses](InputSystem& svc, InputState state){
        if (state != InputState::Begin || maxPage == 0){
            return;
        }

        // Change current page, wrap around if at max page
        if (*currentPage >= maxPage){
            *currentPage = 0;
        } else {
            *currentPage += 1;
        }

        Rune::Inventory::ShowPage(menu, inventoryItems, static_cast<size_t>(*currentPage), *ellipses);
    });

    // Go to the previous page when ZL is pressed
    inputService.connect("PrevPage", [menu, inventoryItems, currentPage, maxPage, ellipses](InputSystem& svc, InputState state){
        if (state != InputState::Begin || maxPage == 0){
            return;
        }

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

        // Unbind pagination actions
        InputSystem& inputService = InputSystem::getService();
        inputService.unbind("NextPage");
        inputService.unbind("PrevPage");

        // Enable right stick navigation
        // Aether::SetRightStickNavigation(true);

        // remove the options control
        menu->controlBar()->removeControl(OPTIONS_BUTTON);
        menu->window()->removeOverlay(optnOvl);

        // Reset the blank pouch tiles
        if (Inventory::CurrentPouch != nullptr){
            Inventory::CurrentPouch->resetBlankSlots();
        }
    });

    return;
}