#define VALUE_CONTAINER_HEIGHT 48
#define INCREMENTOR_WIDTH 60

#define BORDER_WIDTH 3
#define BORDER_RADIUS 8

#define ELEMENT_SPACING 24
#define BORDER_COLOUR Aether::Colour{96, 96, 96, 150}
#define BACKGROUND_COLOUR Aether::Colour{35, 35, 35, 255}
#define MUTED_BACKGROUND Aether::Colour{20, 20, 20, 200}
#define TRANSPARENT Aether::Colour{0, 0, 0, 0}

#define TEXT_SIZE 20
#define TEXT_SPACING 142

#include <string>
#include <algorithm>

#include "Aether/horizon/input/Stepper.hpp"
#include "Aether/types/Colour.hpp"

namespace Aether {
    void Stepper::valueUpdated(){
        this->valueBtn->setString(std::to_string(this->value_));
        if (this->onValueChangeFunc_){
            this->onValueChangeFunc_();
        }
    }

    void Stepper::reorganiseElements(){
        if (!this->label->string().empty()){
            this->valueBtn->setXY(
                this->origin->x()+TEXT_SPACING,
                this->origin->y()
            );
            this->label->setXY(
                origin->x(),
                this->valueBtn->y()+this->valueBtn->h()/2-this->label->textureHeight()/2
            );
        } else {
            this->valueBtn->setXY(
                this->origin->x(),
                this->origin->y()
            );
        }

        this->decrementBtn->setXY(
            this->valueBtn->x()+this->valueBtn->w()+ELEMENT_SPACING,
            this->valueBtn->y()
        );
        this->incrementBtn->setXY(
            this->decrementBtn->x()+this->decrementBtn->w(),
            this->valueBtn->y()
        );
        this->stepBtnBorder->setXY(
            this->decrementBtn->x(),
            this->valueBtn->y()
        );
        this->stepBtnBorderBackground->setXY(
            this->stepBtnBorder->x(),
            this->stepBtnBorder->y()
        );
    }

    void Stepper::increment(){
        this->setValue(std::min(this->value_+this->step, this->maxValue));
    }

    void Stepper::decrement(){
        this->setValue(std::max(this->value_-this->step, this->minValue));
    }

    Stepper::Stepper(int x, int y, std::string s, int w){
        this->origin = new Container(x, y);

        this->label = new Text(x, y, s, TEXT_SIZE);
        this->valueBtn = new BorderButton(x, y, w, VALUE_CONTAINER_HEIGHT, BORDER_WIDTH, "", 24, [](){});

        if (!s.empty()){
            this->label->setY(y+this->valueBtn->h()/2-this->label->textureHeight()/2);
            this->valueBtn->setX(x+TEXT_SPACING);
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
        
        this->addElement(origin);
        this->origin->addElement(this->label);
        this->origin->addElement(this->valueBtn);
        this->origin->addElement(this->stepBtnBorderBackground);
        this->origin->addElement(this->incrementBtn);
        this->origin->addElement(this->decrementBtn);
        this->origin->addElement(this->stepBtnBorder);

        this->valueUpdated();
    }

    void Stepper::setMax(int max){
        if (max < this->minValue){
            return;
        }

        this->maxValue = max;
    }

    void Stepper::setMin(int min){
        if (min > this->maxValue){
            return;
        }

        this->minValue = min;
    }

    int Stepper::max(){
        return this->maxValue;
    }

    int Stepper::min(){
        return this->minValue;
    }

    void Stepper::setValue(int v){
        this->value_ = v;
        valueUpdated();
    }

    void Stepper::setStep(int step){
        this->step = step;
    }

    void Stepper::onValueChange(std::function<void()> f){
        this->onValueChangeFunc_ = f;
    }

    void Stepper::onPress(std::function<void()> f){
        this->onPressFunc_ = f;
        this->valueBtn->onPress(this->onPressFunc_);
    }

    int Stepper::value(){
        return this->value_;
    }

    int Stepper::x(){
        return this->origin->x();
    }

    int Stepper::y(){
        return this->origin->y();
    }

    int Stepper::w(){
        return this->valueBtn->w();
    }

    int Stepper::h(){
        return this->valueBtn->h();
    }

    void Stepper::setX(int x){
        this->origin->setX(x);
    }

    void Stepper::setY(int y){
        this->origin->setY(y);
    }

    void Stepper::setXY(int x, int y){
        this->origin->setXY(x, y);
    }

    void Stepper::setW(int w){
        this->valueBtn->setW(w);
        this->reorganiseElements();
    }

    void Stepper::setH(int h){
        this->valueBtn->setH(h);
        this->incrementBtn->setH(h);
        this->decrementBtn->setH(h);
        this->stepBtnBorder->setH(h);
        this->stepBtnBorderBackground->setH(h);

        this->reorganiseElements();
    }

    void Stepper::setWH(int w, int h){
        this->setW(w);
        this->setH(h);
    }

    void Stepper::setXYWH(int x, int y, int w, int h){
        this->setX(x);
        this->setY(y);
        this->setW(w);
        this->setH(h);
    }

    void Stepper::setString(std::string s){
        this->label->setString(s);
        this->reorganiseElements();
    }

    std::string Stepper::getString(){
        return this->label->string();
    }
}