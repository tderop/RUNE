#pragma once

#include <Aether/Aether.hpp>
#include <Events/Signal.hpp>
#include <Application/Application.hpp>

class SplashScreen : public Aether::Screen{
    private:
        Rune::Application* app;
        Aether::Image* _splashImg = nullptr;

        bool _alive = true;
        int _elapsed = 0;

    public:
        Signal<>* loadCompleteSignal = new Signal<>();

        SplashScreen(Rune::Application* app);

        void buildScreen();

        void onLoad() override;
        void onUnload() override;
        void update(unsigned int dt) override;
};
