// libnx
#include <switch.h>

// C++
#include <filesystem>
#include <fstream>

// C
#include <cstring>

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

    // ENSURING FILESYSTEM INTEGRITY

    // Ensure icon directory exists
    std::filesystem::path iconDir = Rune::Romfs::kSbItemIcoFolder;
    if (!std::filesystem::exists(iconDir) || !std::filesystem::is_directory(iconDir)){
        // Icons not found, different game may have been launched
        printf("[Rune::Romfs::extractAssets] - Folder %s not found.\n", iconDir.c_str());
        return LibnxError_NotFound;
    }

    // Ensure M_UI.bars exists
    std::filesystem::path menuBars = Rune::Romfs::kMenuSoundBars;
    if (!std::filesystem::exists(menuBars)){
        // Sounds not found, different game may have been launched
        printf("[Rune::Romfs::extractAssets] - File %s not found.\n", menuBars.c_str());
        return LibnxError_NotFound;
    }

    // EXTRACTING ITEM ICONS

    // Iterate through every SBITEMICO file in the StockItem dir
    for (const auto& entry : std::filesystem::directory_iterator(iconDir)){
        // Get the image
        decompressYaz0(entry.path());
        break;
    }

    // All is done, unmount BotW RomFS
    rc = Rune::Romfs::unmountBotwRomfs();
    return rc;
}

Result Rune::Romfs::decompressYaz0(const std::string& path){
    // Check that the path exists
    if (!std::filesystem::exists(path)){
        printf("[Rune::Romfs::decodeSbItemIco] - Path doesn't exist: '%s'", path.c_str());
        return LibnxError_NotFound;
    }

    // Open the file
    std::ifstream yaz0(path, std::ios::binary);
    if (!yaz0){
        printf("[Rune::Romfs::decodeSbItemIco] - Failed to open yaz0 file: '%s'", path.c_str());
        return LibnxError_IoError;
    }
    
    // Verify yaz0 integrity
    char buf[4];
    if (!yaz0.read(buf, sizeof(buf))){
        printf("[Rune::Romfs::decodeSbItemIco] - Failed to read Yaz0 magic.");
        return LibnxError_IoError;
    }

    // Compare bytes with Yaz0
    if (std::memcmp(buf, "Yaz0", sizeof(buf)) != 0){
        printf("[Rune::Romfs::decodeSbItemIco] - Failed to verify integrity: '%s'", path.c_str());
        return LibnxError_BadInput;
    }

    // Get the decompressed size
    char sizeBuf[4];
    if (!yaz0.read(sizeBuf, sizeof(sizeBuf))){
        printf("[Rune::Romfs::decodeSbItemIco] - Failed to read decompressed yaz0 size: '%s'", path.c_str());
        return LibnxError_BadInput;
    }

    // Swap the bytes around to convert to big-endian
    size_t decompressedSize = 0;
    for (uint32_t s = 0; s < 4; s++){
        decompressedSize |= static_cast<size_t>(static_cast<unsigned char>(sizeBuf[s])) << (24-(s*8));
        // 24, 16, 8, 
    }          

    // Skip the 16-byte yaz0 header
    yaz0.seekg(16, std::ios::beg);

    // Allocate decompressed buffer
    std::vector<uint8_t> decompressed{};
    decompressed.resize(decompressedSize);

    // The index where the next decompressed byte will be written
    size_t cursor = 0;

    // Main decoding loop
    while (cursor < decompressedSize){
        // Read a byte
        char buf[1];
        if (!yaz0.read(buf, sizeof(buf))){
            printf("[Rune::Romfs::decodeSbItemIco] - Failed to read byte at position %s: '%s'", 
                intToHex(cursor).c_str(),
                path.c_str()
            );

            return LibnxError_BadInput;
        }

        unsigned char byte = static_cast<unsigned char>(buf[0]);

        // Read the byte's bits
        for (int i = 0; i < 8 && cursor < decompressedSize; ++i){ // stop loop when output is full
            bool bit = byte & (0x80 >> i);

            // If bit is 1, read a new byte from the stream
            if (bit){
                char nextByte[1];
                // Check the read succeeded
                if (!yaz0.read(nextByte, sizeof(nextByte))){
                    printf("[Rune::Romfs::decodeSbItemIco] - Failed to read byte at position %s: '%s'", 
                        intToHex(cursor).c_str(),
                        path.c_str()
                    );

                    return LibnxError_BadInput;
                }
                
                // Store the byte at the cursor index
                decompressed[cursor] = static_cast<uint8_t>(nextByte[0]);

                // Increment cursor
                cursor++;
            } else {
                // Read two bytes
                char nextBytes[2];

                // Check the read succeeded
                if (!yaz0.read(nextBytes, sizeof(nextBytes))){
                    printf("[Rune::Romfs::decodeSbItemIco] - Failed to read byte pair at position %s: '%s'", 
                        intToHex(cursor).c_str(),
                        path.c_str()
                    );

                    return LibnxError_BadInput;
                }

                // Combine byte pair into a 16 bit value
                uint16_t pair = (static_cast<unsigned char>(nextBytes[0]) << 8) | 
                    static_cast<unsigned char>(nextBytes[1]);

                size_t distance = (pair & 0x0FFF) + 1;
                size_t lengthCode = pair >> 12;

                // Determine actual length
                size_t length=0;

                if (lengthCode != 0){ // Add two
                    length = lengthCode + 2;
                } else {
                    // Read an additional byte
                    char extraByte[1];
                    if (!yaz0.read(extraByte, sizeof(extraByte))){
                        printf("[Rune::Romfs::decodeSbItemIco] - Failed to read byte pair at position %s: '%s'", 
                            intToHex(cursor).c_str(),
                            path.c_str()
                        );

                        return LibnxError_BadInput;
                    }

                    // Convert unsigned byte to uint8_t
                    uint8_t toAdd = static_cast<uint8_t>(static_cast<unsigned char>(extraByte[0]));
                    length = toAdd + 18;
                }

                // Validate distance
                if (distance > cursor){
                    printf("[Rune::Romgs::decodeSbItemIco] - Back-reference distance cannot be greater than cursor position.\n\tCursor: %s\n\tDistance: %s\n", 
                        intToHex(cursor).c_str(), intToHex(distance).c_str()
                    );
                    return LibnxError_BadInput;
                }

                // Validate length
                size_t remainingSpace = decompressedSize-cursor;
                if (length > remainingSpace){
                    printf("[Rune::Romgs::decodeSbItemIco] - Data length cannot be greater than remaining file space.\n\tCursor: %s\n\tRemaining Space: %s\n", 
                        intToHex(cursor).c_str(), intToHex(remainingSpace).c_str()
                    );
                    return LibnxError_BadInput;
                }

                size_t source = cursor-distance;

                // For the length of this data, write it
                for (size_t j = 0; j < length; ++j){
                    // Read byte at source index
                    uint8_t byte = decompressed.at(source);

                    // Write it to the cursor index
                    decompressed[cursor] = byte;

                    // Increment source and cursor
                    source++; cursor++;
                }
            }
        }
    }


    /** TEMP CODE */

    // Open the output file
    std::ofstream bfres;
    bfres.open("sdmc:/item.bfres", std::ofstream::binary);

    if (!bfres.is_open()){
        printf("[Rune::Romfs::decodeSbItemIco] - Failed to open .bfres output!\n");
        return LibnxError_IoError;
    }

    // Write to the output
    bfres.write(
        reinterpret_cast<const char*>(decompressed.data()),
        static_cast<std::streamsize>(decompressed.size())
    );

    // Close the file
    bfres.close();

    if (!bfres){
        printf("[Rune::Romfs::decodeSbItemIco] - Failed to save .bfres output!\n");
        return LibnxError_IoError;
    }

    /** TEMP CODE */

    return 0;
}
Result Rune::Romfs::decodeSbItemIco(Rune::Romfs::SbItemIco* output, const std::string& path){
    // Decompress the yaz0 file
    Result rc = Rune::Romfs::decompressYaz0(path);
    if (R_FAILED(rc)){
        return rc;
    }

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
        puts("[Rune::Romfs::mountBotwRomfs] - Game data mount failed! romsMountFromCurrentProcess is not implenented in Ryujinx.");
        return LibnxError_IoError; // lol closest thing
    }

    Result rc = romfsMountFromCurrentProcess("botw");
    if (R_FAILED(rc)){
        puts("[Rune::Romfs::mountBotwRomfs] - romfsMountFromCurrentProcess() failed.");
    }
    return rc;
}

Result Rune::Romfs::unmountBotwRomfs(){
    Result rc = romfsUnmount("botw");
    return rc;
}

/**
 * @brief An old implementation of mountBotwRomfs(), which attempts to allow asset extraction
 * from any execution context. Unfortunately, fsOpenDataStorageByProgramId() does not agree with me or my choices.
 */
Result mountBotwRomfsOld(){
    if (Rune::Application::IsEmulator){
        printf("[Rune::Romfs::mountBotwRomfs] - Game data mount failed! fsOpenDataStorageByProgramId is not implenented in Ryujinx.\n");
        return LibnxError_IoError; // lol closest thing
    }

    // Open BotW's data storage to get FsStorage item
    FsStorage botwStorage;

    u64 titleId = BOTW_TITLE_ID;
    Result rc = fsOpenDataStorageByProgramId(&botwStorage, titleId);

    if (R_FAILED(rc)){
        printf("[Rune::Romfs::mountBotwRomfs] - Failed to open BotW storage.\n\tTitle ID: %s\n", 
            intToHex(titleId).c_str()
        );

        return rc;
    }

    // Use FsStorage item to open BotW RomFS at botw:/
    rc = romfsMountFromStorage(botwStorage, 0, "botw");
    if (R_FAILED(rc)){
        printf("[Rune::Romfs::mountBotwRomfs] - Failed to mount BotW storage.\n");
        fsStorageClose(&botwStorage);

        return rc;
    }

    return rc;
}