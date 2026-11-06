#define VALUE_CONTAINER_HEIGHT 48
#define INCREMENTOR_WIDTH 60

#define BORDER_WIDTH 4
#define BORDER_RADIUS 8

#define ELEMENT_SPACING 24
#define BORDER_COLOUR Aether::Colour{96, 96, 96, 150}
#define BACKGROUND_COLOUR Aether::Colour{35, 35, 35, 255}
#define MUTED_BACKGROUND Aether::Colour{20, 20, 20, 200}
#define TRANSPARENT Aether::Colour{0, 0, 0, 0}

#define TEXT_SIZE 20

#include <string>
#include <algorithm>

#include "Aether/horizon/input/Stepper.hpp"
#include "Aether/types/Colour.hpp"

namespace Aether {
    void Stepper::valueUpdated(){
        this->valueBtn->setString(std::to_string(this->value_));
        if (this->onValueChangeFunc){
            this->onValueChangeFunc();
        }
    }

    void Stepper::increment(){
        this->setValue(std::min(this->value_+this->step, this->maxValue));
    }

    void Stepper::decrement(){
        this->setValue(std::max(this->value_-this->step, this->minValue));
    }

    Stepper::Stepper(int x, int y, std::string s, int w){
        this->label = new Text(x, y, s, TEXT_SIZE);
        this->valueBtn = new BorderButton(x, y, w, VALUE_CONTAINER_HEIGHT, BORDER_WIDTH, "", 24, [](){});
        if (!s.empty()){
        this->valueBtn->setX(x+this->label->textureWidth()+(ELEMENT_SPACING*2));
            this->label->setY(y+this->valueBtn->h()/2-this->label->textureHeight()/2);
        }

        this->decrementBtn = new BorderButton(
            this->valueBtn->x()+this->valueBtn->w()+ELEMENT_SPACING, y, 
            INCREMENTOR_WIDTH, VALUE_CONTAINER_HEIGHT, 
            BORDER_WIDTH*2, 
            "—", TEXT_SIZE, 
            [this](){
                this->decrement();
            }
        );
        this->incrementBtn = new BorderButton(
            this->decrementBtn->x()+this->decrementBtn->w(), y,
            INCREMENTOR_WIDTH, VALUE_CONTAINER_HEIGHT,
            BORDER_WIDTH*2,
            "+", 24,
            [this](){
                this->increment();
            }
        );
        this->stepBtnBorder = new Box(
            this->decrementBtn->x(), y,
            INCREMENTOR_WIDTH*2, VALUE_CONTAINER_HEIGHT,
            BORDER_WIDTH, BORDER_RADIUS
        );
        this->stepBtnBorderBackground = new Rectangle(
            this->stepBtnBorder->x(), this->stepBtnBorder->y(),
            this->stepBtnBorder->w(), this->stepBtnBorder->h(),
            BORDER_RADIUS
        );
        this->stepBtnBorderBackground->setColour(BACKGROUND_COLOUR);
        this->valueBtn->setFillColour(MUTED_BACKGROUND);

        this->valueBtn->setBorderColour(BORDER_COLOUR);
        this->stepBtnBorder->setColour(BORDER_COLOUR);
        this->incrementBtn->setBorderColour(TRANSPARENT);
        this->decrementBtn->setBorderColour(TRANSPARENT);
        
        this->addElement(this->label);
        this->addElement(this->valueBtn);
        this->addElement(this->stepBtnBorderBackground);
        this->addElement(this->incrementBtn);
        this->addElement(this->decrementBtn);
        this->addElement(this->stepBtnBorder);

        this->valueUpdated();
    }

    void Stepper::setMax(int max){
        this->maxValue = max;
    }

    void Stepper::setMin(int min){
        this->minValue = min;
    }

    void Stepper::setValue(int v){
        this->value_ = v;
        valueUpdated();
    }

    void Stepper::setStep(int step){
        this->step = step;
    }

    void Stepper::onValueChange(std::function<void()> f){
        this->onValueChangeFunc = f;
    }

    int Stepper::value(){
        return this->value_;
    }
}