#include <Editor/EditorMenu.hpp>
#include <Editor/MenuBuilder.hpp>

#include <Save/Save.hpp>
#include <Utils/Logger.hpp>
#include <Utils/Utils.hpp>

#include <Input/Input.hpp>

#include <string>

const int ENTRIES_PER_PAGE = 50;

void showPage(EditorMenu* menu, Aether::Text* pageCount, size_t page){
    size_t totalEntries = 0;
    
    size_t cursorStart = page*ENTRIES_PER_PAGE;
    size_t cursorEnd = cursorStart+ENTRIES_PER_PAGE;

    menu->clearAllWidgets();

    std::vector<Rune::Save::Entry>& entries = menu->saveFile()->entries;
    pageCount->setString(std::to_string(page+1)+"/"+std::to_string((entries.size()/ENTRIES_PER_PAGE)+1));
    pageCount->setX(menu->body()->centreX()/2-pageCount->textureWidth()/2-12);

    for (size_t cursor = cursorStart; cursor < cursorEnd; ++cursor){
        if (cursor >= entries.size()){
            break;
        }

        Rune::Save::Entry& entry = entries.at(cursor);
        Rune::MenuBuilder::AddWidgetFromSaveKey(menu, entry.dataName, true);
    }

    // Logger::Importantf("Time taken to generate page: %dms", t/1000);
}

void Rune::MenuBuilder::AllEntries(EditorMenu* menu){
    size_t* currentPage = new size_t();
    size_t maxPage = menu->saveFile()->entries.size()/ENTRIES_PER_PAGE;

    int originalY = menu->list()->y();
    int originalH = menu->list()->h();

    int toShrink = originalH*0.1f;
    int newH = originalH-toShrink;

    menu->list()->setH(newH);
    menu->list()->setY(menu->list()->y()+toShrink);

    Aether::Text* pageCount = new Aether::Text(
        0, 0,
        "0/0", 18
    );
    pageCount->setColour(MUTED_TEXT_COLOR);
    pageCount->setXY(
        menu->body()->x()+menu->body()->w()/2-pageCount->textureWidth()/2-12,
        menu->body()->y()+pageCount->textureHeight()+6
    );

    InputSystem& inputService = InputSystem::getService();
    inputService.bind("NextPage", InputObject(InputType::Gamepad, HidNpadButton_ZR));
    inputService.bind("PrevPage", InputObject(InputType::Gamepad, HidNpadButton_ZL));

    inputService.connect("NextPage", [menu, currentPage, pageCount, maxPage](InputSystem& svc, InputState state){
        if (state != InputState::Begin){
            return;
        }

        size_t page = *currentPage + 1;
        if (page > maxPage){
            page = 0;
        }

        *currentPage = page;
        showPage(menu, pageCount, *currentPage);
    });

    inputService.connect("PrevPage", [menu, currentPage, pageCount, maxPage](InputSystem& svc, InputState state){
        if (state != InputState::Begin){
            return;
        }

        size_t page = *currentPage;
        if (page <= 0){
            page = maxPage;
        } else {
            page -= 1;
        }

        *currentPage = page;
        showPage(menu, pageCount, *currentPage);
    });

    menu->giveTask([menu, currentPage, originalY, originalH](){
        InputSystem& inputService = InputSystem::getService();
        inputService.unbind("NextPage");
        inputService.unbind("PrevPage");

        menu->list()->setY(originalY);
        menu->list()->setH(originalH);

        delete currentPage;
    });

    showPage(menu, pageCount, 0);

    menu->addToTrove(pageCount);
    menu->body()->addElement(pageCount);
}