#pragma once

#include <switch.h>
#include <vector>

#include <Aether/Aether.hpp>

class Editor : public Aether::Screen{
private:
    Aether::Window* window;

public:
    // standard Aether class methods
    void onLoad() override;
    void onUnload() override;
    // void update(unsigned int deltaTime) override;

    Editor(Aether::Window* window);
    ~Editor();

    void buildScreen();
    
    void addScreenElements();
    void showSaveSelect();
};