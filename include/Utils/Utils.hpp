#pragma once

#include <switch.h>
#include <iostream>
#include <functional>

#include <Aether/Aether.hpp>

/**
 * @brief Get the current user's AccountUid
 * @param out Where the output will be stored
 * @return Error code
 */
Result getUserID(AccountUid* out);

/**
 * @brief Get the current user's nickname - set via their Switch's user menu
 * @return User's nickname
 */
std::string getUserNickname();

/**
 * @brief Get the current system's nickname - set via the settings menu
 * @return Device nickname
 */
std::string getDeviceNickname();

/**
 * @brief Format a UNIX timestamp in the format DD/MM/YYYY HH:MM
 * @param unixTimestamp The UNIX timestamp
 * @return The formatted timestamp string
 */
std::string formatUnixTimestamp(int32_t unixTimestamp);

/**
 * @brief Convert an integer to a hexadecimal string
 * @param i The integer to convert to hex
 * @param w The width of the string (padded by zeroes)
 * @return The hex string
 */
std::string intToHex(u64 i);

/**
 * @brief A function that does nothing. Use this to quickly stub out function calls.
 * @note Recommended to only use this for one function, don't replace many different functions with this.
 * @param anything Accepts all parameters, and does nothing
 */
void stubbed(const void* _, ...);

/**
 * @brief Clear a line in the libnx console made with consoleInit();
 */
void clearLine();

/**
 * @brief Time how long a function takes to run
 * @param f Function to run
 * @return Time elapsed in milliseconds
 */
uint64_t timeCode(std::function<void()> f);

/**
 * @note A function I stole from NX-Activity-Log to animate the selection borders.
 */
std::function<Aether::Colour(uint32_t)> highlightFunc();