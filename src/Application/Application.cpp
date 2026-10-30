#include <Application/Application.hpp>
#include <Application/Tests.hpp>

#include <Input/Input.hpp>

#include <unordered_map>
#include <Aether/Aether.hpp>

#include <Screens/Menu.hpp>
#include <Screens/Editor.hpp>
#include <Screens/Splash.hpp>

#include <Utils/Utils.hpp>
#include <Config.hpp>

#include <math.h>

/**
 * The current issue is that compilation fails due to linker errors.
 * All to do with Rune::Application. Most likely a syntax misunderstanding.
 * 
 * FIXED ^^ (makefile issue ;-;)
 */

const useconds_t TWEEN_UPDATE_TIME = 16000; // microseconds

/**
 * @brief A simple application manager designed to properly manage screens and their lifetimes and memory.
 */
namespace Rune{
    
    Application::Application(bool* initSuccess, bool fadeScreens){
        this->_initSuccess = initSuccess;
        this->_fadeScreens = fadeScreens;

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

        InputSystem& inputService = InputSystem::getService();

        inputService.bind("StartTests", InputObject(InputType::Gamepad, HidNpadButton_L));
        Connection<InputSystem&, InputObject> beginTestsConn = inputService.inputBegan.connectWithId([this](size_t id, InputSystem& svc, InputObject inputObj){
            if (inputObj.keyCode == HidNpadButton_L){
                svc.inputBegan.disconnect(id);
                if (this->currentScreenName() != "Splash"){
                    return;
                }

                Rune::Tests::Run();
                this->window()->exit();
            }
        });

        // show the initial splash screen
        Connection<> loadCompleteConn = splashScreen->loadCompleteSignal->connectWithId([this, &beginTestsConn, splashScreen](size_t id){
            beginTestsConn.disconnect();
            splashScreen->loadCompleteSignal->disconnect(id);

            if (*this->_initSuccess){
                // transition to menu screen
                this->showScreen("Menu");
            } else { // failed to launch, notify user
                Aether::MessageBox* messageBox = new Aether::MessageBox(
                    "RUNE failed to launch.\n\nPlease make sure you have save data for The Legend of Zelda: "
                    "Breath of the Wild on this account."
                );

                messageBox->addTopButton("Exit", [this](){
                    this->_window->exit();
                });

                messageBox->setRectangleColour(BACKGROUND_COLOUR);
                messageBox->setLineColour(Aether::Colour{0, 0, 0, 0});

                this->_window->addOverlay(messageBox);
            }
        });

        this->showScreen("Splash");
        log("[Rune::Application] - Successfully showed splash screen!");
    }

    Aether::Window* Application::window(){
        return this->_window;
    }

    void Application::deleteWindow(){
        // this->_window->exit();
        delete this->_window;
    }

    Aether::Screen* Application::showScreen(const std::string& screenName){
        Aether::Screen* screen = this->screens.at(screenName);
    
        if (screen != nullptr){
            log("[Rune::Application] - Showing screen '"+screenName+"'...");
            
            // full opacity initially
            float rectOpacity = 0.0f;
            int roundedOpacity;

            // fade out the screen
            Aether::Rectangle* fadeOutRect = nullptr;
            if (this->_currentScreen != nullptr && this->_fadeScreens){ // only fade out if there is a current screen
                fadeOutRect = new Aether::Rectangle(0, 0, WINDOW_RESOLUTION_X, WINDOW_RESOLUTION_Y, 0);
                fadeOutRect->setColour(Aether::Colour{0, 0, 0, 0});

                log("[Rune::Application] - Current screen is not null. Fading out screen.");
                this->_currentScreen->addElement(fadeOutRect); // add rect to current screen

                // fade IN the rectangle
                do {
                    roundedOpacity = rectOpacity;

                    fadeOutRect->setColour(Aether::Colour{0, 0, 0, static_cast<char>(std::min(roundedOpacity, 255))});
                    rectOpacity += 150.0f*(static_cast<float>(TWEEN_UPDATE_TIME)/1000000);
                    log(std::to_string(roundedOpacity));

                    usleep(TWEEN_UPDATE_TIME);
                } while (rectOpacity < 255.0f);

                log("[Rune::Application] - Fade out complete.");
            }

            // store old screen
            Aether::Screen* oldScreen = this->_currentScreen;

            // add rect to new screen
            Aether::Rectangle* fadeInRect = nullptr;

            if (this->_fadeScreens){
                log("[Rune::Application] - Adding fade rect to new screen...");
                fadeInRect = new Aether::Rectangle(0, 0, WINDOW_RESOLUTION_X, WINDOW_RESOLUTION_Y, 0);
                fadeInRect->setColour(Aether::Colour{0, 0, 0, 255});
                screen->addElement(fadeInRect);
            }

            // show new screen
            log("[Rune::Application] - Showing new screen.");
            this->_window->showScreen(screen);
            this->_currentScreen = screen;
            this->_currentScreenName = screenName;

            if (this->_fadeScreens){
                // remove rect from old screen
                if (fadeOutRect != nullptr && oldScreen != nullptr){
                    oldScreen->removeElement(fadeOutRect);
                }

                // fade in new screen
                log("[Rune::Application] - Fading in new screen.");
                do {
                    roundedOpacity = rectOpacity;

                    fadeInRect->setColour(Aether::Colour{0, 0, 0, static_cast<char>(std::max(roundedOpacity, 0))});
                    rectOpacity -= 150.0f*(static_cast<float>(TWEEN_UPDATE_TIME)/1000000);
                    log(std::to_string(roundedOpacity));

                    usleep(TWEEN_UPDATE_TIME);
                } while (rectOpacity > 0.0f);
                log("[Rune::Application] - Fade in complete.");

                // remove rect from new screen
                log("[Rune::Application] - Removing rect from new screen...");
                this->_currentScreen->removeElement(fadeInRect);
            }

        } else {
            log("[Rune::Application] - Screen is nullptr. Ignoring.");
        }

        return screen;
    }

    std::string Application::currentScreenName(){
        return this->_currentScreenName;
    }
}