#pragma once

#include <nlohmann.hpp>

#include <vector>
#include <string>
#include <unordered_map>

#include <switch.h>

namespace Rune{
    using Json = nlohmann::json;
    
    namespace JsonData{
        extern std::unordered_map<std::string, std::string> JsonPaths;
        extern std::unordered_map<std::string, Json> JsonObjects;

        /**
         * @brief Initialise JSON data.
         */
        void Init();

        /**
         * @brief Get JSON data by the JSON ID.
         */
        Result GetJsonById(Rune::Json*& out, const std::string &id);
    } // namespace JsonData
} // namespace Rune