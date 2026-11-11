#include "Aether/Window.hpp"
#include "Aether/types/Tween.hpp"
#include "Aether/base/Texture.hpp"

#include <chrono>

namespace Aether {
    Tween::Tween(Element* e, double seconds, EasingStyle style, EasingDirection direction, PropertyTable propertyTable) 
        : elm_(e), duration_(seconds), style_(style), direction_(direction), propertyTable_(propertyTable){}

    void Tween::setWindow(Window* window){
        Tween::win_ = window;
    }

    double Tween::applyEasing(double t) const{
        t = std::clamp(t, 0.0, 1.0);
        if (style_ == EasingStyle::Linear || direction_ == EasingDirection::None){
            return t;
        }

        switch (direction_){
            case (EasingDirection::In): {
                if (style_ == EasingStyle::Cubic){
                    return std::pow(t, 3.0);
                }
            }
            case (EasingDirection::Out): {
                if (style_ == EasingStyle::Cubic){
                    return 1.0 - std::pow(1.0 - t, 3.0);
                }
            }
            case (EasingDirection::InOut): {
                if (style_ == EasingStyle::Cubic){
                    return t < 0.5 ? 4.0 * std::pow(t, 3.0) : 1.0 - std::pow(-2.0 * t + 2.0, 3.0) / 2.0;
                }
            }

            default:
                return t;
        }
        return 0;
    }

    void Tween::play(){
        // Sanity check
        if (cancelling_ || elm_ == nullptr){
            return;
        }

        // Promote to texture
        Texture* tex = nullptr;
        Colour startCol{255, 255, 255, 255};

        if (propertyTable_.Transparency.has_value()){
            tex = static_cast<Texture*>(elm_);
            if (tex != nullptr){
                startCol = tex->colour();
            }
        }

        const int startX = elm_->x();
        const int startY = elm_->y();
        const int startW = elm_->w();
        const int startH = elm_->h();
        
        const int startCentreX = elm_->centreX();
        const int startCentreY = elm_->centreY();

        const auto startTime = std::chrono::steady_clock::now();
        bool completed=false;

        // Tween properties
        while (win_->loop() && !completed && !cancelling_){
            const auto now = std::chrono::steady_clock::now();
            const double elapsed = std::chrono::duration<double>(now - startTime).count();

            double progress = (duration_ > 0.0) ? elapsed / duration_ : 1.0;
            if (progress >= 1.0){
                progress = 1.0; // final pass set to to exact values
                completed = true;
            }

            const double eased = applyEasing(progress);

            // Handle properties
            if (propertyTable_.Position.has_value()){
                const auto [targetX, targetY] = propertyTable_.Position.value();
                const int x = static_cast<int>(std::round(startX + (targetX - startX) * eased));
                const int y = static_cast<int>(std::round(startY + (targetY - startY) * eased));
                elm_->setXY(x, y);
            }
            
            if (propertyTable_.Size.has_value()){
                const auto [targetW, targetH] = propertyTable_.Size.value();
                const int w = static_cast<int>(std::round(startW + (targetW - startW) * eased));
                const int h = static_cast<int>(std::round(startH + (targetH - startH) * eased));

                elm_->setWH(w, h);

                if (!propertyTable_.Position.has_value()){
                    // Centre instead
                    elm_->setXY(startCentreX-w/2, startCentreY-h/2);
                }
            }

            if (tex != nullptr && propertyTable_.Transparency.has_value()){
                uint8_t targetA = propertyTable_.Transparency.value();
                uint8_t a = static_cast<uint8_t>(std::round(startCol.a() + (targetA - startCol.a()) * eased));
                tex->setColour(Colour{startCol.r(), startCol.g(), startCol.b(), a});
            }
        }

        // Unset cancelling
        this->cancelling_=false;
    }

    void Tween::cancel(){
        this->cancelling_=true;
    }

    Tween::~Tween(){
        this->cancel();
    }
} // namespace Aether