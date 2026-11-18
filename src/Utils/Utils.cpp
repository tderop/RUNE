// lib specific
#include <Utils/Utils.hpp>
#include <Config.hpp>
#include <Aether/Aether.hpp>
#include <Application/Application.hpp>
#include <Utils/Logger.hpp>

// system
#include <switch.h>
#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <functional>
#include <algorithm>
#include <chrono>
#include <cstdio>

Result getUserID(AccountUid* out){
    // get the uid of the preselected user
    Result rc = accountGetPreselectedUser(out);

    if (R_FAILED(rc)){
        Logger::Write("accountGetPreselectedUser() failed, using pselShowUserSelector...");

        // initialise player selector UI
        PselUserSelectionSettings settings{};
        rc = pselShowUserSelector(out, &settings);

        if (R_FAILED(rc)){
            Logger::Write("pselShowUserSelector() failed\n");
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
        Logger::Write("accountGetProfile() failed\n");
    }
    else if (R_SUCCEEDED(rc)){
        rc = accountProfileGet(&profile, &userData, &profileBase);

        if (R_FAILED(rc)) {
            Logger::Write("accountProfileGet() failed\n");
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
        Logger::Write("[UTILS] - getDeviceNickname() failed.");
        return "NX";
    }

    return std::string(nickname.nickname);
}

std::string formatUnixTimestamp(int32_t unixTimestamp){
    TimeCalendarTime calendarTime;
    TimeCalendarAdditionalInfo info;

    if (R_FAILED(timeToCalendarTimeWithMyRule(unixTimestamp, &calendarTime, &info))){
        return "???";
    }

    u64 langCode = 0;
    SetLanguage language = SetLanguage_ENGB;

    if (R_SUCCEEDED(setInitialize())){
        if (R_SUCCEEDED(setGetSystemLanguage(&langCode))){
            setMakeLanguage(langCode, &language);
        }

        setExit();
    }

    char buf[22];
    switch (language){
        case SetLanguage_JA:
        case SetLanguage_KO:
        case SetLanguage_ZHCN:
        case SetLanguage_ZHTW:
        case SetLanguage_ZHHANS:
        case SetLanguage_ZHHANT: {
            std::snprintf(buf, sizeof(buf), "%04u/%02u/%02u %02u:%02u", calendarTime.year, calendarTime.month, calendarTime.day, calendarTime.hour, calendarTime.minute);
            break;
        }

        case SetLanguage_ENUS:
        case SetLanguage_FRCA:
        case SetLanguage_ES419: {
            std::snprintf(buf, sizeof(buf), "%02u/%02u/%04u %02u:%02u", calendarTime.month, calendarTime.day, calendarTime.year, calendarTime.hour, calendarTime.minute);
            break;
        }

        case SetLanguage_ENGB:
        case SetLanguage_FR:
        case SetLanguage_DE:
        case SetLanguage_IT:
        case SetLanguage_ES:
        case SetLanguage_NL:
        case SetLanguage_PT:
        case SetLanguage_PTBR:
        case SetLanguage_RU: {
            std::snprintf(buf, sizeof(buf), "%02u/%02u/%04u %02u:%02u", calendarTime.day, calendarTime.month, calendarTime.year, calendarTime.hour, calendarTime.minute);
            break;
        }

        default:
            break;
    }

    return std::string(buf);
}

void clearLine(){
    printf("\x1b[2K\r");
    fflush(stdout);
}

std::function<Aether::Colour(uint32_t)> highlightFunc() {
    Aether::Colour h1 = TOP_R;
    Aether::Colour h2 = BOTTOM_R;
    return [h1, h2](uint32_t t) {
        Aether::Colour col;
        col.setR(h1.r() + ((h2.r() - h1.r()) * (0.5 * sin(1.8 * 3.14 * (t/1000.0)) + 0.5)));
        col.setG(h1.g() + ((h2.g() - h1.g()) * (0.5 * sin(1.8 * 3.14 * (t/1000.0)) + 0.5)));
        col.setB(h1.b() + ((h2.b() - h1.b()) * (0.5 * sin(1.8 * 3.14 * (t/1000.0)) + 0.5)));
        col.setA(h1.a() + ((h2.a() - h1.a()) * (0.5 * sin(1.8 * 3.14 * (t/1000.0)) + 0.5)));

        return col;
    };
}

std::string intToHex(u64 i){
    std::stringstream ss;
    ss << std::hex << i;
    std::string hexStr = ss.str();

    ss.str("");
    ss.clear();
    ss << "0x" << std::setfill('0') << std::setw(16) << std::hex << i;
    std::string hexFormatted = ss.str();

    return hexFormatted;
}

void stubbed(const void* _, ...){}

uint64_t timeCode(std::function<void()> f){
    auto start = std::chrono::steady_clock::now();
    f();
    auto end = std::chrono::steady_clock::now();

    return std::chrono::duration_cast<
        std::chrono::nanoseconds
    >(end-start).count();
}

void toLower(std::string& s){
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){
        return std::tolower(c);
    });
}