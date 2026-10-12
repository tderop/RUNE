#pragma once

#include <Aether/Aether.hpp>
#include <string>
#include <unordered_map>

#define WINDOW_RESOLUTION_X 1280
#define WINDOW_RESOLUTION_Y 720

#define CAPTION_IMG_ASPECT_RATIO 1.777

inline const Aether::Colour TEXT_COLOUR{255, 255, 255, 255};
inline const Aether::Colour MUTED_TEXT_COLOR{170, 170, 170, 255};
inline const Aether::Colour SELECTED_TEXT_COLOUR{49, 210, 247, 255};

inline const Aether::Colour SEPARATOR_COLOUR{96, 96, 96, 255};
inline const Aether::Colour LIGHT_SEPARATOR_COLOUR{96, 96, 96, 150};
inline const Aether::Colour BACKGROUND_COLOUR{45, 45, 45, 255};
inline const Aether::Colour MUTED_BACKGROUND_COLOUR{30, 30, 30, 255};
inline const Aether::Colour DEBUG_COLOUR{5, 5, 10, 255};
inline const Aether::Colour TOP_R{89, 252, 220, 255};
inline const Aether::Colour BOTTOM_R{20, 202, 197, 255};

inline const Aether::Colour EDITOR_SIDEBAR_COLOUR{69, 69, 69, 255};

const unsigned int TOP_SEPARATOR_Y = 90;
const unsigned int BOTTOM_SEPARATOR_Y = 647;
const unsigned int MAX_SEPARATOR_X = 1250;
const unsigned int MIN_SEPARATOR_X = 30;

const int TEXT_X_OFFSET = 6;
const int TEXT_Y_OFFSET = 12;

const int MIN_SPLASH_SCREEN_TIME = 3000; // milliseconds... people won't see my beautiful splash screen :(

const long int BOTW_TITLE_ID = 0x01007EF00011E000;

const std::unordered_map<std::string, std::string> ASSET_PATHS = {
    {"check",    "romfs:/assets/check.png"},
    {"triforce", "romfs:/assets/triforce.png"},
    {"star",     "romfs:/assets/star.png"},
    {"splash",   "romfs:/assets/splash.png"}
};

const std::string DEFAULT_EDITOR_TAB = "Stats";