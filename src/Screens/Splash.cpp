#include <Screens/Splash.hpp>
#include <Aether/Aether.hpp>
#include <Config.hpp>

#include <Utils/Logger.hpp>
#include <Utils/Trove.hpp>

#include <Events/Signal.hpp>

#include <Application/Application.hpp>
#include <Application/Romfs.hpp>
#include <Application/Audio.hpp>

#include <Editor/Menus/Inventory.hpp>

#include <math.h>

const size_t TEXTURES_PER_FRAME = 16;
const bool PRELOAD_ICON_TEXTURES = false;

SplashScreen::SplashScreen(Rune::Application* app) : app(app) {
    this->_loaded.store(false);
    Logger::Write("[SPLASH] - Successfully initialised splash screen!");
}

void SplashScreen::buildScreen(){
    // show the splash image
    Logger::Write("[SPLASH] - Making splash image...");
    this->_splashImg = new Aether::Image(
        994,
        640,
        ASSET_PATHS.at("splash"),
        Aether::Render::Wait
    );

    // resize
    Logger::Write("[SPLASH] - Rendering splash image...");

    this->_splashImg->setScaleDimensions(270, 64);
    Logger::Write("[SPLASH] - Set image scale dimensions.");

    this->_splashImg->renderSync(); // render image
    Logger::Write("[SPLASH] - Image rendered successfully.");

    this->addElement(this->_splashImg);
    Logger::Write("[SPLASH] - Splash image element added to screen.");

    // Make status text
    int statusTextSize = 20;
    int textPadding = 12;

    this->_statusText = new Aether::Text(
        0, 0,
        "",
        statusTextSize
    );

    this->_statusText->setColour(TEXT_COLOUR);
    this->_statusText->setXY(
        statusTextSize+textPadding,
        WINDOW_RESOLUTION_Y-(statusTextSize*2)-textPadding
    );

    this->addElement(this->_statusText);

    Logger::Write("[SPLASH] - Screen built.");
}

void SplashScreen::loadRune(void* _self){
    SplashScreen* self = static_cast<SplashScreen*>(_self);

    Trove* loadingTrove = new Trove();

    // Initialise JSON files (parse them and turn them into JSON objects, without initialisation the JSON files would be inaccessible through Rune::JsonData methods)
    {
        std::lock_guard<std::mutex> lock(self->_statusMutex);
        self->_status="Initialising metadata...";
    }
    Rune::JsonData::Init();

    // Step 0: Load metadata
    Result rc = Rune::Save::LoadMetadata(runeMetadata);
    if (R_FAILED(rc)){
        // RUNE CANNOT load without metadata.
        std::lock_guard<std::mutex> lock(self->_statusMutex);
        self->_status = "Failed to load metadata ("+intToHex(rc)+") // Press PLUS (+) to exit.";
        return;
    }

    // Step 1: Extract Assets
    // Extract assets and report progress
    rc = Rune::Romfs::ExtractAssets(
        [self](size_t completed, size_t max){
            std::string suffix;

            if (max != 0){ // prevent div by 0
                suffix = std::to_string(static_cast<int>((static_cast<float>(completed)
                /static_cast<float>(max))*100))+"%";
            }

            {
                std::lock_guard<std::mutex> lock(self->_statusMutex);
                self->_status = "Extracting assets: "+suffix;
            }
        }
    );

    // Step 2: Equipment category cache
    Rune::Json* actorData;
    Rune::JsonData::GetJsonById(actorData, "ActorData");
    
    {
        std::lock_guard<std::mutex> lock(Rune::Inventory::CategoryActorsMutex);
        for (const auto&[name, data]: actorData->items()){
            if (Rune::Inventory::GetActorValueFromObject<bool>(data, "Hidden")){ continue; } // ignore hidden items
            Rune::Inventory::Category category = Rune::Inventory::GetCategoryFromActorName(name);

            Rune::Inventory::CategoryActors[category].push_back(name);
        }
    }

    // Step 3: Load sounds
    // The sound susbsystem being uninitialised is not a problem, so just print error code and proceed anyway
    {
        std::lock_guard<std::mutex> lock(self->_statusMutex);
        self->_status = "Loading sounds...";
    }

    const char* audioErr = Rune::Audio::Init();
    if (audioErr != nullptr){
        Logger::Importantf("[INIT] - Failed to initialise sound subsystem: %s", audioErr);
    }

    // I'm loaded, run other loading steps
    self->_workerFinished.store(true);
    return;
}

void SplashScreen::onLoad(){
    this->_alive = true;
    this->_threadStarted = false;
    this->_loaded.store(false);

    // Make loading thread
    Result rc = threadCreate(
        &this->_loadingThread,
        this->loadRune,
        this,
        nullptr,
        0x10000,
        0x2C,
        -2
    );

    if (R_FAILED(rc)){
        this->_loaded = true; // set loaded to proceed anyway
        return;
    }

    rc = threadStart(&this->_loadingThread);

    if (R_FAILED(rc)){
        this->_loaded = true;
        return;
    }

    this->_threadStarted = true;
}

void SplashScreen::onUnload(){
    // delete the image and set alive status
    this->_alive = false;
    this->_elapsed = 0;
    Logger::Write("[SPLASH] - Screen killed.");
}

void SplashScreen::update(unsigned int dt){
    if (!this->_alive || this->_splashImg == nullptr){
        return;
    }

    // Calculate transparency of splash logo based on dt
    this->_elapsed += dt;
    float transparency = 255*((sin(static_cast<float>(this->_elapsed) / 500.0f) / 5.0f) + 0.8f);
    int roundedTransparency = static_cast<int>(transparency);
    this->_splashImg->setColour(Aether::Colour{255, 255, 255, static_cast<char>(roundedTransparency)});
    
    // Read status text, and update UI accordingly
    {
        std::lock_guard<std::mutex> lock(this->_statusMutex);
        if (this->_status != this->_previousStatus 
            && this->_statusText != nullptr){

            this->_statusText->setString(this->_status);
        }   

        this->_previousStatus = this->_status;
    }


    // Upload on main thread for renderer
    Aether::Renderer* renderer = this->app->window()->renderer();
    for (size_t i = 0; i < TEXTURES_PER_FRAME; ++i){
        PendingTexture pending;
        {
            std::lock_guard<std::mutex> lock(this->_textureMutex);

            if (this->_textureQueue.empty()){
                break;
            }

            // Pop a texture to be rendered
            pending = std::move(this->_textureQueue.front());
            this->_textureQueue.pop();
        }
        
        // Convert the surface to a texture on the main thread
        auto& drawable = pending.drawable;
        drawable->convertToTexture();
    }

    // Check if the texture worker has finished
    if (this->_workerFinished.load()){
        std::lock_guard<std::mutex> lock(this->_textureMutex);

        // If the texture queue is empty, set loaded to true
        if (this->_textureQueue.empty()){
            this->_loaded.store(true);
        }
    }

    // When loaded, close worker thread and fire load complete signal, move to main menu
    if (this->_loaded.load() && this->_elapsed >= MIN_SPLASH_SCREEN_TIME){
        if (this->_threadStarted){
            threadWaitForExit(&this->_loadingThread);
            threadClose(&this->_loadingThread);
            
            this->_threadStarted = false;
        }

        this->_statusText->setString("Done!");
        this->loadCompleteSignal->fire();
    }

    Aether::Screen::update(dt);
}