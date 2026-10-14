#include <Aether/Aether.hpp>
#include <functional>

class WidgetSpinner : public Aether::Spinner {
    private:
        std::function<void()> _onPress;

    public:
        using Aether::Spinner::Spinner;

        /**
         * @brief A function to run when the Spinner has been pressed (A/touch)
         * @param std::function<void()> callback to run when pressed
         */
        void onPress(std::function<void()> callback) {
            this->_onPress = std::move(callback);
        }

        bool handleEvent(Aether::InputEvent* event) override;
};