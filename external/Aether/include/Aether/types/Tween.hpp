#ifndef AETHER_TWEEN_HPP
#define AETHER_TWEEN_HPP

#include "Aether/base/Element.hpp"
#include <optional>
#include <algorithm>

namespace Aether {
    /** @brief How the tween eases over time */
    enum class EasingStyle {
        Linear,
        Cubic
    };

    /** @brief The direction of the easing */
    enum class EasingDirection {
        None,
        In,
        Out,
        InOut
    };

    enum class Property {
        Position,
        Size,
        Transparency
    };

    using xyProperty = std::optional<std::pair<int, int>>;
 
    struct PropertyTable {
        xyProperty Position;
        xyProperty Size;
        std::optional<uint8_t> Transparency;
    };

    class Tween {
        private:
            inline static Window* win_=nullptr;
            Element* elm_;
            double duration_;
            EasingStyle style_;
            EasingDirection direction_;
            PropertyTable propertyTable_;

            bool cancelling_=false;
            double applyEasing(double t) const;

        public:
            Tween(Element* e, double seconds, EasingStyle style=EasingStyle::Linear, EasingDirection direction=EasingDirection::Out, PropertyTable propertyTable={});
            static void setWindow(Window* window);
            
            void play();
            void cancel();

            ~Tween();
    };
} // namespace Aether
#endif