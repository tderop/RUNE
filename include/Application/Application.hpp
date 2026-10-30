#pragma once

#include <unordered_map>
#include <Aether/Aether.hpp>

namespace Rune{
    class Application{
        private:
            Aether::Window* _window;
            Aether::Overlay* _fadeOverlay;

            Aether::Screen* _currentScreen = nullptr;
            std::string _currentScreenName = "";

            bool _fadeScreens;
            bool* _initSuccess;
            std::unordered_map<std::string, Aether::Screen*> screens;

        public:
            Application(bool* initSuccess, bool fadeScreens = false);

            /**
             * @brief Get the current application window.
             */
            Aether::Window* window();

            /**
             * @brief Show a screen from its screen name.
             */
            Aether::Screen* showScreen(const std::string& screenName);

            /**
             * @brief Get the current screen name.
             */
            std::string currentScreenName();

            /**
             * @brief Delete the window. Used when ending the program.
             */
            void deleteWindow();
    };
}