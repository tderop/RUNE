#include "Aether/horizon/input/Dropdown.hpp"

#define TEXT_SPACING 142
#define TEXT_SIZE 20

#define BUTTON_HEIGHT 48
#define BORDER_WIDTH 3
#define BORDER_RADIUS 8

#define DROPDOWN_OPTION_HEIGHT 42
#define DROPDOWN_FONT_SIZE 16

#define MUTED_BACKGROUND Aether::Colour{20, 20, 20, 200}
#define DROPDOWN_BACKGROUND_COLOUR Aether::Colour{15, 15, 15, 220}
#define BORDER_COLOUR Aether::Colour{96, 96, 96, 150}
#define LINE_COLOUR Aether::Colour{96, 96, 96, 150}

namespace Aether {
    void Dropdown::showDropdown(){
        if (this->dropdownVisible_){
            return;
        }

        this->backgroundRect_ = new Rectangle(
            this->dropdownBtn_->x(), this->dropdownBtn_->y()+this->dropdownBtn_->h()+BORDER_WIDTH,
            this->dropdownBtn_->w(), this->dropdownH_,
            BORDER_RADIUS
        );
        this->backgroundRect_->setSelectable(false);
        this->backgroundRect_->setColour(DROPDOWN_BACKGROUND_COLOUR);
        this->addElement(this->backgroundRect_);

        this->list_ = new List(
            this->backgroundRect_->x(), this->backgroundRect_->y(),
            this->backgroundRect_->w(), this->backgroundRect_->h(),
            Aether::Padding::None
        );
        this->list_->setScrollBarColour(LINE_COLOUR);
        this->addElement(this->list_);

        bool firstOffset=false;
        for (const auto&[entryName, _] : this->map_){
            ListButton* btn = new ListButton(
                entryName, [this, entryName](){
                    this->value_=entryName;
                    this->hideDropdown();
                    this->setValue(entryName);
                }
            );
            btn->setLineColour(LINE_COLOUR);
            btn->setFontSize(DROPDOWN_FONT_SIZE);

            this->list_->addElement(btn);
            btn->setH(DROPDOWN_OPTION_HEIGHT);
            
            if (!firstOffset){
                firstOffset=true;
                btn->setY(this->list_->y()+8);
            }
        }

        this->dropdownVisible_ = true;
    }

    void Dropdown::hideDropdown(){
        if (!this->dropdownVisible_){
            return;
        }

        if (this->backgroundRect_ != nullptr){
            this->removeElement(this->backgroundRect_);
            this->backgroundRect_ = nullptr;
        }

        if (this->list_ != nullptr){
            this->list_->removeAllElements();
            this->removeElement(this->list_);
            this->list_ = nullptr;
        }

        this->dropdownVisible_ = false;
    }

    void Dropdown::valueUpdated(){
        this->dropdownBtn_->setString(this->value_);
    }

    void Dropdown::reorganiseElements(){
        if (!this->label_->string().empty()){ 
            this->dropdownBtn_->setX(this->label_->x()+TEXT_SPACING); 
            this->label_->setY(this->dropdownBtn_->y()+this->dropdownBtn_->h()/2-this->label_->textureHeight()/2);
        }
    }

    Dropdown::Dropdown(int x, int y, std::string s, int w, int h){
        this->dropdownH_=h;
        this->setXY(x, y);
        this->setH(BUTTON_HEIGHT);

        this->label_ = new Aether::Text(
            x, y, 
            s, TEXT_SIZE
        );
        this->dropdownBtn_ = new Aether::BorderButton(
            x, y, 
            w, BUTTON_HEIGHT,
            BORDER_WIDTH, "", TEXT_SIZE,
            [this](){
                if (this->dropdownVisible_){
                    this->hideDropdown();
                } else {
                    this->showDropdown();
                }
            }
        );
        this->dropdownBtn_->setFillColour(MUTED_BACKGROUND);
        this->dropdownBtn_->setBorderColour(BORDER_COLOUR);
        this->reorganiseElements();

        this->addElement(this->label_);
        this->addElement(this->dropdownBtn_);
    }

    void Dropdown::setMap(std::map<std::string, std::any> map){
        this->map_ = map;
    }

    void Dropdown::setString(std::string s){
        this->label_->setString(s);
        this->reorganiseElements();
    }

    std::string Dropdown::getString(){
        return this->label_->string();
    }

    void Dropdown::setValue(std::string s){
        this->value_ = s;
        this->valueUpdated();
    }

    std::string Dropdown::getValue(){
        return this->value_;
    }
} // namespace Aether