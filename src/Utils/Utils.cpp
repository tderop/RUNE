// lib specific
#include <Utils/Utils.hpp>

// system
#include <switch.h>
#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>
#include <sstream>

// functions
void log(const std::string& msg){
    // get the char string and buffer size
    const char* c_str     = msg.c_str();
    const size_t str_size = msg.size();

    svcOutputDebugString(c_str, str_size);
}

Result getUserID(AccountUid* out){
    // get the uid of the preselected user
    Result rc = accountGetPreselectedUser(out);

    if (R_FAILED(rc)){
        log("accountGetPreselectedUser() failed, using pselShowUserSelector...");

        // initialise player selector UI
        PselUserSelectionSettings settings{};
        rc = pselShowUserSelector(out, &settings);

        if (R_FAILED(rc)){
            log("pselShowUserSelector() failed\n");
            return rc;
        }
    }

    return 0;
}

/**
 * @brief Get the nickname of the user currently running the application if any
 * @return a default value of "None" for no user, or the nickname of the user
 */
std::string getUserNickname(){
    AccountUid userID={0};
    Result rc = getUserID(&userID);
    if (R_FAILED(rc)){
        return "None";
    }

    AccountProfile profile;
    AccountUserData userData;
    AccountProfileBase profileBase;

    char nickname[0x21];

    rc = accountGetProfile(&profile, userID);

    if (R_FAILED(rc)){
        log("accountGetProfile() failed\n");
    }
    else if (R_SUCCEEDED(rc)){
        rc = accountProfileGet(&profile, &userData, &profileBase);

        if (R_FAILED(rc)) {
            log("accountProfileGet() failed\n");
        }
        else if (R_SUCCEEDED(rc)){
            std::strncpy(nickname, profileBase.nickname, sizeof(nickname)-1); // obtain the null-terminated nickname
        }

        accountProfileClose(&profile);
    }
    
    return std::string(nickname);
}

std::string getDeviceNickname(){
    SetSysDeviceNickName nickname;

    // unpack the nickname
    Result rc = setGetDeviceNickname(&nickname);
    if (R_FAILED(rc)){
        log("[UTILS] - getDeviceNickname() failed.");
        return "NX";
    }

    return std::string(nickname.nickname);
}

std::string formatUnixTimestamp(int32_t unixTimestamp){
    time_t timeVal = static_cast<time_t>(unixTimestamp);
    tm* timeInfo = localtime(&timeVal);

    if (timeInfo == nullptr){
        return "Invalid Date";
    }

    std::stringstream ss;
    ss << std::put_time(timeInfo, "%d/%m/%Y %H:%M");

    return ss.str();
}

void clearLine(){
    printf("\x1b[2K\r");
    fflush(stdout);
}
