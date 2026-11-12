#pragma once

#include <Utils/Sound.hpp>

#include <string>
#include <unordered_map>

#include <switch.h>


namespace Rune {
    /**
     * @brief A singleton class for playing loaded `Sound` objects from a sound ID
     */
    class Audio {
        private:
            /** @brief The preloaded sounds */
            inline static std::unordered_map<std::string, Sound*> db_;
        public:
            /** 
             * @brief Initialise the audio subsystem 
             * @note Returns nullptr on success
             */
            static const char* Init();

            /**
             * @brief Play a sound by its sound ID
             * @param id The sound ID to play
             * @return Error code
             */
            static Result playSound(const std::string& id);

    };
} // namespace Rune