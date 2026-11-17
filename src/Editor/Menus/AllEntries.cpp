#include <Aether/utils/Utils.hpp>
#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>

#include <Save/Save.hpp>
#include <Utils/Logger.hpp>
#include <Utils/Utils.hpp>
#include <Input/Input.hpp>
#include <Application/Keyboard.hpp>
#include <Application/Audio.hpp>

#include <string>
#include <memory>

#define NEXT_PAGE Aether::Button::ZR
#define PREV_PAGE Aether::Button::ZL

const int ENTRIES_PER_PAGE = 50;
const Aether::Button SEARCH_BUTTON = Aether::Button::X;

using idx_filter = std::shared_ptr<std::vector<size_t>>;

void showPage(EditorMenu* menu, Aether::Text* pageCount, size_t page, idx_filter filter){
    size_t totalEntries = 0;
    std::vector<Rune::Save::Entry>& entries = menu->saveFile()->entries;
    
    // Clear all existing entries
    menu->clearAllWidgets();

    // The vector cursors
    // Represent positions in all entry lists if no filter, otherwise the positions in the filter
    size_t cursorStart = page*ENTRIES_PER_PAGE;
    size_t cursorEnd = cursorStart+ENTRIES_PER_PAGE;

    for (size_t cursor = cursorStart; cursor < cursorEnd; ++cursor){
        size_t idx = cursor;
        if (filter != nullptr && !filter->empty()){ // Use the next index listed in the filter if one was provided
            if (cursor >= filter->size()){ break; }
            idx = filter->at(cursor);
        }

        if (idx >= entries.size()){ break; }
        Rune::MenuBuilder::AddWidgetFromSaveKey(menu, entries.at(idx)
            .dataName, true);
    }

    // Show relative page counts depending on whether a search filter has been applied or not
    if (filter != nullptr && !filter->empty()){
        pageCount->setString(std::to_string(page+1)+"/"+std::to_string((filter->size()/ENTRIES_PER_PAGE)+1));
    }
    else {
        pageCount->setString(std::to_string(page+1)+"/"+std::to_string((entries.size()/ENTRIES_PER_PAGE)+1));
    }
}

void Rune::MenuBuilder::AllEntries(EditorMenu* menu){
    Rune::Save::File* saveFile = menu->saveFile();
    
    std::shared_ptr currentPage = std::make_shared<size_t>(0);
    std::shared_ptr maxPage = std::make_shared<size_t>(saveFile->entries.size()/ENTRIES_PER_PAGE);

    int originalY = menu->list()->y();
    int originalH = menu->list()->h();

    int toShrink = originalH*0.1f;
    int newH = originalH-toShrink;

    menu->list()->setH(newH);
    menu->list()->setY(menu->list()->y()+toShrink);

    // ZL/ZR prompt
    Aether::Text* pagePrompt = new Aether::Text(0, 0, 
        Aether::Utils::buttonToCharacter(PREV_PAGE)+"  "+Aether::Utils::buttonToCharacter(NEXT_PAGE),
        18);
    pagePrompt->setColour(MUTED_TEXT_COLOR);
    pagePrompt->setXY(menu->body()->x()+18, menu->body()->y()+pagePrompt->textureHeight()+6);

    // Text element showing the current page and the max page
    Aether::Text* pageCount = new Aether::Text(0, 0, 
        "0/0", 18);
    pageCount->setColour(MUTED_TEXT_COLOR);
    pageCount->setX(pagePrompt->right()+24);
    pageCount->centreAtY(pagePrompt->centreY());

    // A list of indexes that represent which save entry names matched a search query if one has been used
    idx_filter filter = std::make_shared<std::vector<size_t>>();

    // Add a control to the screen showing that the user can search
    menu->controlBar()->addControl(SEARCH_BUTTON, "Search");
    menu->screen()->onButtonPress(SEARCH_BUTTON, [menu, saveFile, filter, pageCount, currentPage, maxPage](){
        filter->clear();
        *maxPage = saveFile->entries.size()/ENTRIES_PER_PAGE;

        // Get a search query from the user
        std::string out;
        if (R_SUCCEEDED(Rune::Keyboard::getStringInput(&out, "Enter a search query... nothing will change if no results are found!"))){
            if (out.empty()){ return; }
            
            // Search for matching dataName entries
            toLower(out);
            for (size_t i = 0; i < saveFile->entries.size(); ++i){
                const auto& entry = saveFile->entries[i];
                std::string dataName = entry.dataName; toLower(dataName);
                if (dataName.find(out) != std::string::npos){
                    filter->push_back(i);
                }
            }

            // Update page values and UI
            *currentPage = 0;
            *maxPage = filter->size()/ENTRIES_PER_PAGE;
            showPage(menu, pageCount, *currentPage, filter);
        }
    });

    InputSystem& inputService = InputSystem::getService();
    inputService.bind("NextPage", InputObject(InputType::Gamepad, HidNpadButton_ZR));
    inputService.bind("PrevPage", InputObject(InputType::Gamepad, HidNpadButton_ZL));

    auto viewingEntries = std::make_shared<std::vector<size_t>>();
    inputService.connect("NextPage", [menu, currentPage, pageCount, maxPage, filter](InputSystem& svc, InputState state){
        if (state != InputState::Begin || *maxPage == 0){
            return;
        }

        Audio::playSound("page_slide");
        size_t page = *currentPage + 1;
        if (page > *maxPage){
            page = 0;
        }

        *currentPage = page;
        showPage(menu, pageCount, *currentPage, filter);
    });

    inputService.connect("PrevPage", [menu, currentPage, pageCount, maxPage, filter](InputSystem& svc, InputState state){
        if (state != InputState::Begin || *maxPage == 0){
            return;
        }

        Audio::playSound("page_slide");
        size_t page = *currentPage;
        if (page <= 0){
            page = *maxPage-1;
        } else {
            page -= 1;
        }

        *currentPage = page;
        showPage(menu, pageCount, *currentPage, filter);
    });

    menu->giveTask([menu, currentPage, originalY, originalH](){
        InputSystem& inputService = InputSystem::getService();
        inputService.unbind("NextPage");
        inputService.unbind("PrevPage");

        menu->list()->setY(originalY);
        menu->list()->setH(originalH);

        // Remove and stub out search control
        menu->controlBar()->removeControl(SEARCH_BUTTON);
        menu->screen()->onButtonPress(SEARCH_BUTTON, [](){});
    });

    showPage(menu, pageCount, 0, filter);

    menu->addToTrove(pageCount);
    menu->body()->addElement(pageCount);

    menu->addToTrove(pagePrompt);
    menu->body()->addElement(pagePrompt);
}