// stb
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

// libnx
#include <switch.h>

// C++
#include <filesystem>
#include <fstream>
#include <algorithm>

// C
#include <cstring>

// RUNE
#include <Application/Application.hpp>
#include <Application/Romfs.hpp>

#include <Utils/Utils.hpp>
#include <Config.hpp>

Result Rune::Romfs::ExtractAssets(std::function<void(size_t, size_t)> onProgress){
    // If extracted, ignore
    if (std::filesystem::exists(EXTRACTED_MARKER)){
        return 69;
    }

    // Attempt to mount BotW RomFS
    Result rc = Rune::Romfs::MountBotwRomfs();
    if (R_FAILED(rc)){
        return rc;
    }

    // ENSURING FILESYSTEM INTEGRITY

    // Ensure icon directory exists
    std::filesystem::path iconDir = Rune::Romfs::kSbItemIcoFolder;
    if (!std::filesystem::exists(iconDir) || !std::filesystem::is_directory(iconDir)){
        // Icons not found, different game may have been launched
        stubbed("[Rune::Romfs::ExtractAssets] - Folder %s not found.\n", iconDir.c_str());
        return LibnxError_NotFound;
    }

    // Ensure M_UI.bars exists
    std::filesystem::path menuBars = Rune::Romfs::kMenuSoundBars;
    if (!std::filesystem::exists(menuBars)){
        // Sounds not found, different game may have been launched
        stubbed("[Rune::Romfs::ExtractAssets] - File %s not found.\n", menuBars.c_str());
        return LibnxError_NotFound;
    }

    size_t currentProgress = 0;
    size_t maxProgress = std::distance(
        std::filesystem::directory_iterator(iconDir),
        std::filesystem::directory_iterator{}
    ); 

    // EXTRACTING ITEM ICONS

    // Iterate through every SBITEMICO file in the StockItem dir
    for (const auto& entry : std::filesystem::directory_iterator(iconDir)){
        // Update completed BEFORE (so already processed files don't get ignored lol)
        onProgress(++currentProgress, maxProgress);

        std::filesystem::path entryPath = entry.path();
        entryPath.replace_extension("");
        std::string actorName = entryPath.filename().string();

        if (std::filesystem::exists(THUMB_PATH+actorName+".png")){
            // skip files that have already been extracted
            continue;
        }

        // Extract the thumbnail
        rc = Rune::Romfs::ExtractSbitemIcon(entry.path());
        if (R_FAILED(rc)){
            stubbed("[Rune::Romfs::ExtractAssets] - Failed to extract icon: %08X\n", rc);
            break;
        }
    }

    // All is done, unmount BotW RomFS
    rc = Rune::Romfs::UnmountBotwRomfs();

    // Create a file to indicate that all files were extracted
    std::ofstream marker(EXTRACTED_MARKER);
    if (!marker.is_open()){
        return LibnxError_IoError;
    }

    marker.close();
    return rc;
}


// CHANGES:
/**
 * Make it store in a span/pre-allocated area in memory/pointer thingy for decoding (struct) I am tired goodbye
 */
Result Rune::Romfs::DecompressYaz0(std::vector<uint8_t>& output, const std::string& path){
    // Check that the path exists
    if (!std::filesystem::exists(path)){
        stubbed("[Rune::Romfs::DecompressYaz0] - Path doesn't exist: '%s'", path.c_str());
        return LibnxError_NotFound;
    }

    // Open the file
    std::ifstream yaz0(path, std::ios::binary);
    if (!yaz0){
        stubbed("[Rune::Romfs::DecompressYaz0] - Failed to open yaz0 file: '%s'", path.c_str());
        return LibnxError_IoError;
    }
    
    // Verify yaz0 integrity
    char buf[4];
    if (!yaz0.read(buf, sizeof(buf))){
        stubbed("[Rune::Romfs::DecompressYaz0] - Failed to read Yaz0 magic.");
        return LibnxError_IoError;
    }

    // Compare bytes with Yaz0
    if (std::memcmp(buf, "Yaz0", sizeof(buf)) != 0){
        stubbed("[Rune::Romfs::DecompressYaz0] - Failed to verify integrity: '%s'", path.c_str());
        return LibnxError_BadInput;
    }

    // Get the decompressed size
    char sizeBuf[4];
    if (!yaz0.read(sizeBuf, sizeof(sizeBuf))){
        stubbed("[Rune::Romfs::DecompressYaz0] - Failed to read decompressed yaz0 size: '%s'", path.c_str());
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
    std::vector<uint8_t> decompressed(decompressedSize);

    // The index where the next decompressed byte will be written
    size_t cursor = 0;

    // Main decoding loop
    while (cursor < decompressedSize){
        // Read a byte
        char buf[1];
        if (!yaz0.read(buf, sizeof(buf))){
            stubbed("[Rune::Romfs::DecompressYaz0] - Failed to read byte at position %s: '%s'", 
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
                    stubbed("[Rune::Romfs::DecompressYaz0] - Failed to read byte at position %s: '%s'", 
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
                    stubbed("[Rune::Romfs::DecompressYaz0] - Failed to read byte pair at position %s: '%s'", 
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
                        stubbed("[Rune::Romfs::DecompressYaz0] - Failed to read byte pair at position %s: '%s'", 
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
                    stubbed("[Rune::Romfs::DecompressYaz0] - Back-reference distance cannot be greater than cursor position.\n\tCursor: %s\n\tDistance: %s\n", 
                        intToHex(cursor).c_str(), intToHex(distance).c_str()
                    );
                    return LibnxError_BadInput;
                }

                // Validate length
                size_t remainingSpace = decompressedSize-cursor;
                if (length > remainingSpace){
                    stubbed("[Rune::Romfs::DecompressYaz0] - Data length cannot be greater than remaining file space.\n\tCursor: %s\n\tRemaining Space: %s\n", 
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

    output = std::move(decompressed);
    return 0;
}

Result Rune::Romfs::ExtractSbitemIcon(const std::string& path){
    stubbed("Decoding BFRES '%s'\n", path.c_str());

    // For result
    bool isIconDecoded = false;
    
    // Initialise decoded bfres data
    std::vector<uint8_t> bfres;

    // Decompress the sbitemico yaz0 archive
    Result rc = Rune::Romfs::DecompressYaz0(bfres, path);
    if (R_FAILED(rc)){
        return rc;
    }

    // Where the cursor currently is
    size_t cursor = 0;

    // Check the buffer contains at least 32 bytes
    size_t bfresSize = bfres.size();
    if (bfresSize < 0xCA){
        stubbed("[Rune::Romfs::ExtractSbitemIcon] - BFRES size cannot be smaller than 0xCA bytes.");
        return LibnxError_BadInput;
    }

    // Check for magic
    if (std::memcmp(&bfres.at(0), "FRES    ", 8) != 0){
        stubbed("[Rune::Romfs::ExtractSbitemIcon] - Failed to verify file integrity: '%s'", path.c_str());
        return LibnxError_BadInput;
    }

    // Check format
    uint16_t byteOrder = (static_cast<uint16_t>(bfres.at(0x0C)) << 8) 
        | bfres.at(0x0D);

    if (byteOrder != 0xFFFE){
        // Only allow little-endian input
        stubbed("[Rune::Romfs::ExtractSbitemIcon] - BFRES must be in little endian format!\n\tGot: %s\n",
            intToHex(byteOrder).c_str());

        return LibnxError_BadInput;
    }

    // Read version as four-byte little endian integer
    uint32_t version = 0;
    for (uint32_t s = 0; s < 4; s++){
        version |= static_cast<uint32_t>(static_cast<unsigned char>(
            bfres.at(0x08 + s))
        ) << (s*8);
    }

    stubbed("\tBFRES version: %s\n", intToHex(version).c_str());

    // Read file size as four-byte little endian integer
    size_t declaredSize = 0;
    for (uint32_t s = 0; s < 4; s++){
        declaredSize |= static_cast<size_t>(static_cast<unsigned char>(
            bfres.at(0x1C + s))
        ) << (s*8);
    }

    // Make sure sizes match
    stubbed("\tDeclared size: %s\n", intToHex(declaredSize).c_str());
    stubbed("\tActual size: %s\n", intToHex(bfresSize).c_str());

    if (bfresSize != declaredSize){
        puts("[Rune::Romfs::ExtractSbitemIcon] - Actual BFRES size does not match declared size.");
        return LibnxError_BadInput;
    }

    // Get the offset to the embedded file entry table
    uint64_t tblOffset = 0;
    std::memcpy(&tblOffset, &bfres.at(0x98), sizeof(tblOffset));

    // Get the number of files that need to be extracted
    uint16_t entryCount = 0;
    std::memcpy(&entryCount, &bfres.at(0xC8), sizeof(entryCount));

    stubbed("\tOffset: %s\n", intToHex(tblOffset).c_str());
    stubbed("\tEntry Count: %d\n", entryCount);

    if (entryCount == 0){
        // There are no files to extract
        return LibnxError_BadInput;
    }

    // Reject zero offset
    if (tblOffset == 0){
        return LibnxError_BadInput;
    }

    // Reject offset outside of the buffer
    if (tblOffset >= bfres.size()){
        return LibnxError_BadInput;
    }

    // Verify the table fits within the BFRES
    size_t tblSize = entryCount * 16;
    size_t sizeLeft = bfres.size() - tblOffset;

    if (tblSize > sizeLeft){
        return LibnxError_BadInput;
    }

    // Iterate through BFRES entries
    for (int i = 0; i < entryCount; ++i){
        uint64_t entryOffset = tblOffset + (i*16);

        // The 8 bytes at the entry offset are the offset of the embedded file
        uint64_t fileOffset = 0;
        std::memcpy(&fileOffset, &bfres.at(entryOffset), sizeof(fileOffset));

        // Ensure the file offset is within the bounds of the BFRES data
        if (fileOffset >= bfres.size()){
            return LibnxError_BadInput;
        }

        // The 8 bytes after this are the size of the embedded file
        uint64_t fileSize = 0;
        std::memcpy(&fileSize, &bfres.at(entryOffset+8), sizeof(fileSize));
        
        // Skip files smaller than 8 bytes (can't contain B N T X \0 . . . magic)
        if (fileSize < 8){
            continue;
        }

        // Ensure the file size is within the BFRES bounds
        if (fileOffset >= bfres.size() - fileOffset){
            return LibnxError_BadInput;
        }

        // Get first 8 bytes of file
        uint64_t magicBuf = 0;
        uint64_t bntxMagic = 0x0000000058544E42; // B N T X little endian

        // Move the first 8 bytes of the embedded file into magicBuf
        std::memcpy(&magicBuf, &bfres.at(fileOffset), sizeof(magicBuf));

        // Verify BNTX
        if (std::memcmp(&magicBuf, &bntxMagic, sizeof(magicBuf)) != 0){
            // Magic doesn't match
            stubbed("[Rune::Romfs::ExtractSbitemIcon] - Failed to verify BNTX integrity:\n\tGot: %s\n", 
                intToHex(magicBuf).c_str()
            );

            continue; // Not a BNTX file, continue don't return
        }

        // BNTX found, begin looking for texture
        
        // Check the BNTX file contains at least 0x30 bytes
        if (fileSize < 0x30){
            return LibnxError_BadInput;
        }

        // Check byte order
        uint64_t bntxByteOrder = (static_cast<uint16_t>(bfres.at(fileOffset+0x0C)) << 8) 
            | bfres.at(fileOffset+0x0D);
        
        if (bntxByteOrder != 0xFFFE){
            // Only allow little-endian input
            stubbed("[Rune::Romfs::ExtractSbitemIcon] - BNTX must be in little endian format!\n\tGot: %s\n",
                intToHex(bntxByteOrder).c_str());

            continue; // just ignore the file and try to look for the next valid BNTX
        }

        // I CAN'T STOP TYPING BNTX AS BTNX
        Rune::Romfs::BNTX bntx;

        std::memcpy(&bntx.size, &bfres.at(fileOffset+0x1C), sizeof(bntx.size));
        std::memcpy(&bntx.textureCount, &bfres.at(fileOffset+0x24), sizeof(bntx.textureCount));
        std::memcpy(&bntx.textureTblOffset, &bfres.at(fileOffset+0x28), sizeof(bntx.textureTblOffset));

        stubbed("\t-- BNTX --\n\t\tSize: %s\n\t\tTexture Count: %d\n\t\tOffset: %s\n",
            intToHex(bntx.size).c_str(),
            bntx.textureCount,
            intToHex(bntx.textureTblOffset).c_str()
        );

        // verify values

        // BNTX must be larger than 0x30 bytes
        if (bntx.size < 0x30){
            return LibnxError_BadInput;
        }

        // BNTX must be within fileSize
        if (bntx.size > fileSize){
            return LibnxError_BadInput;
        }

        // BNTX cannot be larger than the remaining BFRES size
        if (fileOffset+bntx.size >= bfres.size()){
            return LibnxError_BadInput;
        }

        // Ignore BNTX with 0 files
        if (bntx.textureCount == 0){
            continue;
        }

        // Ignore 0 offset
        if (bntx.textureTblOffset == 0){
            continue;
        }

        // BNTX table offset cannot be outside of the BFRES bounds
        if (bntx.textureTblOffset >= bfres.size()){
            return LibnxError_BadInput;
        }

        // Check if texture size overflows
        uint64_t texSize = bntx.textureCount*8;
        if (texSize > bntx.size-bntx.textureTblOffset){
            return LibnxError_BadInput;
        }

        // Read the first entry for the first texture's BRTI offset
        uint64_t brtiOffsetInBntx = 0;
        std::memcpy(&brtiOffsetInBntx, &bfres.at(fileOffset+bntx.textureTblOffset), 
            sizeof(brtiOffsetInBntx));

        // Verify BRTI offset
        if (brtiOffsetInBntx == 0){
            return LibnxError_BadInput;
        }

        // BRTI offset cannot be outside of BNTX bounds
        if (brtiOffsetInBntx > bntx.size){
            return LibnxError_BadInput;
        }

        // verify BRTI magic
        int32_t brtiMagicBuf = 0;
        uint64_t brtiMagic = 0x49545242;

        std::memcpy(&brtiMagicBuf, &bfres.at(fileOffset+brtiOffsetInBntx), sizeof(brtiMagicBuf));
        if (std::memcmp(&brtiMagicBuf, &brtiMagic, sizeof(brtiMagicBuf)) != 0){
            stubbed("[Rune::Romfs::ExtractSbitemIcon] - Failed to verify BRTI integrity.\n\tGot: %s\n", 
                intToHex(brtiMagicBuf).c_str()
            );

            return LibnxError_BadInput;
        }

        // Copy BRTI metadata
        BRTI brti;
        uint64_t brtiOffsetInBfres = fileOffset+brtiOffsetInBntx;

        // Require at least 0x78 bytes for BRTI struct
        if (brtiOffsetInBfres - bntx.size < 0x78){
            return LibnxError_BadInput;
        }

        // Unpack BRTI meta (oh boy)
        std::memcpy(&brti.tileMode, &bfres.at(brtiOffsetInBfres+0x12), sizeof(brti.tileMode));
        std::memcpy(&brti.mipCount, &bfres.at(brtiOffsetInBfres+0x16), sizeof(brti.mipCount));
        std::memcpy(&brti.pixelFormat, &bfres.at(brtiOffsetInBfres+0x1C), sizeof(brti.pixelFormat));
        std::memcpy(&brti.width, &bfres.at(brtiOffsetInBfres+0x24), sizeof(brti.width));
        std::memcpy(&brti.height, &bfres.at(brtiOffsetInBfres+0x28), sizeof(brti.height));
        std::memcpy(&brti.depth, &bfres.at(brtiOffsetInBfres+0x2C), sizeof(brti.depth));
        std::memcpy(&brti.textureLayout, &bfres.at(brtiOffsetInBfres+0x34), sizeof(brti.textureLayout));
        std::memcpy(&brti.imgDataSize, &bfres.at(brtiOffsetInBfres+0x50), sizeof(brti.imgDataSize));
        std::memcpy(&brti.channelSelectors, &bfres.at(brtiOffsetInBfres+0x58), sizeof(brti.channelSelectors));
        std::memcpy(&brti.textureNameOffset, &bfres.at(brtiOffsetInBfres+0x60), sizeof(brti.textureNameOffset)); // Relative to BNTX
        std::memcpy(&brti.mipTblOffset, &bfres.at(brtiOffsetInBfres+0x70), sizeof(brti.mipTblOffset)); // Also seemingly relative to BNTX

        stubbed("\t\t-- BRTI --\n\t\t\tWidth: %d, Height: %d, Depth: %d\n\t\t\t"
            "Mip Count: %d\n\t\t\tFormat: %s\n\t\t\tTile Mode: %s\n\t\t\tLayout: %s\n\t\t\t"
            "Selectors: %s\n\t\t\tData Size: %s\n\t\t\tTexture Name Offset: %s\n\t\t\tMip Table Offset: %s\n",
            brti.width, brti.height, brti.depth, brti.mipCount,
            intToHex(brti.pixelFormat).c_str(), intToHex(brti.tileMode).c_str(), intToHex(brti.textureLayout).c_str(),
            intToHex(brti.channelSelectors).c_str(), intToHex(brti.imgDataSize).c_str(), intToHex(brti.textureNameOffset).c_str(),
            intToHex(brti.mipTblOffset).c_str()
        );

        // Require pixel format 0x0B06 and tile mode 0 for inventory icons
        if (brti.pixelFormat != 0x0B06 || brti.tileMode != 0){
            stubbed("[Rune::Romfs::ExtractSbitemIcon] - Pixel format / tile mode are not supported.\n");
            continue;
        }

        // Check that textureNameOffset is within the bounds of the BNTX data
        if (brti.textureNameOffset > bntx.size){
            stubbed("[Rune::Romfs::ExtractSbitemIcon] - Texture Name Offset is outside of valid bounds.\n");
            return LibnxError_BadInput;
        }

        // Check that at least 2 bytes remain
        if (bntx.size - brti.textureNameOffset < 2){
            stubbed("[Rune::Romfs::ExtractSbitemIcon] - Not enough space in BNTX.\n");
            return LibnxError_BadInput;
        }

        // Get texture name info
        uint16_t textureNameLength = 0;
        std::memcpy(&textureNameLength, &bfres.at(fileOffset+brti.textureNameOffset), sizeof(textureNameLength));

        // Ignore zero-length name
        if (textureNameLength == 0){
            continue;
        }

        // Get the name's characters
        char textureName[textureNameLength+1];
        std::memcpy(
            &textureName, 
            &bfres.at(fileOffset+brti.textureNameOffset+2), 
            textureNameLength
        );

        textureName[textureNameLength] = '\0'; // terminate the texture name

        // Assign the texture name
        brti.textureName = std::string(textureName);
        stubbed("\t\t\tTexture Name: %s\n",brti.textureName.c_str());

        // Validate mip data
        if (brti.mipCount == 0){
            continue;
        }

        if (brti.mipTblOffset == 0){
            continue;
        }
        
        if (brti.mipTblOffset > bntx.size){
            return LibnxError_BadInput;
        }

        // At least 8 bytes must remain
        if (bntx.size - brti.mipTblOffset < 8){
            return LibnxError_BadInput;
        }

        // Get the image data offset
        std::memcpy(&brti.imgDataOffset, &bfres.at(fileOffset+brti.mipTblOffset), sizeof(brti.imgDataOffset));

        // Validate the image data range
        if (brti.imgDataOffset == 0){
            continue;
        }

        if (brti.imgDataSize == 0){
            continue;
        }

        if (brti.imgDataSize > bntx.size - brti.imgDataOffset){
            return LibnxError_BadInput;
        }

        // Copy image data
        std::vector<uint8_t> imgData(brti.imgDataSize);

        // Copy bytes into imgData
        for (size_t b = 0; b < brti.imgDataSize; ++b){
            imgData[b] = bfres.at(fileOffset+brti.imgDataOffset+b);
        }

        // Output results
        stubbed("\t\t\tImage Data Offset: %s\n\t\t\tImage Data Size: %s\n\t\t\tActual Data Size: %s\n", 
            intToHex(brti.imgDataOffset).c_str(), intToHex(brti.imgDataSize).c_str(), intToHex(imgData.size()).c_str()
        );

        // Decode image
        // Calculate block height
        uint32_t gobBlockHeight = 1 << (brti.textureLayout & 7); // block height in groups of bytes
        uint32_t pixelBlockHeight = gobBlockHeight*8; // block height in pixels

        // Calculate how many gob columns span the icon and round up
        uint32_t gobColumns = ((brti.width*4)+63)/64;

        // Output vector where decoded (deswizzled) bytes are stored
        std::vector<uint8_t> decodedIcon(brti.width*brti.height*4);

        // Print values
        stubbed("\t\t\tBlock Height: %d\n\t\t\tGOB Columns: %d\n\t\t\tDecoded Size: %d\n",
            gobBlockHeight, gobColumns, static_cast<int>(decodedIcon.size())
        );

        // Deswizzle
        for (size_t y = 0; y < brti.height; ++y){
            // which row of blocks contains the pixel
            uint32_t blockRow = y / pixelBlockHeight;

            for (size_t x = 0; x < brti.width; ++x){
                uint32_t xBytes = x * 4;

                // which column contains the pixel
                uint32_t gobColumn = xBytes / 64;
                // which gob within the column contains the pixel
                uint32_t gobInBlock = (y % pixelBlockHeight) / 8;

                uint32_t gobOffset = (blockRow*gobColumns*gobBlockHeight*512)
                + (gobColumn*gobBlockHeight*512)
                + (gobInBlock * 512);

                // the local x byte position local to the gob
                uint8_t localXBytePos = xBytes % 64;

                // the local row within the gob
                uint8_t localYPos = y % 8; 

                // Calculate the internal byte offset
                uint32_t offsetInGob = (localXBytePos / 32) * 256
                + (localYPos / 2) * 64
                + ((localXBytePos % 32) / 16) * 32
                + (localYPos % 2) * 16
                + localXBytePos % 16;

                // the overall byte offset relative to the whole file
                uint32_t sourceOffset = gobOffset + offsetInGob;

                // before reading, validate
                if (sourceOffset >= imgData.size()){
                    stubbed("[Rune::Romfs::ExtractSbitemIcon] - BRTI image data is malformed or invalid (0)\n");
                    return LibnxError_BadInput;
                }

                // calculate the destination position
                uint32_t destinationOffset = (y * brti.width + x) * 4;

                // validate
                if (destinationOffset >= decodedIcon.size()){
                    stubbed("[Rune::Romfs::ExtractSbitemIcon] - BRTI image data is malformed or invalid (1)\n");
                    return LibnxError_BadInput;
                }

                // copy the swizzled input at sourceOffset to decodedIcon at destinationOffset

                // COPY FOUR BYTES NOT ONE OTHERWISE CURSED
                // decodedIcon[destinationOffset] = imgData.at(sourceOffset);
                std::memcpy(&decodedIcon[destinationOffset], &imgData.at(sourceOffset), 4);
            }
        }

        // Create output directory
        std::filesystem::create_directories(THUMB_PATH);

        // Write the output PNG
        int writeSuccess = stbi_write_png(
            std::string(THUMB_PATH+brti.textureName+".png").c_str(),
            static_cast<int>(brti.width),
            static_cast<int>(brti.height),
            4,
            decodedIcon.data(),
            static_cast<int>(brti.width*4)
        );

        // Write failed
        if (!writeSuccess){
            return LibnxError_IoError;
        }

        // Set decoded to true
        isIconDecoded = true;
    }

    return (isIconDecoded) ? 0 : LibnxError_NotFound;
}

Result Rune::Romfs::DecodeBars(Rune::Romfs::Bars* out, const std::string& path){

    return 0;
}

Result Rune::Romfs::GetAudio(Rune::Romfs::Bars& bars, const std::string& barsPath, const std::string& output){

    return 0;
}


Result Rune::Romfs::MountBotwRomfs(){
    if (Rune::Application::IsEmulator){
        puts("[Rune::Romfs::MountBotwRomfs] - Game data mount failed! romsMountFromCurrentProcess is not implenented in Ryujinx.");
        return LibnxError_IoError; // lol closest thing
    }

    Result rc = romfsMountFromCurrentProcess("botw");
    if (R_FAILED(rc)){
        puts("[Rune::Romfs::MountBotwRomfs] - romfsMountFromCurrentProcess() failed.");
    }
    return rc;
}

Result Rune::Romfs::UnmountBotwRomfs(){
    Result rc = romfsUnmount("botw");
    return rc;
}

/**
 * @brief An old implementation of MountBotwRomfs(), which attempts to allow asset extraction
 * from any execution context. Unfortunately, fsOpenDataStorageByProgramId() does not agree with me or my choices.
 */
Result MountBotwRomfsOld(){
    if (Rune::Application::IsEmulator){
        stubbed("[Rune::Romfs::MountBotwRomfs] - Game data mount failed! fsOpenDataStorageByProgramId is not implenented in Ryujinx.\n");
        return LibnxError_IoError; // lol closest thing
    }

    // Open BotW's data storage to get FsStorage item
    FsStorage botwStorage;

    u64 titleId = BOTW_TITLE_ID;
    Result rc = fsOpenDataStorageByProgramId(&botwStorage, titleId);

    if (R_FAILED(rc)){
        stubbed("[Rune::Romfs::MountBotwRomfs] - Failed to open BotW storage.\n\tTitle ID: %s\n", 
            intToHex(titleId).c_str()
        );

        return rc;
    }

    // Use FsStorage item to open BotW RomFS at botw:/
    rc = romfsMountFromStorage(botwStorage, 0, "botw");
    if (R_FAILED(rc)){
        stubbed("[Rune::Romfs::MountBotwRomfs] - Failed to mount BotW storage.\n");
        fsStorageClose(&botwStorage);

        return rc;
    }

    return rc;
}