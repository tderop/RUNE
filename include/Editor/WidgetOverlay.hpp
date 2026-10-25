#pragma once

#include <Aether/Aether.hpp>

#include <Utils/Utils.hpp>
#include <Utils/Trove.hpp>
#include <Events/Signal.hpp>

#include <functional>

const int OVL_PANEL_WIDTH = 1280;
const int OVL_PANEL_HEIGHT = 470;

class WidgetOverlay : public Aether::Overlay{
    private:
        Trove* _trove;
        Aether::Text* _titleLabel = nullptr;
        Aether::Rectangle* _panel = nullptr;
        Aether::List* _list = nullptr;

        // std::function<void()> _onSave;

    public:
        Signal<bool>* onCloseSignal = nullptr;

        WidgetOverlay(const std::string &title="");
        ~WidgetOverlay();

        /**
         * @brief Clear the widget overlay's temporary element trove.
         */
        void clearTrove();

        /**
         * @brief Close the widget overlay.
         */
        void closeWidgetOverlay(bool isSave);

        /**
         * @brief Get the panel.
         */
        Aether::Rectangle* panel();

        /**
         * @brief Add an element that will be deleted when the overlay menu closes.
         */
        void addTempElement(Aether::Element* element);

        /**
         * @brief Add an Aether::List to the overlay. This is temporary and will be deleted when the overlay is closed.
         */
        void addList();

        /**
         * @brief Get the list. Will return nullptr if no list has been created with addList()
         */
        Aether::List* list();

        /**
         * @brief Set the title text content of the widget overlay.
         */
        void setTitleLabel(const std::string &title);
};