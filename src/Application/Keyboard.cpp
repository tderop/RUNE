#include <Application/Keyboard.hpp>
#include <string>

Result Rune::Keyboard::getStringInput(std::string* outStr, const std::string& hint){
    SwkbdConfig kbd;
    Result rc = swkbdCreate(&kbd, 0);
    if (R_FAILED(rc)){
        return rc;
    }

    swkbdConfigMakePresetDefault(&kbd);
    swkbdConfigSetGuideText(&kbd, hint.c_str());

    char buf[256] = {0}; // default to null terminated string
    rc = swkbdShow(&kbd, buf, sizeof(buf));
    if (R_FAILED(rc)){
        return rc;
    }

    swkbdClose(&kbd);

    *outStr = std::string(buf);
    return 0;
}

Result Rune::Keyboard::getNumberInput(float* outFloat, const std::string& hint, float minValue, float maxValue){
    SwkbdConfig kbd;
    Result rc = swkbdCreate(&kbd, 0);
    if (R_FAILED(rc)){
        return rc;
    }

    swkbdConfigSetType(&kbd, SwkbdType_NumPad);
    swkbdConfigSetGuideText(&kbd, hint.c_str());

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

    if (inputFloat < minValue || inputFloat > maxValue){
        return LibnxError_BadInput;
    }

    *outFloat = inputFloat;
    return 0;
}