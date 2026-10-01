#pragma once

#include <switch.h>
#include <vector>

#include <Aether/Aether.hpp>

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

    EditorTabData tabData;
    std::vector<std::string> tabs;
    std::vector<TempElement> temp;

    std::vector<Aether::ListButton*> tabButtons;

public:
    // standard Aether class methods
    void onLoad() override;
    void onUnload() override;
    // void update(unsigned int deltaTime) override;

    Editor(Aether::Window* window);
    ~Editor();

    void positionSelectIndicator(Aether::Rectangle* selectIndicator, Aether::ListButton* targetBtn);

    void buildScreen();
    void showTab();

    void cleanTemp();
    
    void addScreenElements();
    void showSaveSelect();
};