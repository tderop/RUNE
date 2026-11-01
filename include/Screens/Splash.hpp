#pragma once

#include <Aether/Aether.hpp>
#include <Events/Signal.hpp>
#include <Application/Application.hpp>

#include <atomic>

class SplashScreen : public Aether::Screen{
    private:
        Rune::Application* app = nullptr;
        Aether::Image* _splashImg = nullptr;

        bool _alive = true;
        int _elapsed = 0;

        bool _threadStarted = false;
        std::atomic_bool _loaded;

        Thread _loadingThread{};
        
        std::mutex _statusMutex;
        std::string _status;

        std::string _previousStatus;

        Aether::Text* _statusText = nullptr;

        std::atomic<size_t> _progress{0};

    public:
        Signal<>* loadCompleteSignal = new Signal<>();

        SplashScreen(Rune::Application* app);

        void buildScreen();

        static void loadRune(void* _self);

        void onLoad() override;
        void onUnload() override;
        void update(unsigned int dt) override;
};