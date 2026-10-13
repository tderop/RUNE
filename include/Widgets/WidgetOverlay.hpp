#include <Aether/Aether.hpp>

#include <Utils/Utils.hpp>
#include <Utils/Trove.hpp>

#include <functional>

const int OVL_PANEL_WIDTH = 1280;
const int OVL_PANEL_HEIGHT = 470; // really?

class WidgetOverlay : public Aether::Overlay{
    private:
        Trove* _trove;
        Aether::Text* _titleLabel = nullptr;
        Aether::Rectangle* _panel = nullptr;

        std::function<void()> _onSave;

    public:
        WidgetOverlay(const std::string &title="");
        ~WidgetOverlay();

        /**
         * @brief Clear the widget overlay's temporary element trove.
         */
        void clearTrove();

        /**
         * @brief Close the widget overlay.
         */
        void closeWidgetOverlay();

        /**
         * @brief Get the panel.
         */
        Aether::Rectangle* panel();

        /**
         * @brief Add an element that will be deleted when the overlay menu closes.
         */
        void addTempElement(Aether::Element* element);

        /**
         * @brief Set the title text content of the widget overlay.
         */
        void setTitleLabel(const std::string &title);
};