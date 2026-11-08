#ifndef AETHER_STEPPER_HPP
#define AETHER_STEPPER_HPP

#include <functional>
#include <string>

#include "Aether/base/Container.hpp"
#include "Aether/horizon/button/BorderButton.hpp"
#include "Aether/primary/Text.hpp"
#include "Aether/utils/Types.hpp"

namespace Aether {
    class Stepper : public Container {
        private:
            /** @brief The label description of the stepper */
            Text * label;

            /** @brief The button that shows the value of the stepper */
            BorderButton * valueBtn;
            /** @brief The decrement (-) button */
            BorderButton * decrementBtn;
            /** @brief The increment (+) button */
            BorderButton * incrementBtn;
            
            /** @brief The box that outlines the increment and decrement buttons */
            Box * stepBtnBorder;
            /** @brief The background to the box declared above */
            Rectangle * stepBtnBorderBackground;

            /** @brief The value of the stepper */
            int value_ = 0;
            /** @brief The step */
            int step = 1;
            /** @brief The minimum value of the stepper */
            int minValue = 0;
            /** @brief The maximum value of the stepper */
            int maxValue = __INT32_MAX__;

            /** @brief The function to run when the value of the stepper is changed */
            std::function<void()> onValueChangeFunc_;
            /** @brief The function to run when the stepper is pressed */
            std::function<void()> onPressFunc_;
        
            /**
             * @brief Run updates such as changing the value string and running the value change function
             */
            void valueUpdated();

            /**
             * @brief Run visual updates to reorganise elements with the correct relative positional offsets
             */
            void reorganiseElements();

            /**
             * @brief Increment the value by step
             */
            void increment();

            /**
             * @brief Decrement the value by step
             */
            void decrement();

        public:
            /**
             * @brief Construct a new stepper object
             * @note Value is initialised to 0
             * 
             * @param x x coordinate of stepper
             * @param y y coordinate of stepper
             * @param s (optional) the string to give the stepper
             * @param w (optional) width of stepper text box
             */
           Stepper(int x, int y, std::string s="", int w = 180);

           /**
            * @brief Set the maximum value of the stepper object
            * @note Fails if new max is lower than current min value
            * @param max The new max value
            */
           void setMax(int max);

           /**
            * @brief Set the minimum value of the stepper object
            * @note Fails if new min is larger than current max value
            * @param min The new min value
            */
           void setMin(int min);

           void setMinMax(int min, int max);

           /**
            * @brief Get the maximum value
            * @return The max value the stepper can hold
            */
           int max();

           /**
            * @brief Get the minimum value
            * @return The min value the stepper can hold
            */
           int min();

           /**
            * @brief Set the value of the stepper object
            * @param v The new value
            */
           void setValue(int v);

           /**
            * @brief Set the step that the increment buttons should increment by
            * @param step The new step value
            */
           void setStep(int step);

           /**
            * @brief Set the function to run when the value of the stepper is changed
            * @param f The function to run
            */
           void onValueChange(std::function<void()> f);

           /**
            * @brief Set the function to run when the stepper is pressed
            * @param f The function to run
            */
           void onPress(std::function<void()> f);

           /**
            * @brief Get the current value of the stepper
            * @return The integer value of the stepper
            */
           int value();

           /**
            * @brief Get the width of the stepper's text button
            * @return width of stepper
            */
           int w() override;

           /**
            * @brief Get the height of the stepper's text button
            * @return height of stepper
            */
           int h() override;

           /**
            * @brief Set the width of the stepper's text button
            * @param w width of the element
            */
           void setW(int w) override;

           /**
            * @brief Set the height of the stepper's elements
            * @param h height of the element
            */
           void setH(int h) override;

            /**
             * @brief Set the width and the height of the stepper
             * @param w width of the element
             * @param h height of the element
             */
            void setWH(int w, int h) override;

            /**
             * @brief Set all dimensions of the stepper
             * @param x x-coordinate of the element
             * @param y y-coordinate of the element
             * @param w width of the element
             * @param h height of the element
             */
            void setXYWH(int x, int y, int w, int h) override;

            /**
             * @brief Set the label string
             * @param s The new string
             */
            void setString(std::string s);

            /**
             * @brief Get the label string
             * @return The label string
             */
            std::string getString();
    };
};

#endif