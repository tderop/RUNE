#include "Aether/Overlay.hpp"

// Background colour
#define BG_COLOUR Colour{0, 0, 0, 150}

namespace Aether {
    Overlay::Overlay() : Screen() {
        this->close_ = false;
    }

    void Overlay::close() {
        if (this->onCloseFunc_){
            this->onCloseFunc_();
        }
        
        this->close_ = true;
    }

    void Overlay::reuse() {
        this->close_ = false;
    }

    void Overlay::opened(){
        if (this->onOpenFunc_){
            this->onOpenFunc_();
        }
    }

    bool Overlay::shouldClose() {
        return this->close_;
    }

    void Overlay::render() {
        // Draw background
        this->renderer->fillWindow(BG_COLOUR);

        // Draw elements
        Screen::render();
    }

    void Overlay::onOpen(std::function<void()> f){
        this->onOpenFunc_ = f;
    }

    void Overlay::onClose(std::function<void()> f){
        this->onCloseFunc_ = f;
    }
};