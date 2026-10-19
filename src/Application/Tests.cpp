#include <Application/Tests.hpp>
#include <Utils/Utils.hpp>

#include <functional>
#include <string>
#include <map>

// Tested modules
#include <Save/Save.hpp>

// stub for now
bool Rune::Tests::saveCompiler(){
    return true;
}

namespace Rune{
    namespace Tests{
        std::map<std::string, std::function<bool()>> Tests = {
            {"SAVE ENCODING", Rune::Tests::saveCompiler}
        };
    }
}

void Rune::Tests::Run(){
    int testsPassed = 0;
    int totalTests = Rune::Tests::Tests.size();

    for (auto const& [testName, test] : Rune::Tests::Tests){
        bool passed = test();

        if (passed){
            log("["+testName+"] - PASSED");
            testsPassed+=1;
        } else {
            log("["+testName+"] - FAILED");
        }
    }

    log("Test summary - "+std::to_string(testsPassed)+"/"+std::to_string(totalTests));
}