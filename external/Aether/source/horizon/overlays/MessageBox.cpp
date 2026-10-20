#include "Aether/horizon/overlays/MessageBox.hpp"

// Default dimensions
#define WIDTH 770
#define HEIGHT 400

// Button height
#define BUTTON_HEIGHT 70

namespace Aether {
    MessageBox::MessageBox(const std::string& message) {
        this->rect_ = new Rectangle(640 - WIDTH/2, 360 - HEIGHT/2, WIDTH, HEIGHT, 8);
        this->addElement(this->rect_);

        this->body = nullptr;
        this->left = nullptr;
        this->right = nullptr;
        this->top = nullptr;
        this->topH = new Rectangle(this->rect_->x(), this->rect_->y(), this->rect_->w(), 2);
        this->bottomH = new Rectangle(this->rect_->x(), this->rect_->y(), this->rect_->w(), 2);
        this->vertH = new Rectangle(this->rect_->x() + this->rect_->w()/2, this->rect_->y() + this->rect_->h() - BUTTON_HEIGHT, 2, BUTTON_HEIGHT);
        this->topH->setHidden(true);
        this->vertH->setHidden(true);
        this->addElement(this->topH);
        this->addElement(this->bottomH);
        this->addElement(this->vertH);

        this->label_ = new Aether::Text(0, 0, "", 24);
        this->addElement(this->label_);
        this->setString(message);

        // Pressing b just closes the box
        this->onButtonPress(Button::B, [this](){
            this->close();
        });
    }

    BorderButton* MessageBox::leftButton(){
        return this->left;
    }

    BorderButton* MessageBox::rightButton(){
        return this->right;
    }

    BorderButton* MessageBox::topButton(){
        return this->top;
    }

    Rectangle* MessageBox::rect(){
        return this->rect_;
    }

    void MessageBox::repositionButtons() {
        bool twoRows = (this->top != nullptr && (this->left != nullptr || this->right != nullptr));
        this->bottomH->setY(this->rect_->y() + this->rect_->h() - BUTTON_HEIGHT - 2);
        if (this->top != nullptr) {
            if (twoRows) {
                this->top->setXY(this->rect_->x(), this->rect_->y() + this->rect_->h() - 2*BUTTON_HEIGHT - 1);
                this->topH->setY(this->rect_->y() + this->rect_->h() - 2*BUTTON_HEIGHT - 3);
            } else {
                this->top->setXY(this->rect_->x(), this->rect_->y() + this->rect_->h() - BUTTON_HEIGHT);
                this->topH->setY(this->rect_->y() + this->rect_->h() - BUTTON_HEIGHT - 2);
            }
        }

        if (this->left != nullptr) {
            this->left->setXY(this->rect_->x(), this->rect_->y() + this->rect_->h() - BUTTON_HEIGHT);
            this->vertH->setX(this->rect_->x() + this->rect_->w()/2 - 1);
        }

        if (this->right != nullptr) {
            this->right->setXY(this->rect_->x() + this->rect_->w()/2 + 1, this->rect_->y() + this->rect_->h() - BUTTON_HEIGHT);
            this->vertH->setX(this->rect_->x() + this->rect_->w()/2 - 1);
        }
    }

    void MessageBox::setString(const std::string& text){
        this->label_->setString(text);

        Aether::BorderButton* btn = nullptr;
        if (this->left != nullptr){
            btn = this->left;
        } else if (this->right != nullptr){
            btn = this->right;
        } else if (this->top != nullptr) {
            btn = this->top;
        }
        
        int containerHeight = this->rect_->h();
        if (btn != nullptr){
            containerHeight -= btn->h();
        }

        this->label_->setXY(
            this->rect_->x() + this->rect_->w()/2 - this->label_->textureWidth()/2,
            this->rect_->y() + containerHeight/2 - this->label_->textureHeight()/2
        );
    }

    void MessageBox::resizeElements() {
        this->topH->setRectSize(this->rect_->w() - 1, 2);
        this->bottomH->setRectSize(this->rect_->w() - 1, 2);
        if (this->left != nullptr) {
            this->left->setW(this->rect_->w()/2);
        }
        if (this->right != nullptr) {
            this->right->setW(this->rect_->w()/2 - 1);
        }
        if (this->top != nullptr) {
            this->top->setW(this->rect_->w());
        }
    }

    void MessageBox::setLineColour(Colour c) {
        this->topH->setColour(c);
        this->bottomH->setColour(c);
        this->vertH->setColour(c);
    }

    void MessageBox::setRectangleColour(Colour c) {
        this->rect_->setColour(c);
    }

    void MessageBox::setTextColour(Colour c) {
        if (this->left != nullptr) {
            this->left->setTextColour(c);
        }
        if (this->right != nullptr) {
            this->right->setTextColour(c);
        }
        if (this->top != nullptr) {
            this->top->setTextColour(c);
        }
        this->buttonColour = c;
    }

    void MessageBox::addLeftButton(std::string s, std::function<void()> f) {
        this->left = new BorderButton(0, 0, this->rect_->w()/2, BUTTON_HEIGHT, 2, s, 26, f);
        this->left->setBorderColour(Colour{255, 255, 255, 0});
        this->left->setTextColour(this->buttonColour);
        this->addElement(this->left);
        this->vertH->setHidden(false);
        this->repositionButtons();
    }

    void MessageBox::addRightButton(std::string s, std::function<void()> f) {
        this->right = new BorderButton(0, 0, this->rect_->w()/2 - 1, BUTTON_HEIGHT, 2, s, 26, f);
        this->right->setBorderColour(Colour{255, 255, 255, 0});
        this->right->setTextColour(this->buttonColour);
        this->addElement(this->right);
        this->vertH->setHidden(false);
        this->repositionButtons();
    }

    void MessageBox::addTopButton(std::string s, std::function<void()> f) {
        this->top = new BorderButton(0, 0, this->rect_->w(), BUTTON_HEIGHT, 2, s, 26, f);
        this->top->setBorderColour(Colour{255, 255, 255, 0});
        this->top->setTextColour(this->buttonColour);
        this->addElement(this->top);
        this->topH->setHidden(false);
        this->repositionButtons();
    }

    void MessageBox::getBodySize(int * w, int * h) {
        *w = this->rect_->w();
        *h = this->rect_->h();
        if (this->top != nullptr) {
            *h -= BUTTON_HEIGHT;
        }
        if (this->left != nullptr || this->right != nullptr) {
            *h -= BUTTON_HEIGHT;
        }
    }

    void MessageBox::setBodySize(int w, int h) {
        if (this->top != nullptr) {
            h += BUTTON_HEIGHT;
        }
        if (this->left != nullptr || this->right != nullptr) {
            h += BUTTON_HEIGHT;
        }
        this->rect_->setRectSize(w, h);
        this->rect_->setXY(640 - this->rect_->w()/2, 360 - this->rect_->h()/2);
        if (this->body != nullptr) {
            this->body->setXY(this->rect_->x(), this->rect_->y());
        }
        this->topH->setX(this->rect_->x());
        this->bottomH->setX(this->rect_->x());
        this->vertH->setY(this->rect_->y() + this->rect_->h() - BUTTON_HEIGHT);
        this->resizeElements();
        this->repositionButtons();
    }

    void MessageBox::emptyBody() {
        if (this->body != nullptr) {
            this->removeElement(this->body);
        }
        this->body = nullptr;
    }

    void MessageBox::setBody(Element * e) {
        if (this->body != nullptr) {
            return;
        }

        this->body = e;
        this->body->setXY(this->rect_->x(), this->rect_->y());
        this->addElement(this->body);
    }
};
