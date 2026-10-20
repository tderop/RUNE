// portlibs
#include <switch.h>
#include <Aether/Aether.hpp>

// source libs
#include <Events/Signal.hpp>
#include <Input/Input.hpp>
#include <Utils/Utils.hpp>
#include <Save/Save.hpp>
#include <Application/Application.hpp>
#include <Json/Json.hpp>
#include <Screens/Menu.hpp>
#include <Config.hpp>

#include <cstdlib>

// declarations
bool exitQueued = false;
bool mountSuccess = true;

Rune::Save::Metadata saveMetadata;
Rune::Application* app = nullptr;

InputSystem& inputService = InputSystem::getService();

/**
 * Exits the applet.
 * @return none
 */
void exitApplet(InputSystem& inputService, InputState state){
    if (state == InputState::Begin){
        if (app != nullptr && app->currentScreenName() == "Menu"){
            exitQueued = true;
        } else if (app == nullptr){
            exitQueued = true;
        }   
    }
}   

/**
 * Initialises the applet.
 * @return Result
 */
Result initApplet(){
    // check for emulator presence
    FILE* emulatorFile = fopen("sdmc:/emulator", "rb");
    if (emulatorFile != nullptr){
        logServiceIsEmulator = true;
        log("[INIT] - Dev emulator environment detected!");
    }

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

        while(appletMainLoop() && !exitQueued){
            inputService.update();
        }

        consoleExit(NULL);
        std::exit(EXIT_SUCCESS);
    }

    // initialise romfs
    Result rc = romfsInit();
    if (R_FAILED(rc)){
        log("[MAIN] - Failed to initialize RomFS: " + std::to_string(rc));
        return -3;
    }

    log("[INIT] - Initialised RomFS");

    app = new Rune::Application();
    log("[Rune::Application] - Successfully loaded application manager.");

    // initialise JSON
    Rune::JsonData::Init();
    log("[INIT] - Initialised Rune::Json");

    // load save metadata
    rc = Rune::Save::LoadMetadata(&saveMetadata);
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

    // mount save data
    rc = Rune::Save::MountSaveData();
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
    // the application
    initApplet();

    // show the menu once all has loaded
    // app->showScreen("Menu");

    while (app->window()->loop() && !exitQueued){
        // do updates
        inputService.update();

        // main code here
    }

    log("[MAIN] - Exiting RUNE...");

    // close romfs
    Result rc = romfsExit();
    if (R_FAILED(rc)){
        log("[MAIN] - Failed to exit RomFS: " + std::to_string(rc));
    }

    accountExit();

    // if (R_FAILED(rc)){
    //     log("[MAIN] - Failed to exit accountSvc: " + std::to_string(rc));
    // }

    app->deleteWindow();
    return 0;
}