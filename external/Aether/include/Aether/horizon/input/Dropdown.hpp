#ifndef AETHER_DROPDOWN_HPP
#define AETHER_DROPDOWN_HPP

#include <optional>
#include <string>
#include <map>
#include <any>

#include "Aether/base/Container.hpp"
#include "Aether/primary/Text.hpp"
#include "Aether/horizon/button/BorderButton.hpp"
#include "Aether/horizon/list/List.hpp"

namespace Aether {
    class Dropdown : public Container {
        private:
            /** @brief The text label next to the dropdown button */
            Text * label_;
            /** @brief The dropdown button */
            BorderButton * dropdownBtn_;

            /** @brief The current value of the dropdown */
            std::string value_;
            /** @brief The current data map */
            std::map<std::string, std::any> map_;

            /** @brief The currently showing background rectangle in the dropdown */
            Rectangle * backgroundRect_=nullptr;
            /** @brief The list showing the dropdown options */
            List* list_=nullptr;
            
            /** @brief The function to run when the value is changed */
            std::function<void()> onValueChangeFunc_;
            /** @brief The function to run when the dropdown button is pressed */
            std::function<void(bool)> onPressFunc_;

            /** @brief The height of the dropdown menu */
            int dropdownH_;

            /** @brief The current state of the dropdown menu */
            bool dropdownVisible_=false;

            /** @brief Show the dropdown menu */
            void showDropdown();

            /** @brief Hide the dropdown menu */
            void hideDropdown();

            /** @brief To be run once the value has been updated */
            void valueUpdated();

            /** @brief Update the position of elements */
            void reorganiseElements();

        public: 
            /**
             * @brief Create a new dropdown
             * @param x The x-coordinate of the dropdown
             * @param y The y-coordinate of the dropdown
             * @param s The text to set the label to
             * @param w The width of the dropdown button
             * @param h The height of the dropdown menu
             */
            Dropdown(int x, int y, std::string s="", int w = 180, int h = 240);

            /**
             * @brief Set the function to run when the value is changed
             * @param f The function to run when value is changed
             */
            void onValueChange(std::function<void()> f);

            /**
             * @brief Set the function to run when the dropdown button is pressed
             * @param f The function to run
             */
            void onPress(std::function<void(bool isOpen)> f);

            /**
             * @brief Return the list in the dropdown
             * @note This goes stale every time the dropdown is closed
             */
            List* list();


            /**
             * @brief Set the dropdown data
             * @note first: string shown in dropdown menu, second: value used by selection callback
             * @param map The data map to be used by the dropdown
             */
            void setMap(std::map<std::string, std::any> map);

            /**
             * @brief Set the string on the label
             * @param s The string to set the label to
             */
            void setString(std::string s);

            /**
             * @brief Get the string on the label
             * @return The string on the label
             */
            std::string getString();

            /**
             * @brief Set the string value of the dropdown
             * @param s The new string
             */
            void setValue(std::string s);

            /**
             * @brief Get the string value of the dropdown
             * @return The value string
             */
            std::string getValue();

            /** 
             * @brief Get the data stored in the dropdown menu
             * @return The stored data
             */
            template <typename T>
            std::optional<T> getValueData(){
                if (this->map_.empty()){
                    return std::nullopt;
                }

                auto it = this->map_.find(this->value_);
                if (it == this->map_.end()){
                    return std::nullopt;
                }

                // return std::any_cast<T>(it->second);
                auto* valuePtr = std::any_cast<T>(&it->second);
                if (valuePtr != nullptr) {
                    return *valuePtr;
                }

                return std::nullopt;
            }

    };
} // namespace Aether

#endif