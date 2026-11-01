#include <Screens/Splash.hpp>
#include <Aether/Aether.hpp>
#include <Config.hpp>

#include <Utils/Utils.hpp>
#include <Utils/Trove.hpp>

#include <Events/Signal.hpp>

#include <Application/Application.hpp>
#include <Application/Romfs.hpp>

#include <Editor/Inventory.hpp>

#include <math.h>
#include <filesystem>

const size_t TEXTURES_PER_FRAME = 16;

SplashScreen::SplashScreen(Rune::Application* app) : app(app) {
    this->_loaded.store(false);
    log("[SPLASH] - Successfully initialised splash screen!");
}

void SplashScreen::buildScreen(){
    // show the splash image
    log("[SPLASH] - Making splash image...");
    this->_splashImg = new Aether::Image(
        994,
        640,
        ASSET_PATHS.at("splash"),
        Aether::Render::Wait
    );

    // resize
    log("[SPLASH] - Rendering splash image...");

    this->_splashImg->setScaleDimensions(270, 64);
    log("[SPLASH] - Set image scale dimensions.");

    this->_splashImg->renderSync(); // render image
    log("[SPLASH] - Image rendered successfully.");

    this->addElement(this->_splashImg);
    log("[SPLASH] - Splash image element added to screen.");

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

    log("[SPLASH] - Screen built.");
}

void SplashScreen::loadRune(void* _self){
    SplashScreen* self = static_cast<SplashScreen*>(_self);

    Trove* loadingTrove = new Trove();

    // Step 1: Extract Assets
    // Extract assets and report progress
    Result rc = Rune::Romfs::ExtractAssets(
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

    // Step 2: Preload textures
    rc = Rune::Inventory::PreloadTextures(
        self->app->window()->renderer(),

        // Add texture to queue to be rendered
        [self](const std::string& key, std::unique_ptr<Aether::Drawable> drawable){
            std::lock_guard<std::mutex> lock(self->_textureMutex);
            self->_textureQueue.push({
                key, std::move(drawable)
            });
        },

        // Show progress
        [self](size_t completed, size_t max){
            std::string suffix;

            if (max != 0){ // prevent div by 0
                suffix = std::to_string(static_cast<int>((static_cast<float>(completed)
                /static_cast<float>(max))*100))+"%";
            }

            {
                std::lock_guard<std::mutex> lock(self->_statusMutex);
                self->_status = "Preloading textures: "+suffix;
            }
        }
    );

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
    log("[SPLASH] - Screen killed.");
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