#pragma once

#include <switch.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <filesystem>

/**
 * @brief A simple sound instance used for playing one-shot sound effects using SDL2_mixer
 * @note Automatically frees resources on destruction, make sure to delete allocated resources
 * if using raw pointers to this object
 */
class Sound {
    private:
        /** @brief Whether the sound service is initialized or not */
        inline static bool initialized_ = false;
        /** @brief The path to the sound to be played */
        std::filesystem::path path_;
        /** @brief The mix audio object */
        Mix_Music* audio_ = nullptr;

    public:
        /** 
         * @brief Run this to initialise the use of sounds in the project 
         * @note Returns error code on failure, and nullptr on success
         * */
        static const char* Init();

        Sound() = default;

        /**
         * @brief Create a new sound object
         * @note Only .opus files can be used
         */
        Sound(const std::string& path);

        /**
         * @brief Play the sound
         */
        Result play();

        /** 
         * @brief Update the audio path
         * @param path The new audio path
         */
        Result setPath(const std::string& path);

        /**
         * @brief Determine if the sound is successfully loaded or not
         */
        bool isLoaded();

        /**
         * @brief Free resources on destruction
         */
        ~Sound();
};