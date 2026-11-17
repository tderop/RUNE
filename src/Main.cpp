// libnx
#include <switch.h>

// Source libs
#include <Input/Input.hpp> // Provides my own InputService
#include <Utils/Logger.hpp> // Provides my own logging service
#include <Save/Save.hpp> // Provides Rune::Save::Metadata functionality and other Rune::Save types for parsing metadata on load
#include <Application/Application.hpp> // Provides Rune::Application for managing different screens
#include <Application/Audio.hpp>
#include <Json/Json.hpp> // Provides Rune::Json and Rune::JsonData for handling RUNE's romFS JSON files
#include <Application/Romfs.hpp> // Handles extracting assets from the local BotW installation
#include <Application/Tests.hpp>
#include <Config.hpp>

// External libs
#include <Aether/Aether.hpp> // The main UI library used by RUNE, initially developed by @tallbl0nde, but slightly modified by myself

// C++ libs
#include <filesystem> // Moved an fopen call to std::filesystem::exists to be less evil
#include <chrono> // used for startup for small delay before running program to check for test execution

// C libs
#include <cstdlib> // Required for std::exit for early exits (when the application was launched in applet mode instead of with title takeover)

#define DEBUG_MODE false

// Checked every "frame" - determines whether the application should exit or not.
bool exitQueued = false;

// Determines whether the BotW save data was successfully mounted at save:/
bool mountSuccess = true;

// A bool pointer for the Application Manager to reference to determine if the launch was successful or not.
bool* initSuccess = &mountSuccess;

// The save metadata. Provides additional information about save entries
Rune::Save::Metadata saveMetadata;

// The application manager for RUNE
Rune::Application* app = nullptr;

// My own input system. Makes getting raw inputs and binding them to actions much more convenient.
InputSystem& inputService = InputSystem::getService();

/**
 * @brief Exits the applet.
 * @param inputService the main InputService object
 * @param state the InputState provided by the InputService event
 */
void exitApplet(InputSystem& inputService, InputState state){
    // If the exit button has just been pressed down, queue exit
    if (state == InputState::Begin){
        // If there is a valid app, and the current active screen is the main menu, queue exit
        if (app != nullptr && app->window() != nullptr 
            && (app->currentScreenName() == "Menu" || app->currentScreenName() == "Splash")){

            app->window()->exit();
        } else if (app == nullptr){ // If app is uninitialised, exit
            exitQueued = true;
        }   
    }
}   

/**
 * @brief Initialises the applet.
 * @return Result
 */
Result initApplet(){
    // Overclock the CPU, and downclock the GPU
    // RUNE is not GPU hungry but frequently performs heavy CPU work
    Result rc = appletSetCpuBoostMode(ApmCpuBoostMode_FastLoad);
    if (R_FAILED(rc)){
        Logger::Importantf("Failed to set CPU boost mode: %08X", rc);
    } // no boost is fine, it just means loading is slower

    // Check for the presence of an emulator to determine whether Logger::Write() should print messages to the console or not
    // This is unique to my own emulator development environment, I put an empty file called "emulator" on the virtual SD card
    if (std::filesystem::exists("sdmc:/emulator")){
        Rune::Application::IsEmulator = true;
    }

    Logger::Write("[INIT] - Initialising RUNE...");

    // Determine the applet type
    // This is in order to prevent launching RUNE in applet mode; it will crash without a selected user and may not have enough
    // memory to run otherwise.
    AppletType type = appletGetAppletType();

    // Initialise InputService
    inputService.init();
    Logger::Write("[INIT] - Initialised InputService");
    
    // Bind the exit keybind
    inputService.bind("Exit", InputObject(InputType::Gamepad, HidNpadButton_Plus));
    inputService.connect("Exit", exitApplet);

    // If RUNE is in applet mode, initialise the console and tell the user with a big
    // warning to open RUNE in title takeover mode
    if (type != AppletType_Application && type != AppletType_SystemApplication){
        consoleInit(NULL);
        printf("\x1b[1;31m======================================\x1b[0m\n\n");
        printf("\x1b[1;31m               WARNING\x1b[0m\n\n");
        printf("\x1b[1;31m======================================\x1b[0m\n\n");

        printf("\x1b[1;31mRUNE WAS NOT LAUNCHED IN TITLE TAKEOVER MODE.\x1b[0m\n\n");

        printf("\x1b[1;33mRUNE cannot run in applet mode.\x1b[0m\n");
        printf("\x1b[1;33mPlease relaunch RUNE using TITLE TAKEOVER.\x1b[0m\n\n");

        printf("\x1b[1;36mHOW TO LAUNCH IN TITLE TAKEOVER MODE:\x1b[0m\n");
        printf("  \x1b[1;37m1.\x1b[0m Return to the HOME Menu.\n");
        printf("  \x1b[1;37m2.\x1b[0m Hold \x1b[1;32m[R]\x1b[0m while launching any game.\n");
        printf("  \x1b[1;37m3.\x1b[0m Continue holding \x1b[1;32m[R]\x1b[0m until the homebrew menu appears.\n\n");

        printf("\x1b[1;36mRUNE requires title takeover to access your\x1b[0m\n");
        printf("\x1b[1;36muser and save data correctly.\x1b[0m\n\n");

        printf("\x1b[1;37m--------------------------------------\x1b[0m\n\n");
        printf("\x1b[1;37m             \x1b[1;32m[+]\x1b[1;37m  EXIT\x1b[0m\n\n");
        printf("\x1b[1;37m--------------------------------------\x1b[0m\n");

        // Update the console to show prints
        consoleUpdate(NULL);

        // Wait for an exit to be queued by pressing "+"
        while(appletMainLoop() && !exitQueued){
            inputService.update();
        }

        // Close the console
        consoleExit(NULL);

        // Force exit the program
        std::exit(EXIT_SUCCESS);
    }

    // initialise romFS for accessing RUNE assets
    rc = romfsInit();
    if (R_FAILED(rc)){
        Logger::Write("[MAIN] - Failed to initialize RomFS: " + std::to_string(rc));
        return -3; // These arbitrary negative values just specify what type of error ocurred during applet initialisation.
        // Currently, they don't do anything, but this value may get used in the future, that is why I assigned each failure route with a unique error code.
    }

    Logger::Write("[INIT] - Initialised RomFS");
    
    if (DEBUG_MODE){
        // Initialise console so program begins running ready for input
        consoleInit(NULL);
        puts("[RUNE DEBUG] - Press L to run tests");

        // Initialise tests and do not proceed if L is held during launch
        InputObject testsButton = InputObject(InputType::Gamepad, HidNpadButton_L);
        inputService.update(); // capture a snapshot of the current input states

        // Before initialising UI, check if the user presses L to begin tests
        const auto end = std::chrono::steady_clock::now() 
        + std::chrono::seconds(STARTUP_DELAY);

        bool runTests = false;
        while (std::chrono::steady_clock::now() < end) {
            consoleUpdate(NULL);
            inputService.update();
            
            // If L is pressed, break and run tests
            if (inputService.buttonPressed(testsButton)
            || inputService.buttonHeld(testsButton)){
                runTests = true;
                break;
            }
        }

        bool exitRune = false;

        if (runTests){
            puts("------------");
            Rune::Tests::Run();
            puts("------------");

            puts("Press L to continue to RUNE.");
            puts("Press PLUS (+) to exit RUNE.");

            while (appletMainLoop()){
                consoleUpdate(NULL);
                inputService.update();

                // Break and exit if plus is pressed
                if(inputService.buttonHeld(
                    InputObject(InputType::Gamepad, HidNpadButton_Plus)
                )){
                    exitRune = true;
                    break;
                }

                // Just continue if L is held
                if (inputService.buttonPressed(
                    testsButton
                )){
                    break;
                }
            }
        }

        // exit the console once tests/startup delay end
        consoleClear();
        consoleExit(NULL);

        // exit ROMFS and exit program if + was pressed
        if (exitRune){
            romfsExit();
            std::exit(EXIT_SUCCESS);
        }
    }

    // Extract assets from BotW on first launch
    // rc = Rune::Romfs::ExtractAssets();

    // Initialise the RUNE application manager. This handles screen management.
    app = new Rune::Application(initSuccess, true);
    Rune::Application::setApplication(app);

    // Initialise Aether tweening
    Aether::Tween::setWindow(app->window());

    Logger::Write("[Rune::Application] - Successfully loaded application manager.");

    // Initialise the account service. This is used for knowing which user's save file should be accessed.
    rc = accountInitialize(AccountServiceType_Application);
    if (R_FAILED(rc)){
        return -1;
    }

    Logger::Write("[INIT] - Initialised AccountService");

    // Mount the BotW Save Data. This mounts the save data as an accessible filesystem on save:/
    // Making changes to this filesystem without using fsdevCommitDevice("save:/") will not save changes (as I learned the hard way)
    rc = Rune::Save::MountSaveData();
    if (R_FAILED(rc)){
        Logger::Write("[INIT] - Failed to mount BotW save data.");
        mountSuccess = false;
    }
    else {
        Logger::Write("[INIT] - Mounted BotW save data");
    }

    // The application initialised successfully
    if (mountSuccess){
        Logger::Write("[INIT] - Initialised RUNE successfully!");
    }
    else { // Allow launch with failures for now, mainly for testing purposes
        Logger::Write("[INIT] - Initialised RUNE, but with problems.");
    }

    return 0;
}

int main(){
    // Initialise RUNE
    initApplet();

    // If you're wondering where the main code is, check /src/Application/Application.cpp
    // This handles showing the splash screen, and then loading the first screen: the menu (Menu.cpp)
    // This is the case because of the new Rune::Application that was created in initApplet()
    Result rc;

    // The rest of the main function covers InputService polling, and exiting services when closing
    while (app->window()->loop()){
        // Update InputService (allows for InputService connections, input detection, basically required for the whole system to work)
        inputService.update();
    }

    Logger::Write("[MAIN] - Exiting RUNE...");

    // Exit the account service
    accountExit();

    // Exit the audio service
    Rune::Audio::Exit();

    // Exit romFS (the path used to access RUNE's assets such as images and JSON files)
    rc = romfsExit();
    if (R_FAILED(rc)){
        Logger::Write("[MAIN] - Failed to exit RomFS: " + std::to_string(rc));
    }

    // Delete the window for full cleanup
    if (app->window() != nullptr){
        app->deleteWindow();
    }

    // Exit SDL
    SDL_Quit();

    // Reset CPU mode
    appletSetCpuBoostMode(ApmCpuBoostMode_Normal);
    return 0;
}