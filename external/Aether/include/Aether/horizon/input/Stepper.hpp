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
            std::function<void()> onValueChangeFunc;
        
            /**
             * @brief Run updates such as changing the value string and running the value change function
             */
            void valueUpdated();

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
            * @param max The new max value
            */
           void setMax(int max);

           /**
            * @brief Set the minimum value of the stepper object
            * @param min The new min value
            */
           void setMin(int min);

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
    };
};

#endif