// for handling encoding and recompilation
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>

#include <Save/Save.hpp>
#include <Utils/Utils.hpp>
#include <Utils/Logger.hpp>
#include <Config.hpp>

#include <switch.h>

/**
 * oh boy
 */
void Rune::Save::Encode(Rune::Save::RawFile* out, Rune::Save::File* saveFile, std::function<void(size_t cur, size_t max)> onProgress){
    // MOVE HEADER AND PATH
    out->path   = saveFile->path;
    out->header = saveFile->header;

    // MAIN ITERATOR
    size_t currentProgress=0;
    size_t maxProgress=saveFile->entries.size();

    Logger::Writef("Entries: %d", maxProgress);

    for (const auto& entry : saveFile->entries){
        switch (entry.dataType){
            case RuneBool:
            case RuneRevivalBool: {
                // convert the bool to uint32_t
                uint32_t encodedValue = std::get<bool>(entry.value);
                out->entries.emplace_back(entry.id, encodedValue);

                break;
            }

            case RuneS32:
            case RuneRevivalS32: {
                // convert s32 to uint32_t
                uint32_t encodedValue = std::get<int32_t>(entry.value);
                out->entries.emplace_back(entry.id, encodedValue);

                break;
            }

            case RuneF32:{
                // convert F32 to uint32_t
                RawEntry rawEntry;
                rawEntry.id = entry.id;

                float decodedValue = std::get<float>(entry.value);
                std::memcpy(&rawEntry.value, &decodedValue, sizeof(decodedValue));

                out->entries.push_back(rawEntry);
                break;
            }

            case RuneString32:
            case RuneString64:
            case RuneString256: {
                size_t stringSize;
                switch (entry.dataType){
                    case RuneString32:
                        stringSize = 32;
                        break;
                    case RuneString64:
                        stringSize = 64;
                        break;
                    case RuneString256:
                        stringSize = 256;
                        break;
                    default:
                        continue;
                }

                std::string value = std::get<std::string>(entry.value);

                // determine the total number of chunks required
                size_t chunkCount = stringSize / sizeof(uint32_t);
                
                // iterate through and encode each chunk
                for (size_t chunk = 0; chunk < chunkCount; chunk++){
                    uint32_t rawValue = 0;

                    // for each byte in the string, re-encode it
                    for (size_t byte = 0; byte < sizeof(uint32_t); byte++){
                        size_t stringIdx = (chunk*sizeof(uint32_t)) + byte;

                        char c;
                        if (stringIdx < value.size() && stringIdx < stringSize - 1){
                            c = value[stringIdx];
                        } else {
                            c = '\0'; // null term when over string length
                        }

                        // convert the char into an unsigned char
                        // promote the byte to 32 bit integer
                        // shift the bits in the byte into its final position
                        // merge this result with rawValue
                        rawValue |= static_cast<uint32_t>(static_cast<unsigned char>(c) << (byte * 8));
                    }

                    out->entries.emplace_back(entry.id, rawValue);
                }

                break;
            }

            case RuneVector2f:
            case RuneVector3f:
            case RuneVector4f: {
                size_t vectorSize;
                switch (entry.dataType){
                    case RuneVector2f:
                        vectorSize = 2;
                        break;
                    case RuneVector3f:
                        vectorSize = 3;
                        break;
                    case RuneVector4f:
                        vectorSize = 4;
                        break;
                    default:
                        continue;
                }

                // Get the original decoded vector
                std::vector decodedValue = std::get<std::vector<float>>(entry.value);

                // Iterate through the decoded vector
                for (size_t componentIdx = 0; 
                    componentIdx < vectorSize; componentIdx++){
                    // Create a new entry for each component
                    RawEntry rawEntry;
                    rawEntry.id = entry.id;
                    
                    // Get the original decoded component
                    float decodedComponent = decodedValue.at(componentIdx);

                    // Move the decoded component into the uint32_t value of the encoded raw entry
                    std::memcpy(&rawEntry.value, &decodedComponent, sizeof(decodedComponent));

                    // Add the new component entry to the list of encoded raw entries
                    out->entries.push_back(rawEntry);
                }

                break;
            }

            case RuneString64Array:
            case RuneString256Array: {
                size_t stringSize = 0;
                switch (entry.dataType){
                    case RuneString64Array: {
                        stringSize = 64;
                        break;
                    }
                    case RuneString256Array: {
                        stringSize = 256;
                        break;
                    }
                    default: {
                        // unhandled
                        break;
                    }
                }

                // if nothing makes sense at all, break
                if (stringSize == 0){
                    break;
                }

                std::vector<std::string> values = 
                    std::get<std::vector<std::string>>(entry.value);

                // go through each string and encode it
                for (const std::string &toEncode : values){
                    size_t chunkCount = stringSize / sizeof(uint32_t);

                    for (size_t chunk = 0; chunk < chunkCount; chunk++){
                        uint32_t rawValue = 0;

                        for (size_t byte = 0; byte < sizeof(uint32_t); byte++){
                            size_t stringIdx = (chunk*sizeof(uint32_t)) + byte;

                            char c;
                            // always verify last byte is null term in case string is truncated
                            if (stringIdx < toEncode.size() && stringIdx < stringSize - 1) {
                                c = toEncode[stringIdx];
                            } else {
                                c = '\0';
                            }

                            rawValue |= static_cast<uint32_t>(static_cast<unsigned char>(c)) << (byte * 8);
                        }

                        // create a new entry for each chunk in the string
                        out->entries.emplace_back(entry.id, rawValue);
                    }
                }
                
                break;
            }

            case RuneVector2fArray:
            case RuneVector3fArray: {
                size_t vectorSize = 0;
                switch (entry.dataType){
                    case RuneVector2fArray: {
                        vectorSize = 2;
                        break;
                    }
                    case RuneVector3fArray: {
                        vectorSize = 3;
                        break;
                    }
                    default: {
                        break;
                    }
                }  

                // just to make the compiler shut up <3
                if (vectorSize == 0){
                    break;
                }

                const auto& values = std::get<std::vector<std::vector<float>>>(entry.value);

                for (const auto& toEncode : values){
                    for (size_t componentIdx = 0; componentIdx < vectorSize; componentIdx++){
                        RawEntry rawEntry;
                        rawEntry.id = entry.id;
                        float decodedComponent = toEncode.at(componentIdx);
                        std::memcpy(&rawEntry.value, &decodedComponent, sizeof(decodedComponent));
                        out->entries.push_back(rawEntry);
                    }
                }

                break;
            }

            case RuneS32Array: {
                const auto& values =
                    std::get<std::vector<int32_t>>(entry.value);

                for (int32_t toEncode : values){
                    out->entries.emplace_back(entry.id, 
                        static_cast<uint32_t>(toEncode));
                }

                break;
            }

            case RuneF32Array: {
                const auto& values = 
                    std::get<std::vector<float>>(entry.value);
                
                for (float toEncode : values){
                    RawEntry rawEntry;
                    rawEntry.id = entry.id;
                    std::memcpy(&rawEntry.value, &toEncode, sizeof(toEncode));
                    out->entries.push_back(rawEntry);
                }
                break;
            }

            case RuneBoolArray: {
                const auto& values = 
                    std::get<std::vector<uint8_t>>(entry.value);

                for (uint8_t toEncode : values){
                    out->entries.emplace_back(
                        entry.id,
                        static_cast<uint32_t>(toEncode)
                    );
                }

                break;
            }

            default: {
                // out->entries.emplace_back(
                //     entry.id,
                //     static_cast<uint32_t>(entry.value) ich bin stupid
                // )
                // just do nothing
                break;
            }
        }

        currentProgress++;
        onProgress(currentProgress, maxProgress-1);
    }

    // return out;
}

// Also stores a backup
Result Rune::Save::CompileTo(RawFile* rawSaveFile, const std::string &outputPath, std::function<void(size_t cur, size_t max)> onProgress){
    // Step 1: Get the existing file at outputPath
    if (std::filesystem::exists(outputPath)){
        time_t now = time(nullptr);
        tm* localTime = localtime(&now);

        char buf[17];
        strftime(buf, sizeof(buf), "%d.%m.%Y_%H.%M", localTime);
        std::string destPath = BACKUP_PATH+"game_data.sav."+"BAK_"+std::string(buf);
        Logger::Write("[Rune::Save::CompileTo] - Destination path: "+destPath);

        Logger::Write("[Rune::Save::CompileTo] - Creating directories...");
        std::filesystem::create_directories(BACKUP_PATH);

        Logger::Write("[Rune::Save::CompileTo] - Provided output path: "+outputPath);

        // move the file across from the original save path to the backup path
        Logger::Write("[Rune::Save::CompileTo] - Copying file from original path to backup path...");
        std::ifstream src(outputPath, std::ios::binary);
        std::ofstream dst(destPath, std::ios::binary | std::ios::trunc);

        if (!src.is_open() || !dst.is_open()){
            Logger::Write("[Rune::Save::CompileTo] - Failed to open files!");
            return LibnxError_IoError;
        }

        dst << src.rdbuf();
        dst.flush();

        if (!dst){
            Logger::Write("[Rune::Save::CompileTo] - Failed to write backup!");
            return LibnxError_IoError;
        }

        // Logger::Write("[Rune::Save::CompileTo] - Backup moved! Removing original file...");
        // std::filesystem::remove(outputPath);
    }

    // Step 2: Build a new save file
    Logger::Write("[Rune::Save::CompileTo] - Opening new save file...");
    std::ofstream sav;
    sav.open(outputPath, std::ofstream::binary);

    if (!sav.is_open()){
        Logger::Write("[Rune::Save::CompileTo] - Failed to open new save file!");
        return LibnxError_IoError;
    }

    // Everything is written in 4-byte chunks

    // Write the header
    Logger::Write("[Rune::Save::CompileTo] - Writing header...");
    Rune::Save::Header header = rawSaveFile->header;

    Logger::Write("[Rune::Save::CompileTo] - Header details: "+std::to_string(header.version)+", "+std::to_string(header.marker)+", "+std::to_string(header.formatVersion));
    sav.write(reinterpret_cast<const char*>(&header.version), sizeof(header.version));
    sav.write(reinterpret_cast<const char*>(&header.marker), sizeof(header.marker));
    sav.write(reinterpret_cast<const char*>(&header.formatVersion), sizeof(header.formatVersion));

    // Write the entires
    Logger::Write("[Rune::Save::CompileTo] - Writing entries...");

    size_t currentProgress = 0;
    size_t maxProgress = rawSaveFile->entries.size();
    for (const auto& rawEntry : rawSaveFile->entries){
        sav.write(reinterpret_cast<const char*>(&rawEntry.id), sizeof(rawEntry.id));
        sav.write(reinterpret_cast<const char*>(&rawEntry.value), sizeof(rawEntry.value));

        currentProgress++;
        onProgress(currentProgress, maxProgress-1);
    }

    // Write the padding bytes
    Logger::Write("[Rune::Save::CompileTo] - Writing padding...");
    uint32_t padding = 0xFFFFFFFF;
    sav.write(reinterpret_cast<const char*>(&padding), sizeof(uint32_t));

    // Close the file
    Logger::Write("[Rune::Save::CompileTo] - Closing new file...");
    sav.close();

    // COMMIT CHANGES
    Result rc = fsdevCommitDevice("save");
    if (R_FAILED(rc)){
        return rc;
    }

    // All is well
    Logger::Write("[Rune::Save::CompileTo] - All is well.");
    return 0;
}