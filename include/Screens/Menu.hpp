#pragma once

#include <switch.h>
#include <vector>

#include <Aether/Aether.hpp>
#include <Utils/Trove.hpp>
#include <Application/Application.hpp>

class MenuScreen : public Aether::Screen{
private:
    Rune::Application* app;
    Aether::List* saveSlotScroll = nullptr;
    Trove* trove;
    // std::vector<Aether::Element*> myElements;

public:
    // standard Aether class methods
    void onLoad() override;
    void onUnload() override;
    // void update(unsigned int deltaTime) override;

    MenuScreen(Rune::Application* app);
    ~MenuScreen();

    void buildScreen();
    void showSaveSelect();

    void addTempElement(Aether::Element* element);
};