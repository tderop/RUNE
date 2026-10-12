#include <Screens/Splash.hpp>
#include <Aether/Aether.hpp>
#include <Config.hpp>
#include <Utils/Utils.hpp>

#include <Events/Signal.hpp>

#include <Application/Application.hpp>
#include <math.h>

SplashScreen::SplashScreen(Rune::Application* app) : app(app) {
    log("[SPLASH] - Successfully initialised splash screen!");
}

void SplashScreen::buildScreen(){
    // show the splash image
    log("[SPLASH] - Making splash image...");
    Aether::Image* splashImg = new Aether::Image(
        940,
        648,
        ASSET_PATHS.at("splash"),
        Aether::Render::Wait
    );

    // resize
    log("[SPLASH] - Rendering splash image...");

    // STUCK (?)
    splashImg->setScaleDimensions(270, 64);
    log("[SPLASH] - Set image scale dimensions.");

    // STUCK ON RENDERING... WHY?
    splashImg->renderSync(); // render image

    log("[SPLASH] - Image rendered successfully.");

    // maybe frozen because unrendered?
    this->addElement(splashImg);
    log("[SPLASH] - Splash image element added to screen.");

    this->_splashImg = splashImg;
    log("[SPLASH] - Set splash image private value.");

    log("[SPLASH] - Screen built.");
}

void SplashScreen::onLoad(){
    this->_alive = true;
}

void SplashScreen::onUnload(){
    // delete the image and set alive status
    this->_alive = false;
    log("[SPLASH] - Screen killed.");
}

void SplashScreen::update(unsigned int dt){
    if (!this->_alive || this->_splashImg == nullptr){
        return;
    }

    // stub for test
    // calculate transparency based on dt
    this->_elapsed += dt;
    if (this->_elapsed >= MIN_SPLASH_SCREEN_TIME){
        this->loadCompleteSignal->fire();
    }
    
    float transparency = 255*((sin(static_cast<float>(this->_elapsed) / 500.0f) / 5.0f) + 0.8f);
    int roundedTransparency = static_cast<int>(transparency);

    this->_splashImg->setColour(Aether::Colour{255, 255, 255, static_cast<char>(roundedTransparency)}); // does transparency even work for this? it does
}