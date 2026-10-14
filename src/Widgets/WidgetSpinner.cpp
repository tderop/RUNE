#include <Widgets/WidgetSpinner.hpp>

bool WidgetSpinner::handleEvent(Aether::InputEvent* event){
    // determines if on press should run
    const bool activate = (this->highlighted()) && // must be highlighted

    (((event->type() == Aether::EventType::ButtonReleased) &&
    (event->button() == Aether::Button::A)) ||  // must press A OR

    (event->type() == Aether::EventType::TouchReleased)); // must release touch

    // run base event handler
    const bool handled = Aether::Spinner::handleEvent(event);

    if (activate && this->_onPress) {
        _onPress();
        return true;
    }

    return handled;
}