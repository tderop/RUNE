// libnx
#include <switch.h>

// RUNE
#include <Application/Application.hpp>
#include <Application/Romfs.hpp>

#include <Utils/Utils.hpp>
#include <Config.hpp>

Result Rune::Romfs::extractAssets(){
    // Attempt to mount BotW RomFS
    Result rc = Rune::Romfs::mountBotwRomfs();
    if (R_FAILED(rc)){
        return rc;
    }

    // All is done, unmount BotW RomFS
    rc = Rune::Romfs::unmountBotwRomfs();
    return rc;
}

Result Rune::Romfs::decodeSbItemIco(Rune::Romfs::SbItemIco* output, const std::string& path){

    return 0;
}

Result Rune::Romfs::getThumb(Rune::Romfs::SbItemIco& sbitemico, const std::string& output){

    return 0;
}

Result Rune::Romfs::decodeBars(Rune::Romfs::Bars* out, const std::string& path){

    return 0;
}

Result Rune::Romfs::getAudio(Rune::Romfs::Bars& bars, const std::string& barsPath, const std::string& output){

    return 0;
}


Result Rune::Romfs::mountBotwRomfs(){
    if (Rune::Application::IsEmulator){
        log("[Rune::Romfs::mountBotwRomfs] - Game data mount failed! fsOpenDataStorageByProgramId is not implenented in Ryujinx.", true);
        return LibnxError_IoError; // lol closest thing
    }

    // Open BotW's data storage to get FsStorage item
    FsStorage botwStorage;
    Result rc = fsOpenDataStorageByProgramId(&botwStorage, BOTW_TITLE_ID);
    if (R_FAILED(rc)){
        return LibnxError_IoError;
    }

    // Use FsStorage item to open BotW RomFS at botw:/
    rc = romfsMountFromStorage(botwStorage, 0, "botw");
    return rc;
}

Result Rune::Romfs::unmountBotwRomfs(){
    Result rc = romfsUnmount("botw");
    return rc;
}