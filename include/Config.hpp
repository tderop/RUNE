#pragma once

#include <Aether/Aether.hpp>
#include <string>
#include <unordered_map>

#include <switch.h>

#define WINDOW_RESOLUTION_X 1280
#define WINDOW_RESOLUTION_Y 720

#define CAPTION_IMG_ASPECT_RATIO 1.777

inline const Aether::Colour TEXT_COLOUR{255, 255, 255, 255};
inline const Aether::Colour MUTED_TEXT_COLOR{170, 170, 170, 255};
inline const Aether::Colour SELECTED_TEXT_COLOUR{49, 210, 247, 255};

inline const Aether::Colour SEPARATOR_COLOUR{96, 96, 96, 255};
inline const Aether::Colour LIGHT_SEPARATOR_COLOUR{96, 96, 96, 150};
inline const Aether::Colour LIGHT_BACKGROUND_COLOUR{55, 55, 55, 255};
inline const Aether::Colour BACKGROUND_COLOUR{35, 35, 35, 255};
inline const Aether::Colour MUTED_BACKGROUND_COLOUR{20, 20, 20, 255};
inline const Aether::Colour DEBUG_COLOUR{5, 5, 10, 255};
inline const Aether::Colour TOP_R{89, 252, 220, 255};
inline const Aether::Colour BOTTOM_R{20, 202, 197, 255};

inline const Aether::Colour EDITOR_SIDEBAR_COLOUR{45, 45, 45, 255};

const unsigned int TOP_SEPARATOR_Y = 90;
const unsigned int BOTTOM_SEPARATOR_Y = 647;
const unsigned int MAX_SEPARATOR_X = 1250;
const unsigned int MIN_SEPARATOR_X = 30;

const int TEXT_X_OFFSET = 6;
const int TEXT_Y_OFFSET = 12;

const int MIN_SPLASH_SCREEN_TIME = 2000; // artificial delay for *polish*
const int STARTUP_DELAY = 2; // seconds, wait for 2 seconds to check if the user has pressed L

const u64 BOTW_TITLE_ID = 0x01007EF00011E000;

const std::unordered_map<std::string, std::string> ASSET_PATHS = {
    {"check",    "romfs:/img/check.png"},
    {"edit",     "romfs:/img/edit.png"},
    {"triforce", "romfs:/img/triforce.png"},
    {"star",     "romfs:/img/star.png"},
    {"splash",   "romfs:/img/splash.png"},
    {"chib",     "romfs:/img/hd/chib.png"},
    {"star_alt", "romfs:/img/star_alt.png"},
    {"swap",     "romfs:/img/swap.png"},
    {"bin",      "romfs:/img/bin.png"},

    {"staminawheel_1", "romfs:/img/wheel_1.png"},
    {"staminawheel_2", "romfs:/img/wheel_2.png"},
    {"staminawheel_3", "romfs:/img/wheel_3.png"},
    {"staminawheel_4", "romfs:/img/wheel_4.png"},
    {"staminawheel_5", "romfs:/img/wheel_5.png"},

    {"heart", "romfs:/img/heart.png"}
};

const std::string DEFAULT_EDITOR_TAB = "Stats";
const std::string RUNE_PATH   = "sdmc:/switch/RUNE/";
const std::string BACKUP_PATH = RUNE_PATH+"backups/";
const std::string THUMB_PATH  = RUNE_PATH+"assets/thumb/";
const std::string SOUND_PATH  = RUNE_PATH+"assets/sound/";
const std::string EXTRACTED_MARKER = RUNE_PATH+"assets/extracted.done";
const std::string PLACEHOLDER_THUMB_PATH = "romfs:/img/placeholder.png";

const std::string RUNE_VERSION = "0.0.0 (indev)";