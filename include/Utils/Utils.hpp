#pragma once
#include <switch.h>
#include <iostream>

void log(const std::string& msg);
Result getUserID(AccountUid* out);
std::string getUserNickname();
std::string getDeviceNickname();
std::string formatUnixTimestamp(int32_t unixTimestamp);

void clearLine();