#pragma once

#include <switch.h>
#include <vector>

#include <Aether/Aether.hpp>
#include <Input/Input.hpp>
#include <Save/Save.hpp>

struct EditorTab{
    std::vector<std::string> menus;
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
    std::vector<TempElement> temp;

    std::vector<Aether::ListButton*> tabButtons;
    Aether::Rectangle* topPanel;

    std::string selectedTab;
    int selectedMenu;

    std::vector<Aether::BorderButton*> menuLabels;
    Aether::Rectangle* menuLabelSelIndicator;

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

    void cleanTemp();
    
    void addScreenElements();
    void showSaveSelect();
};