// portlibs
#include <switch.h>
#include <Aether/Aether.hpp>

// source libs
#include <Events/Signal.hpp>
#include <Input/Input.hpp>
#include <Utils/Utils.hpp>
#include <Save/Save.hpp>

#include <Screens/Menu.hpp>
#include <Config.hpp>

// declarations
bool exitQueued = false;
InputSystem& inputService = InputSystem::getService();
RuneSaveMetadata saveMetadata;
bool mountSuccess = true;

/**
 * Exits the applet.
 * @return none
 */
void exitApplet(InputSystem& inputService, InputState state){
    if (state == InputState::Begin){
        // do exit code here for a safe shutdown

        // queue software exit
        exitQueued = true;
    }
}   

/**
 * Initialises the applet.
 * @return Result
 */
Result initApplet(){
    log("[INIT] - Initialising RUNE...");

    AppletType type = appletGetAppletType();

    // bind exit
    inputService.init();
    log("[INIT] - Initialised InputService");
    
    inputService.bind("Exit", InputObject(InputType::Gamepad, HidNpadButton_Plus));
    inputService.connect("Exit", exitApplet);

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

        consoleUpdate(NULL);
        return -4;
    }

    // load save metadata
    Result rc = runeSaveLoadMetadata(&saveMetadata);
    if (R_FAILED(rc)){
        log("Failed to parse BotW .bgsvdata metadata.");
        exitApplet(inputService, InputState::Begin);
        return -2;
    }

    log("[INIT] - Parsed save metadata files");

    // initialise the account system
    rc = accountInitialize(AccountServiceType_Application);
    if (R_FAILED(rc)){
        return -1;
    }

    log("[INIT] - Initialised AccountService");
    
    // initialise romfs
    rc = romfsInit();
    if (R_FAILED(rc)){
        log("[MAIN] - Failed to initialize RomFS: " + std::to_string(rc));
        return -3;
    }

    log("[INIT] - Initialised RomFS");

    // mount save data
    rc = runeSaveMountSaveData();
    if (R_FAILED(rc)){
        log("Failed to mount BotW save data.");
        mountSuccess = false;
        // return;
    }
    else {
        log("[INIT] - Mounted BotW save data");
    }


    // init finished, log ready
    if (mountSuccess){
        log("[INIT] - Initialised RUNE successfully!");
    }
    else {
        log("[INIT] - Initialised RUNE, but with problems.");
    }

    return 0;
}

int main(){
    Result rc = initApplet();
    Result appletLaunchStatus = rc;

    // stop processing and show error if failed to initialise
    // if (R_FAILED(appletLaunchStatus)){
    //     log("[INIT] - an error ocurred when initialising the applet. Exiting...");
    //     exitApplet(inputService, InputState::Begin);
    // }

    // if failed, wait for + press
    if (R_FAILED(appletLaunchStatus)){
        while(appletMainLoop() && !exitQueued){
            inputService.update();
        }

        consoleExit(NULL);
        return 0;
    }

    // the UI window
    Aether::Window* window = new Aether::Window("Rune", WINDOW_RESOLUTION_X, WINDOW_RESOLUTION_Y, [](const std::string message, const bool important){
        if (important){
            log("[!! AETHER !!] - "+message);
        } else {
            log("[AETHER] - "+message);
        }
    });

    log("[MAIN] - Successfully loaded Aether window!");

    // prepare the menu
    Aether::Screen* menu = new Menu(window);
    window->showScreen(menu);

    log("[MAIN] - Successfully loaded menu screen!");

    while (window->loop() && !exitQueued){
        // do updates
        inputService.update();

        // main code here
    }

    log("[MAIN] - Exiting RUNE...");

    // close romfs
    rc = romfsExit();
    if (R_FAILED(rc)){
        log("[MAIN] - Failed to exit RomFS: " + std::to_string(rc));
    }

    accountExit();

    // if (R_FAILED(rc)){
    //     log("[MAIN] - Failed to exit accountSvc: " + std::to_string(rc));
    // }

    delete window;
    return 0;
}