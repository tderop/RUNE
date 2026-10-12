#include <Application/Application.hpp>

#include <unordered_map>
#include <Aether/Aether.hpp>

#include <Screens/Menu.hpp>
#include <Screens/Editor.hpp>
#include <Screens/Splash.hpp>

#include <Utils/Utils.hpp>
#include <Config.hpp>

/**
 * The current issue is that compilation fails due to linker errors.
 * All to do with Rune::Application. Most likely a syntax misunderstanding.
 * 
 * FIXED ^^ (makefile issue ;-;)
 */

/**
 * @brief A simple application manager designed to properly manage screens and their lifetimes and memory.
 */
namespace Rune{
    Application::Application(){
        // initialise the window
        this->_window = new Aether::Window(
            "RUNE", 
            WINDOW_RESOLUTION_X, WINDOW_RESOLUTION_Y, 

            [](const std::string message, const bool important){
            if (important){
                log("[!! AETHER !!] - "+message);
            } else {
                log("[AETHER] - "+message);
            }
        });

        this->_window->setHighlightAnimation(highlightFunc());
        log("[APPLICATION] - Successfully loaded Aether window!");

        this->_window->setFadeIn(true);
        this->_window->setFadeOut(true);


        // create the screens
        log("[Rune::Application] - Initialising screens.");

        SplashScreen* splashScreen = new SplashScreen(this);
        MenuScreen* menuScreen = new MenuScreen(this);
        EditorScreen* editorScreen = new EditorScreen(this);

        log("[Rune::Application] - Screens initialised.");

        // build the basic screen elements
        splashScreen->buildScreen();
        log("[Rune::Application] - Built SplashScreen.");

        menuScreen->buildScreen();
        log("[Rune::Application] - Built MenuScreen.");

        editorScreen->buildScreen();
        log("[Rune::Application] - Built EditorScreen.");

        this->screens = {
            {
                "Splash",
                splashScreen
            },
            {
                "Menu",
                menuScreen
            },
            {
                "Editor",
                editorScreen
            }
        };

        log("[Rune::Application] - Screen setup complete.");

        // show the initial splash screen
        Connection<> loadCompleteConn = splashScreen->loadCompleteSignal->connect([this, &loadCompleteConn](){
            loadCompleteConn.disconnect(); // remove this connection
            // transition to menu screen
            this->showScreen("Menu");
        });

        this->showScreen("Splash");
        log("[Rune::Application] - Successfully showed splash screen!");
    }

    Aether::Window* Application::window(){
        return this->_window;
    }

    void Application::deleteWindow(){
        delete this->_window;
    }

    Aether::Screen* Application::showScreen(const std::string& screenName){
        Aether::Screen* screen = this->screens.at(screenName);
    
        if (screen != nullptr){
            log("[Rune::Application] - Showing screen '"+screenName+"'...");
            this->_window->showScreen(screen);

            // unload previous screen (?)
            this->_currentScreen = screen;
        } else {
            log("[Rune::Application] - Screen is nullptr. Ignoring.");
        }

        return screen;
    }
}