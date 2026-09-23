#pragma once
#include <Aether/Aether.hpp>
#include <string>

#define WINDOW_RESOLUTION_X 1280
#define WINDOW_RESOLUTION_Y 720

#define CAPTION_IMG_ASPECT_RATIO 1.777

inline const Aether::Colour TEXT_COLOUR{255, 255, 255, 255};
inline const Aether::Colour MUTED_TEXT_COLOR{170, 170, 170, 255};

inline const Aether::Colour SEPARATOR_COLOUR{96, 96, 96, 255};
inline const Aether::Colour BACKGROUND_COLOUR{45, 45, 45, 255};
inline const Aether::Colour MUTED_BACKGROUND_COLOUR{30, 30, 30, 255};
inline const Aether::Colour DEBUG_COLOUR{5, 5, 10, 255};

const unsigned int TOP_SEPARATOR_Y = 90;
const unsigned int BOTTOM_SEPARATOR_Y = 647;
const unsigned int MAX_SEPARATOR_X = 1250;
const unsigned int MIN_SEPARATOR_X = 30;

const long int BOTW_TITLE_ID = 0x01007EF00011E000;