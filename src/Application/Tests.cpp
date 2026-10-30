#include <switch.h>

#include <Application/Tests.hpp>
#include <Application/Romfs.hpp>

#include <Utils/Utils.hpp>

#include <functional>
#include <string>
#include <map>

// Tested modules
#include <Save/Save.hpp>

Result Rune::Tests::botwRomfsMounting(){
    // Test mounting
    Result rc = Rune::Romfs::mountBotwRomfs();
    if (R_FAILED(rc)){
        puts("\tmountBotwRomfs() failed.");
        return rc;
    }

    // Unmount
    rc = Rune::Romfs::unmountBotwRomfs();
    if (R_FAILED(rc)){
        puts("\tunmountBotwRomfs() failed.");
        return rc;
    }

    return 0;
}

Result Rune::Tests::botwRomfsExtracting(){
    Result rc = Rune::Romfs::extractAssets();
    return rc;
}

namespace Rune{
    namespace Tests{
        std::map<std::string, std::function<Result()>> Tests = {
            {"Game RomFS Mounting", Rune::Tests::botwRomfsMounting},
            {"Game RomFS Extracting", Rune::Tests::botwRomfsExtracting}
        };
    }
}

void Rune::Tests::Run(){
    int testsPassed = 0;
    int totalTests = Rune::Tests::Tests.size();

    for (auto const& [testName, test] : Rune::Tests::Tests){
        Result result = test();

        if (R_SUCCEEDED(result)){
            printf("[%s] - \x1b[32mPASSED\x1b[0m: %s\n\n", testName.c_str(), intToHex(result).c_str());
            testsPassed+=1;
        } else {
            printf("[%s] - \x1b[31mFAILED\x1b[0m: %s\n\n", testName.c_str(), intToHex(result).c_str());
        }

        consoleUpdate(NULL);
    }

    printf("Test summary - %d / %d\n", testsPassed, totalTests);
}