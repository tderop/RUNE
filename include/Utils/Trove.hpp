#pragma once

#include <vector>
#include <variant>
#include <functional>

#include <Aether/Aether.hpp>

/**
 * @brief bruh?
 */
using AetherElement = std::variant<
    Aether::Element*,

    Aether::Animation*,
    Aether::AsyncItem*,

    Aether::BaseProgress*,
    Aether::ProgressBar*,
    Aether::RoundProgressBar*,

    Aether::BorderButton*,

    Aether::Container*,
    Aether::ControlBar*,
    Aether::ListHeadingHelp*,
    Aether::Spinner*,
    Aether::Stepper*,
    Aether::Screen*,
    Aether::Overlay*,
    Aether::DateTime*,
    Aether::MessageBox*,
    Aether::PopupList*,
    Aether::Scrollable*,
    Aether::List*,
    Aether::Menu*,

    
    // Aether::ControlItem*,
    Aether::FilledButton*,
    Aether::HelpButton*,
    Aether::ListButton*,
    Aether::ListComment*,
    Aether::ListHeading*,
    Aether::ListOption*,
    Aether::ListSeparator*,
    Aether::MenuOption*,
    Aether::MenuSeparator*,

    Aether::Texture*,
    Aether::BaseText*,
    Aether::Box*,
    Aether::Ellipse*,
    Aether::Image*,
    Aether::Rectangle*,

    Aether::Text*,
    Aether::TextBlock*,

    Aether::Tick*
>;

/**
 * @brief An object for cleaning up Aether elements more efficiently.
 */
class Trove{
    private:
        std::vector<AetherElement> elements;
        std::vector<std::function<void()>> tasks;

    public:
        /**
         * @brief Create a new element trove.
         */
        Trove();
        ~Trove();

        /**
         * @brief Add an element to the trove.
         */
        void addElement(AetherElement element);

        /**
         * @brief Give the trove a task to complete when it is cleared.
         */
        void giveTask(std::function<void()> task);

        /**
         * @brief Remove all elements in the trove.
         */
        void clear();
};