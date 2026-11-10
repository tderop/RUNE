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

            Aether::MessageBox* _loadingMsg = nullptr;
            Aether::RoundProgressBar* _loadingProgress = nullptr;

            inline static Application* _app = nullptr;

            bool _fadeScreens;
            bool* _initSuccess;

            bool _isAlive = true;
            std::unordered_map<std::string, Aether::Screen*> screens;

        public:
            Application(bool* initSuccess, bool fadeScreens = false);

            /**
             * @brief Set the current application
             * @note This can only work once
             */
            static void setApplication(Application* application);

            /**
             * @brief Get the current application
             * @note This will return nullptr when called on an instance, can only be called on instantiator
             */
            static Application* getApplication();

            /**
             * @brief Get the current application window.
             */
            Aether::Window* window();

            /**
             * @brief Show a screen from its screen name.
             */
            Aether::Screen* showScreen(const std::string& screenName);

            /**
             * @brief Get a screen by its name.
             */
            Aether::Screen* getScreen(const std::string& screenName) const;

            /**
             * @brief Get the current screen name.
             */
            std::string currentScreenName();

            /**
             * @brief Show a loading message on the screen to indicate progress is being made
             * despite my abysmally optimised code
             * @note It's your job to close this overlay! BUT DON'T DELETE IT!!!!! (or at least null it afterwards if you do)
             */
            Aether::MessageBox* showLoading(const std::string& msg="Loading...", bool showProgress=false);

            /**
             * @brief Set the loading bar progress (0 to 1)
             * @param progress The new progress
             * @note Ensure progress bar is visible when calling showLoading()
             */
            void setLoadingProgress(float progress);

            /**
             * @brief Delete the window. Used when ending the program.
             */
            void deleteWindow();

            bool loop();

            // Determines whether RUNE is running in my development testing environment or not. (Will not work for you unless you put a file called 'emulator' in the root of your Ryujinx SD card)
            static inline bool IsEmulator = false;

            // Determines whether to provide extensive logs or only important ones
            static inline bool IsVerbose = false;

            static inline bool TestsRunning = false;
    };
}