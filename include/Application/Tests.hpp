#pragma once

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
         * @brief Test the save encoder.
         */
        bool saveCompiler();

        /**
         * All the tests.
         */
        extern std::map<std::string, std::function<bool()>> Tests;
    }
}


