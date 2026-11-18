#include <Application/Keyboard.hpp>
#include <Application/Audio.hpp>

#include <Utils/Logger.hpp>
#include <string>

Result Rune::Keyboard::getStringInput(std::string* outStr, const std::string& hint){
    SwkbdConfig kbd;
    Result rc = swkbdCreate(&kbd, 0);
    if (R_FAILED(rc)){
        return rc;
    }

    swkbdConfigMakePresetDefault(&kbd);
    swkbdConfigSetGuideText(&kbd, hint.c_str());

    Audio::playSound("keyboard");

    char buf[256] = {0}; // default to null terminated string
    rc = swkbdShow(&kbd, buf, sizeof(buf));

    if (R_FAILED(rc)){
        return rc;
    }

    swkbdClose(&kbd);

    *outStr = std::string(buf);
    return 0;
}

Result Rune::Keyboard::getNumberInput(float* outFloat, const std::string& hint, float minValue, float maxValue, bool showDecimal, bool showMinus){
    SwkbdConfig kbd;
    Result rc = swkbdCreate(&kbd, 0);
    if (R_FAILED(rc)){
        return rc;
    }

    swkbdConfigSetType(&kbd, SwkbdType_NumPad);
    swkbdConfigSetGuideText(&kbd, hint.c_str());
    swkbdConfigSetBlurBackground(&kbd, 1); // 1 is the default value used by swkbdMakePresetDefault()

    // Set left and right keys
    char minus = '-';
    char decimal = '.';

    if (showMinus){
        swkbdConfigSetLeftOptionalSymbolKey(&kbd, &minus);
    }

    if (showDecimal){
        swkbdConfigSetRightOptionalSymbolKey(&kbd, &decimal);
    }

    Audio::playSound("keyboard");

    char buf[16] = {0};
    rc = swkbdShow(&kbd, buf, sizeof(buf));
    if (R_FAILED(rc)){
        return rc;
    }

    swkbdClose(&kbd);

    std::string input = std::string(buf);
    char* endptr = nullptr;

    float inputFloat = std::strtof(input.c_str(), &endptr);
    if (endptr == input.c_str()){
        // failed to convert
        return LibnxError_BadInput;
    }

    *outFloat = inputFloat; // Still allow reading the result
    if (inputFloat < minValue || inputFloat > maxValue){
        Logger::Write("[Rune::Keyboard::getNumberInput] - Error: Input value is OOB ("+std::to_string(minValue)
            +", "+std::to_string(maxValue)+")");
        return LibnxError_BadInput;
    }

    return 0;
}