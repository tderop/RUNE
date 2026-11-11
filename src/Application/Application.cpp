#include <Application/Application.hpp>
#include <Application/Tests.hpp>

#include <Input/Input.hpp>

#include <unordered_map>
#include <Aether/Aether.hpp>

#include <Screens/Menu.hpp>
#include <Screens/Editor.hpp>
#include <Screens/Splash.hpp>

#include <Utils/Logger.hpp>
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
                Logger::Write("[!! AETHER !!] - "+message);
            } else {
                Logger::Write("[AETHER] - "+message);
            }
        });

        // Set font spacing (why)
        this->_window->setFontSpacing(1.25);

        this->_window->setHighlightAnimation(highlightFunc());
        Logger::Write("[APPLICATION] - Successfully loaded Aether window!");

        this->_window->setFadeIn(true);
        this->_window->setFadeOut(true);

        // create the screens
        Logger::Write("[Rune::Application] - Initialising screens.");

        SplashScreen* splashScreen = new SplashScreen(this);
        MenuScreen* menuScreen = new MenuScreen(this);
        EditorScreen* editorScreen = new EditorScreen(this);

        Logger::Write("[Rune::Application] - Screens initialised.");

        // build the basic screen elements
        splashScreen->buildScreen();
        Logger::Write("[Rune::Application] - Built SplashScreen.");

        menuScreen->buildScreen();
        Logger::Write("[Rune::Application] - Built MenuScreen.");

        editorScreen->buildScreen();
        Logger::Write("[Rune::Application] - Built EditorScreen.");

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

        Logger::Write("[Rune::Application] - Screen setup complete.");
        InputSystem& inputService = InputSystem::getService();

        // show the initial splash screen
        Connection<> loadCompleteConn = splashScreen->loadCompleteSignal->connectWithId([this, splashScreen](size_t id){
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
        Logger::Write("[Rune::Application] - Successfully showed splash screen!");
    }

    void Application::setApplication(Application* application){
        if (Rune::Application::_app != nullptr) { return; } // Can only be called once
        Rune::Application::_app = application;
    }

    Application* Application::getApplication(){
        return Rune::Application::_app;
    }

    Aether::Window* Application::window(){
        return this->_window;
    }

    void Application::deleteWindow(){
        // this->_window->exit();
        delete this->_window;
        this->_window = nullptr;
    }

    Aether::Screen* Application::getScreen(const std::string& screenName) const{
        auto screenIt = this->screens.find(screenName);
        if (screenIt == this->screens.end()){
            return nullptr;
        }

        return screenIt->second;
    }

    Aether::Screen* Application::showScreen(const std::string& screenName){
        Aether::Screen* screen = this->screens.at(screenName);
    
        if (screen != nullptr){
            Logger::Write("[Rune::Application] - Showing screen '"+screenName+"'...");
            
            // full opacity initially
            float rectOpacity = 0.0f;
            int roundedOpacity;

            // fade out the screen
            Aether::Rectangle* fadeOutRect = nullptr;
            if (this->_currentScreen != nullptr && this->_fadeScreens){ // only fade out if there is a current screen
                fadeOutRect = new Aether::Rectangle(0, 0, WINDOW_RESOLUTION_X, WINDOW_RESOLUTION_Y, 0);
                fadeOutRect->setColour(Aether::Colour{0, 0, 0, 0});

                Logger::Write("[Rune::Application] - Current screen is not null. Fading out screen.");
                this->_currentScreen->addElement(fadeOutRect); // add rect to current screen

                // fade IN the rectangle
                do {
                    roundedOpacity = rectOpacity;

                    fadeOutRect->setColour(Aether::Colour{0, 0, 0, static_cast<char>(std::min(roundedOpacity, 255))});
                    rectOpacity += 150.0f*(static_cast<float>(TWEEN_UPDATE_TIME)/1000000);
                    Logger::Write(std::to_string(roundedOpacity));

                    usleep(TWEEN_UPDATE_TIME);
                } while (rectOpacity < 255.0f);

                Logger::Write("[Rune::Application] - Fade out complete.");
            }

            // store old screen
            Aether::Screen* oldScreen = this->_currentScreen;

            // add rect to new screen
            Aether::Rectangle* fadeInRect = nullptr;

            if (this->_fadeScreens){
                Logger::Write("[Rune::Application] - Adding fade rect to new screen...");
                fadeInRect = new Aether::Rectangle(0, 0, WINDOW_RESOLUTION_X, WINDOW_RESOLUTION_Y, 0);
                fadeInRect->setColour(Aether::Colour{0, 0, 0, 255});
                screen->addElement(fadeInRect);
            }

            // show new screen
            Logger::Write("[Rune::Application] - Showing new screen.");
            this->_window->showScreen(screen);
            this->_currentScreen = screen;
            this->_currentScreenName = screenName;

            if (this->_fadeScreens){
                // remove rect from old screen
                if (fadeOutRect != nullptr && oldScreen != nullptr){
                    oldScreen->removeElement(fadeOutRect);
                }

                // fade in new screen
                Logger::Write("[Rune::Application] - Fading in new screen.");
                do {
                    roundedOpacity = rectOpacity;

                    fadeInRect->setColour(Aether::Colour{0, 0, 0, static_cast<char>(std::max(roundedOpacity, 0))});
                    rectOpacity -= 150.0f*(static_cast<float>(TWEEN_UPDATE_TIME)/1000000);
                    Logger::Write(std::to_string(roundedOpacity));

                    usleep(TWEEN_UPDATE_TIME);
                } while (rectOpacity > 0.0f);
                Logger::Write("[Rune::Application] - Fade in complete.");

                // remove rect from new screen
                Logger::Write("[Rune::Application] - Removing rect from new screen...");
                this->_currentScreen->removeElement(fadeInRect);
            }

        } else {
            Logger::Write("[Rune::Application] - Screen is nullptr. Ignoring.");
        }

        return screen;
    }

    std::string Application::currentScreenName(){
        return this->_currentScreenName;
    }

    Aether::MessageBox* Application::showLoading(const std::string& msg, bool showProgress, bool noUpdate){
        // Show loading
        if (this->_loadingMsg == nullptr){
            this->_loadingMsg = new Aether::MessageBox("Loading...");

            _loadingMsg->setRectangleColour(BACKGROUND_COLOUR);
            _loadingMsg->setLineColour(Aether::Colour{0, 0, 0, 0});
            _loadingMsg->setBodySize(770, 320);
            _loadingMsg->setTextColour(TEXT_COLOUR);
        }

        if (this->_loadingProgress == nullptr){
            _loadingMsg->addTopButton("", [](){});
            _loadingMsg->topButton()->setHidden(true);
            
            this->_loadingProgress = new Aether::RoundProgressBar((WINDOW_RESOLUTION_X/2)-(385/2), 400, 385);
            _loadingProgress->setBackgroundColour(MUTED_BACKGROUND_COLOUR);
            _loadingProgress->setForegroundColour(TOP_R);
            _loadingProgress->setValue(0);

            _loadingMsg->addElement(_loadingProgress);
        }

        _loadingMsg->setString(msg);

        _loadingProgress->setHidden(!showProgress);
        _loadingProgress->setValue(0.0f);
        
        // Add the loading message and force a UI update
        this->_window->addOverlay(_loadingMsg);
        if (!noUpdate){
            this->_window->loop();
        }

        return _loadingMsg;
    }

    void Application::setLoadingProgress(float progress){
        if (this->_loadingProgress == nullptr){
            return;
        }

        _loadingProgress->setValue(progress*100);
        this->_window->loop();
    }

}