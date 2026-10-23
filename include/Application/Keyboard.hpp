#include <switch.h>
#include <string>

namespace Rune{
    class Keyboard{
        private:
            

        public:
            /**
             * @brief Show the keyboard, requesting a string input from the user
             * @param str hint showed to the user in the empty keyboard text box
             * @param str where the input should be stored
             * 
             * @return the result (whether input succeeded or errored)
             */
            static Result getStringInput(std::string* outStr, const std::string& hint="");

            /** 
             * @brief Show the keyboard, requesting a numeric input from the user
             * @param str hint showed to the user in the empty keyboard text box
             * @param float where the input should be stored
             * 
             * @return the result (whether input succeeded or errored)
            */
           static Result getNumberInput(float* outFloat, const std::string& hint="", float minValue = -__FLT_MAX__, float maxValue = __FLT_MAX__, bool showDecimal=false, bool showMinus=true);

    }; // namespace Keyboard
} // namespace Rune