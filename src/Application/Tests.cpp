#include <switch.h>

#include <Application/Tests.hpp>
#include <Application/Romfs.hpp>

#include <Utils/Utils.hpp>

#include <functional>
#include <string>
#include <map>
#include <filesystem>

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

Result Rune::Tests::botwRomfsDecoding(){
    std::filesystem::path testIcoPath = "sdmc:/TestIco";

    for (const auto& entry : std::filesystem::directory_iterator(testIcoPath)){
        Result rc = Rune::Romfs::extractSbitemIcon(entry.path());
        if (R_FAILED(rc)){
            return rc;
        }
        printf("\n");
    }

    return 0;
}

namespace Rune{
    namespace Tests{
        std::map<std::string, std::function<Result()>> Tests = {
            {"Game RomFS Mounting", Rune::Tests::botwRomfsMounting},
            {"Game RomFS Extracting", Rune::Tests::botwRomfsExtracting}
            // {"Yaz0/BFRES Decoding", Rune::Tests::botwRomfsDecoding}
        };
    }
}

void Rune::Tests::Run(){
    int testsPassed = 0;
    int totalTests = Rune::Tests::Tests.size();

    for (auto const& [testName, test] : Rune::Tests::Tests){
        Result result = test();

        if (R_SUCCEEDED(result)){
            printf("[%s] - \x1b[32mPASSED\x1b[0m: %08X\n\n", testName.c_str(), result);
            testsPassed+=1;
        } else {
            printf("[%s] - \x1b[31mFAILED\x1b[0m: %08X\n\n", testName.c_str(), result);
        }

        consoleUpdate(NULL);
    }

    printf("Test summary - %d / %d\n", testsPassed, totalTests);
}