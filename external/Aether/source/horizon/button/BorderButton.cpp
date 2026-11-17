#include "Aether/horizon/button/BorderButton.hpp"

// Corner radius of rectangle
#define CORNER_RAD 8
// Horizontal padding for text
#define PADDING 20

namespace Aether {
    BorderButton::BorderButton(int x, int y, int w, int h, unsigned int b, std::string t, unsigned int s, std::function<void()> f) : Element(x, y, w, h) {
        this->box = new Box(this->x(), this->y(), this->w(), this->h(), b, CORNER_RAD);
        this->fill = new Rectangle(this->x(), this->y(), this->w(), this->h(), CORNER_RAD);
        this->fill->setColour(Aether::Colour{0, 0, 0, 0}); // Transparent by default
        this->addElement(fill);
        this->addElement(this->box);

        this->textSize = s;
        this->textContent = t;

        this->text = new Text(0, 0, "", 0);
        this->setString(t);
        this->text->setCanScroll(true);
        this->text->setScrollPause(1500);
        this->text->setScrollSpeed(40);
        this->resizeText();
        this->addElement(this->text);
        this->onPress(f);
    }

    void BorderButton::setFillColour(Colour c){
        this->fill->setColour(c);
    }

    void BorderButton::resizeText(){
        unsigned int fontSize = this->textSize + 1;

        std::pair<int, int> size;
        do {
            fontSize--;
            size = this->renderer->calculateTextDimensions(this->textContent, fontSize);
        }
        while ((size.first > this->w() - (PADDING*2) || size.second > this->h() - 4) && fontSize > 0);
        this->text->setFontSize(fontSize);
    }

    Colour BorderButton::getBorderColour() {
        return this->box->colour();
    }

    void BorderButton::setBorderColour(Colour c) {
        this->box->setColour(c);
    }

    Colour BorderButton::getTextColour() {
        return this->text->colour();
    }

    void BorderButton::setTextColour(Colour c) {
        this->text->setColour(c);
    }

    std::string BorderButton::getString() {
        return this->text->string();
    }

    void BorderButton::setString(std::string s) {
        this->text->setString(s);
        this->textContent = s;

        this->resizeText();
        if (this->text->textureWidth() > this->w() - 1.5*PADDING) {
            this->text->setW(this->w() - 2*PADDING);
        }
        this->text->setXY(this->x() + (this->w() - this->text->w())/2, this->y() + (this->h() - this->text->h())/2);
    }

    void BorderButton::setW(int w) {
        Element::setW(w);
        this->box->setBoxSize(this->w(), this->h());
        this->setString(this->text->string());
    }

    void BorderButton::setH(int h) {
        Element::setH(h);
        this->box->setBoxSize(this->w(), this->h());
        this->text->setY(this->y() + (this->h() - this->text->h())/2);
    }

    Drawable * BorderButton::renderHighlightBG() {
        return this->renderer->renderFilledRoundRectTexture(this->w(), this->h(), CORNER_RAD-4);
    }

    Drawable * BorderButton::renderHighlight() {
        return this->renderer->renderRoundRectTexture(this->w() + 2*(this->hiSize - this->box->thickness()), this->h() + 2*(this->hiSize - this->box->thickness()), CORNER_RAD + 2, this->hiSize);
    }

    Drawable * BorderButton::renderSelection() {
        return this->renderer->renderFilledRoundRectTexture(this->w(), this->h(), CORNER_RAD-4);
    }
};
