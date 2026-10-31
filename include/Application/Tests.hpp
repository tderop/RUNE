#pragma once
#include <switch.h>

#include <functional>
#include <string>
#include <map>

namespace Rune{
    namespace Tests{
        /**
         * @brief Run all tests.
         */
        void Run();

        /**
         * @brief Test romfs mounting
         */
        Result botwRomfsMounting();

        /**
         * @brief Test romfs extracting
         */
        Result botwRomfsExtracting();

        /**
         * @brief Test the sbitemico decoding
         */
        Result botwRomfsDecoding();

        /**
         * All the tests.
         */
        extern std::map<std::string, std::function<Result()>> Tests;
    }
}


