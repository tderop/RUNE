#pragma once

#include <unordered_map>
#include <Aether/Aether.hpp>

namespace Rune{
    class Application{
        private:
            Aether::Window* _window;
            Aether::Screen* _currentScreen;
            std::unordered_map<std::string, Aether::Screen*> screens;

        public:
            Application();

            /**
             * @brief Get the current application window.
             */
            Aether::Window* window();

            /**
             * @brief Show a screen from its screen name.
             */
            Aether::Screen* showScreen(const std::string& screenName);

            /**
             * @brief Delete the window. Used when ending the program.
             */
            void deleteWindow();
    };
}