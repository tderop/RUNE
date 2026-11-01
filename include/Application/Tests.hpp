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
        Result BotwRomfsMounting();

        /**
         * @brief Test romfs extracting
         */
        Result BotwRomfsExtracting();

        /**
         * @brief Test the sbitemico decoding
         */
        Result BotwRomfsDecoding();

        /**
         * All the tests.
         */
        extern std::map<std::string, std::function<Result()>> Tests;
    }
}


