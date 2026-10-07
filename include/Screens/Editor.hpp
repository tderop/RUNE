#pragma once

#include <switch.h>
#include <vector>
#include <unordered_map>

#include <Aether/Aether.hpp>
#include <Input/Input.hpp>
#include <Save/Save.hpp>
#include <WidgetBuilder.hpp>

/**
 * IMPORTANT KEYS
 * CurrentMamo (-830800173): 
 */
struct EditorTab{
    std::vector<std::string> menus;
    std::unordered_map<std::string, std::vector<std::string>> menuEntries;
};

struct TempElement{
    Aether::Element* parentElm;
    Aether::Element* elm;
};

using EditorTabData = std::unordered_map<std::string, EditorTab>;

class Editor : public Aether::Screen{
private:
    Aether::Window* window;
    RuneSaveFile saveFile;

    EditorTabData tabData;
    std::vector<std::string> tabs;
    Trove* trove;

    std::vector<EditorWidget*> tempWidgets;

    std::vector<Aether::ListButton*> tabButtons;
    Aether::Rectangle* topPanel;

    std::string selectedTab;
    int selectedMenu;

    std::vector<Aether::BorderButton*> menuLabels;
    Aether::Rectangle* menuLabelSelIndicator;

    Aether::PopupList* popupList;

public:
    // standard Aether class methods
    void onLoad() override;
    void onUnload() override;
    // void update(unsigned int deltaTime) override;

    Editor(Aether::Window* window, RuneSaveFile saveFile);
    ~Editor();

    void positionTabSelectIndicator(Aether::Rectangle* selectIndicator, Aether::ListButton* targetBtn);

    void buildScreen();
    void showTab(std::string tabName);

    void showMenu(int menuIdx);

    void nextMenu(InputSystem& inputService, InputState state);
    void prevMenu(InputSystem& inputService, InputState state);

    EditorWidget* getWidgetFromSaveKey(const std::string& saveKey, RuneSaveFile& saveFile);
    Aether::PopupList* getPopupList();

    void cleanEditorWidgets();
    
    void addScreenElements();
    void showSaveSelect();
};