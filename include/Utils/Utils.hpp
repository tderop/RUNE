#pragma once

#include <switch.h>
#include <iostream>
#include <functional>

#include <Aether/Aether.hpp>

extern bool logServiceIsEmulator;

void log(const std::string& msg, bool isImportant=false);
Result getUserID(AccountUid* out);
std::string getUserNickname();
std::string getDeviceNickname();
std::string formatUnixTimestamp(int32_t unixTimestamp);
std::function<Aether::Colour(uint32_t)> highlightFunc();
std::string intToHex(int i);

void clearLine();