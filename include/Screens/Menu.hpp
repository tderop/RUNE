#pragma once

#include <switch.h>
#include <vector>

#include <Aether/Aether.hpp>

class Menu : public Aether::Screen{
private:
    Aether::Window* window;
    Aether::List* saveSlotScroll = nullptr;
    // std::vector<Aether::Element*> myElements;

public:
    // standard Aether class methods
    void onLoad() override;
    void onUnload() override;
    // void update(unsigned int deltaTime) override;

    Menu(Aether::Window* window);
    ~Menu();

    void addScreenElements();
    void showSaveSelect();
};