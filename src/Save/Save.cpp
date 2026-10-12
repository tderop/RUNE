#include <switch.h>
#include <dirent.h>
#include <string>
#include <fstream>
#include <cstring>
#include <sstream>
#include <unordered_map>

#include <Save/Save.hpp>
#include <Utils/Utils.hpp>
#include <Config.hpp>
#include <byml/byml.h>

// the save data meta files
#include "0_bgsvdata.h"
#include "1_bgsvdata.h"
#include "2_bgsvdata.h"
#include "3_bgsvdata.h"
#include "4_bgsvdata.h"
#include "5_bgsvdata.h"
#include "6_bgsvdata.h"
#include "7_bgsvdata.h"

// files containing information about the datanames
#include "bool_array_data_0_bgdata.h"
#include "bool_data_0_bgdata.h"
#include "bool_data_1_bgdata.h"
#include "bool_data_2_bgdata.h"
#include "f32_array_data_0_bgdata.h"
#include "f32_data_0_bgdata.h"
#include "revival_bool_data_0_bgdata.h"
#include "revival_bool_data_1_bgdata.h"
#include "revival_bool_data_2_bgdata.h"
#include "revival_bool_data_3_bgdata.h"
#include "revival_bool_data_4_bgdata.h"
#include "revival_bool_data_5_bgdata.h"
#include "revival_bool_data_6_bgdata.h"
#include "revival_bool_data_7_bgdata.h"
#include "revival_s32_data_0_bgdata.h"
#include "s32_array_data_0_bgdata.h"
#include "s32_data_0_bgdata.h"
#include "string32_data_0_bgdata.h"
#include "string64_array_data_0_bgdata.h"
#include "string64_data_0_bgdata.h"
#include "string256_array_data_0_bgdata.h"
#include "string256_data_0_bgdata.h"
#include "vector2f_array_data_0_bgdata.h"
#include "vector2f_data_0_bgdata.h"
#include "vector3f_array_data_0_bgdata.h"
#include "vector3f_data_0_bgdata.h"
#include "vector4f_data_0_bgdata.h"

Rune::Save::Metadata* runeMetadata = nullptr;
const char* testSavePath = "sdmc:/TestSave/";

std::string Rune::Save::GetVersionString(Rune::Save::Version version){
    switch (version){
        case RuneSaveVersion_100:
            return "1.0.0";

        case RuneSaveVersion_110:
            return "1.1.0";

        case RuneSaveVersion_120:
            return "1.2.0";
        
        case RuneSaveVersion_130:
            return "1.3.0";
        
        case RuneSaveVersion_131:
            return "1.3.1";
        
        case RuneSaveVersion_133:
            return "1.3.3";
        
        case RuneSaveVersion_134:
            return "1.3.4";
        
        case RuneSaveVersion_140:
            return "1.4.0 / 1.4.1";
        
        case RuneSaveVersion_150:
            return "1.5.0";

        case RuneSaveVersion_160:
            return "1.6.0";
    }

    return "Unknown";
}

Result Rune::Save::GetSaveHeader(Rune::Save::Header* out, const std::string& savePath){
    std::ifstream saveFile(savePath, std::ios::binary);
    
    // check if the file successfully opened
    bool fileOpen = saveFile.is_open();
    if (!fileOpen){ 
        return LibnxError_NotFound;
    }

    // unpack the save header
    if (!saveFile.read(reinterpret_cast<char*>(out), sizeof(Rune::Save::Header))){
        return LibnxError_IoError;
    }

    return 0;
}

/**
 * @brief mounts the BotW save data to the "save:/" system
 */
Result Rune::Save::MountSaveData(){
    AccountUid userID;
    Result rc = getUserID(&userID);
    if (R_FAILED(rc)){
        return rc;
    }

    // attempt to mount normal save
    rc = fsdevMountSaveData("save", BOTW_TITLE_ID, userID);
    if (R_FAILED(rc)){
        // attempt to mount another location to save
        // stub for now
    }

    return rc;
}

/**
 * @brief Unpacks all of the save slots in a BotW save file.
 */
Result Rune::Save::GetSaveSlots(std::vector<Rune::Save::Slot>* out){
    // open the directory
    DIR* dir = opendir("save:/");
    std::string thisPath = "save:/";

    if (dir == nullptr){
        // return LibnxError_NotFound;
        dir = opendir(testSavePath);
        thisPath = testSavePath;
    }

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr){
        // create a new save slot
        if (entry->d_type == DT_DIR){
            if (entry->d_name[0] >= '0' && entry->d_name[0] <= '7'){
                // get the name of the slot
                int slotIdx = std::stoi(entry->d_name);

                // build the Rune::Save::Slot
                Rune::Save::Slot saveSlot;

                saveSlot.index           = slotIdx;
                saveSlot.path            = thisPath+std::string(entry->d_name);
                saveSlot.gameDataPath    = saveSlot.path+"/game_data.sav";
                saveSlot.captionDataPath = saveSlot.path+"/caption.sav";
                saveSlot.captionImgPath  = saveSlot.path+"/caption.jpg";

                out->push_back(saveSlot);
            }
        }
    }

    closedir(dir);
    return 0;
}

/**
 * @brief Get a Rune::Save::RawFile containing all entries from a provided save file path.
 */
Result Rune::Save::GetSaveRawFile(Rune::Save::RawFile* out, std::string& filePath){
    std::ifstream saveFile(filePath, std::ios::binary);
    
    if (!saveFile.is_open()){
        log("Failed to open save file path. ('"+filePath+"')");
        return LibnxError_NotFound;
    }

    // get the header
    Rune::Save::Header header;
    Result rc = Rune::Save::GetSaveHeader(&header, filePath);
    if (R_FAILED(rc)){
        log("Failed to get save header.");
        return rc;
    }

    out->header = header;

    // seek to position 12 from the beginning
    saveFile.seekg(12, std::ios::beg);
    
    int pos = 12;

    while (true){
        Rune::Save::RawEntry entry;

        // usually 4 bytes off the end, last 4 bytes are always 0xFF 0xFF 0xFF 0xFF
        if (!saveFile.read(reinterpret_cast<char*>(&entry), 8)){
            log("[SAVE] - Raw read finished, cursor stopped at position: "+std::to_string(pos));
            break;
        }
        
        pos += 8;
        out->entries.push_back(entry);
    }

    return 0;
}

/**
 * @brief a helper for decoding rune save values. Requires context array. 
 */
// Rune::Save::SaveValue Rune::Save::DecodeValue(Rune::Save::DataType dataType, Rune::Save::RawFile& rawFileContext, int* i){
//     // stub
//     // this is where unusedfunction waring is
//     return 1;
// }

/**
 * @brief Decode a raw save file.
 */
Result Rune::Save::Decode(Rune::Save::File* out, const Rune::Save::RawFile& raw, const Rune::Save::Metadata& metadata){
    // move trivial values
    out->header = raw.header;

    for (size_t i = 0; i < raw.entries.size(); i++){
        const Rune::Save::RawEntry& rawEntry = raw.entries[i];

        Rune::Save::Entry entry;
        entry.id = rawEntry.id;

        // data name
        Result rc = Rune::Save::GetDataName(metadata, entry.id, &entry.dataName);
        if (R_FAILED(rc)){
            // unknown ID
            log(
                "[DECODE] - Unknown ID: " +
                std::to_string(rawEntry.id) +
                " Value: " +
                std::to_string(rawEntry.value)
            );
            continue;
        }

        rc = Rune::Save::GetDataType(metadata, entry.id, &entry.dataType);
        if (R_FAILED(rc)){
            log("[DECODE] - [NAME:"+std::string(entry.dataName)+"] failed to get data type (skipping)");
            continue;
        }

        // decode the value
        switch (entry.dataType){
            // trivial case 1
            case RuneBool:
            case RuneRevivalBool:
                entry.value = rawEntry.value != 0;
                break;

            // trivial case 2
            case RuneS32:
            case RuneRevivalS32:
                entry.value = static_cast<int32_t>(rawEntry.value);
                break;
            
            // trivial case 3
            case RuneF32: {
                float value;
                std::memcpy(&value, &rawEntry.value, sizeof(value)); // reinterpret four bytes as IEEE-754
                entry.value = value;
                break;
            }
            
            // evil case 1
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
                        return LibnxError_BadInput; // aka cheats
                }
                
                size_t chunkCount = stringSize / sizeof(uint32_t);

                std::string value;
                value.reserve(stringSize);

                for (size_t chunk = 0; chunk < chunkCount; chunk++) {
                    uint32_t rawValue = raw.entries[i + chunk].value;

                    for (size_t byte = 0; byte < sizeof(uint32_t); byte++) {
                        char c = static_cast<char>(
                            (rawValue >> (byte * 8)) & 0xFF
                        );

                        if (c == '\0')
                            break;

                        value += c;
                    }
                }

                entry.value = std::move(value);
                i += chunkCount - 1;
                break; // bruh
            }

            // vegctgros
            case RuneVector2f:
            case RuneVector3f:
            case RuneVector4f:{
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
                        return LibnxError_BadInput;
                }

                std::vector<float> resultVector(vectorSize);
 
                for (size_t componentIdxOffset = 0;
                        componentIdxOffset < vectorSize;
                        componentIdxOffset++){

                    uint32_t rawValue = raw.entries[i + componentIdxOffset].value;
                    float value;

                    std::memcpy(&value, &rawValue, sizeof(value));
                    resultVector[componentIdxOffset] = value;
                }

                entry.value = std::move(resultVector);
                i += vectorSize - 1;
                break;
            }

            // ultimate evil case
            case RuneString64Array:
            case RuneString256Array: {
                size_t stringSize;
                
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
                        return LibnxError_BadInput;
                    }
                }

                std::vector<std::string> values;
                uint32_t id = rawEntry.id;

                size_t chunkCount = stringSize / sizeof(uint32_t);

                while (i < raw.entries.size() &&
                    raw.entries[i].id == id)
                {
                    if (i + chunkCount > raw.entries.size())
                        return LibnxError_BadInput;

                    std::string value;
                    value.reserve(stringSize);

                    for (size_t chunk = 0; chunk < chunkCount; chunk++) {
                        uint32_t rawValue = raw.entries[i + chunk].value;

                        for (size_t byte = 0; byte < 4; byte++) {
                            char c = static_cast<char>(
                                (rawValue >> (byte * 8)) & 0xFF
                            );

                            if (c == '\0')
                                break;

                            value += c;
                        }
                    }

                    values.push_back(std::move(value));

                    i += chunkCount;
                }

                entry.value = std::move(values);
                i--;

                break;
            }

            // THE evil case
            case RuneVector2fArray:
            case RuneVector3fArray:{
                size_t vectorSize;
                switch (entry.dataType){
                    case RuneVector2fArray:
                        vectorSize = 2;
                        break;

                    case RuneVector3fArray:
                        vectorSize = 3;
                        break;

                    default:
                        return LibnxError_BadInput;
                }

                std::vector<std::vector<float>> values;
                uint32_t id = rawEntry.id;

                while (i < raw.entries.size() && raw.entries[i].id == id){
                    // calculate vector
                    std::vector<float> resultVector(vectorSize);
    
                    for (size_t componentIdxOffset = 0;
                            componentIdxOffset < vectorSize;
                            componentIdxOffset++){

                        uint32_t rawValue = raw.entries[i + componentIdxOffset].value;
                        float value;

                        std::memcpy(&value, &rawValue, sizeof(value));
                        resultVector[componentIdxOffset] = value;
                    }

                    // add vector to values
                    values.push_back(std::move(resultVector));

                    i += vectorSize;
                }

                entry.value = std::move(values);
                i--;
                break;
            }

            // evil case 2
            case RuneS32Array: {
                std::vector<int32_t> values;

                uint32_t id = rawEntry.id;
                
                // keep going until no match
                while (i < raw.entries.size() &&
                    raw.entries[i].id == id)
                {
                    values.push_back(
                        static_cast<int32_t>(raw.entries[i].value)
                    );

                    i++;
                }

                entry.value = std::move(values);
                i--;

                break;
            }

            // evil case 3
            case RuneF32Array: {
                std::vector<float> values;

                uint32_t id = rawEntry.id;

                while (i < raw.entries.size() &&
                    raw.entries[i].id == id)
                {
                    float value;
                    std::memcpy(&value, &raw.entries[i].value, sizeof(value));

                    values.push_back(value);
                    i++;
                }

                entry.value = std::move(values);
                i--;

                break;
            }

            // evil case 4
            case RuneBoolArray: {
                std::vector<bool> values;

                uint32_t id = rawEntry.id;

                while (i < raw.entries.size() &&
                    raw.entries[i].id == id)
                {
                    values.push_back(raw.entries[i].value != 0);
                    i++;
                }

                entry.value = std::move(values);
                i--;

                break;
            }

            default: {
                entry.value = static_cast<int32_t>(rawEntry.value);
            }
        }

        out->entries.push_back(std::move(entry));
    }

    return 0;
}

// std::string serializeVector(std::vector<std::variant<std::string, float, std::vector<float>>>& vector, bool showQuotes){
//     std::ostringstream stream;
//     std::visit([&stream, showQuotes](const auto& value){

//     }, vector);
// }

/**
 * @brief Convert a Rune::Save::Value to a displayable string
 */
std::string Rune::Save::ValueToString(const Rune::Save::SaveValue& value, bool showStringQuotes)
{
    std::ostringstream stream;

    std::visit([&stream, showStringQuotes](const auto& value) {

        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, bool>) {
            stream << (value ? "true" : "false");
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            if (showStringQuotes){
                stream << '"' << value << '"';
            }
            else {
                stream << value;
            }
        }
        else if constexpr (
            std::is_same_v<T, int32_t> ||
            std::is_same_v<T, float>
        ) {
            stream << value;
        }

        // case for strings to give them quotes
        else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            stream << "[";

            for (size_t i = 0; i < value.size(); i++) {
                if (i > 0)
                    stream << ", ";

                if (showStringQuotes){
                    stream << '"' << value[i] << '"';
                }
                else {
                    stream << value[i];
                }
            }

            stream << "]";
        }
        else if constexpr (std::is_same_v<T, std::vector<std::vector<float>>>) {
            stream << "[";

            for (size_t i = 0; i < value.size(); i++) {
                if (i > 0) {
                    stream << ", ";
                }

                stream << "{";

                const auto& innerVector = value[i];

                for (size_t j = 0; j < innerVector.size(); j++) {
                    if (j > 0) {
                        stream << ", ";
                    }

                    stream << innerVector[j];
                }

                stream << "}";
            }

            stream << "]";
        }
        else {
            stream << "[";

            for (size_t i = 0; i < value.size(); i++) {
                if (i > 0)
                    stream << ", ";

                stream << value[i];
            }

            stream << "]";
        }

    }, value);

    return stream.str();
}

/**
 * @brief Unpack save metadata
 */
Result Rune::Save::LoadMetadata(Rune::Save::Metadata* out){
    // all of the meta files
    Rune::Save::BgsvData bgsvFiles[] = {
        {_0_bgsvdata, _0_bgsvdata_size},
        {_1_bgsvdata, _1_bgsvdata_size},
        {_2_bgsvdata, _2_bgsvdata_size},
        {_3_bgsvdata, _3_bgsvdata_size},
        {_4_bgsvdata, _4_bgsvdata_size},
        {_5_bgsvdata, _5_bgsvdata_size},
        {_6_bgsvdata, _6_bgsvdata_size},
        {_7_bgsvdata, _7_bgsvdata_size}
    };

    // all of the data type files
    Rune::Save::BgData bgFiles[] = {
        {bool_array_data_0_bgdata, bool_array_data_0_bgdata_size, RuneBoolArray},
        {bool_data_0_bgdata, bool_data_0_bgdata_size, RuneBool},
        {bool_data_1_bgdata, bool_data_1_bgdata_size, RuneBool},
        {bool_data_2_bgdata, bool_data_2_bgdata_size, RuneBool},

        {f32_array_data_0_bgdata, f32_array_data_0_bgdata_size, RuneF32Array},
        {f32_data_0_bgdata, f32_data_0_bgdata_size, RuneF32},

        {revival_bool_data_0_bgdata, revival_bool_data_0_bgdata_size, RuneRevivalBool},
        {revival_bool_data_1_bgdata, revival_bool_data_1_bgdata_size, RuneRevivalBool},
        {revival_bool_data_2_bgdata, revival_bool_data_2_bgdata_size, RuneRevivalBool},
        {revival_bool_data_3_bgdata, revival_bool_data_3_bgdata_size, RuneRevivalBool},
        {revival_bool_data_4_bgdata, revival_bool_data_4_bgdata_size, RuneRevivalBool},
        {revival_bool_data_5_bgdata, revival_bool_data_5_bgdata_size, RuneRevivalBool},
        {revival_bool_data_6_bgdata, revival_bool_data_6_bgdata_size, RuneRevivalBool},
        {revival_bool_data_7_bgdata, revival_bool_data_7_bgdata_size, RuneRevivalBool},

        {revival_s32_data_0_bgdata, revival_s32_data_0_bgdata_size, RuneRevivalS32},
        {s32_array_data_0_bgdata, s32_array_data_0_bgdata_size, RuneS32Array},
        {s32_data_0_bgdata, s32_data_0_bgdata_size, RuneS32},

        {string32_data_0_bgdata, string32_data_0_bgdata_size, RuneString32},
        {string64_array_data_0_bgdata, string64_array_data_0_bgdata_size, RuneString64Array},
        {string64_data_0_bgdata, string64_data_0_bgdata_size, RuneString64},
        {string256_array_data_0_bgdata, string256_array_data_0_bgdata_size, RuneString256Array},
        {string256_data_0_bgdata, string256_data_0_bgdata_size, RuneString256},
        {vector2f_array_data_0_bgdata, vector2f_array_data_0_bgdata_size, RuneVector2fArray},
        {vector2f_data_0_bgdata, vector2f_data_0_bgdata_size, RuneVector2f},
        {vector3f_array_data_0_bgdata, vector3f_array_data_0_bgdata_size, RuneVector3fArray},
        {vector3f_data_0_bgdata, vector3f_data_0_bgdata_size, RuneVector3f},
        {vector4f_data_0_bgdata, vector4f_data_0_bgdata_size, RuneVector4f}
    };

    // for getting ID
    std::unordered_map<std::string, uint32_t> dataNameToId; 

    for (size_t i = 0; i < sizeof(bgsvFiles) / sizeof(bgsvFiles[0]); i++){
        // log("Parsing bgsvdata file " + std::to_string(i));
        const Rune::Save::BgsvData& file = bgsvFiles[i];

        byml::Reader reader{
            byml::Buffer{file.data, file.size}
        };

        if (!reader.isValid()){
            log("bgsvdata " + std::to_string(i) + ": reader invalid");
            return LibnxError_BadInput;
        }

        auto root = reader.getHash();
        if (!root){
            log("bgsvdata " + std::to_string(i) + ": no root hash");
            return LibnxError_BadInput;
        }

        auto fileListItem = root->getByKey("file_list");
        if (!fileListItem){
            log("bgsvdata " + std::to_string(i) + ": no file list");
            return LibnxError_BadInput;
        }

        auto fileList = fileListItem->getArray();
        if (!fileList){
            log("bgsvdata " + std::to_string(i) + ": file list is not an array");
            return LibnxError_BadInput;
        }

        auto mappingsItem = fileList->getByIndex(1);
        if (!mappingsItem){
            log("bgsvdata " + std::to_string(i) + ": file_list[1] does not exist");
            return LibnxError_BadInput;
        }

        auto mappings = mappingsItem->getArray();
        if (!mappings){
            log("bgsvdata " + std::to_string(i) + ": file_list[1] is not an array");
            return LibnxError_BadInput;
        }

        for (const auto& item : *mappings){
            auto mapping = item.getHash();
            if (!mapping){
                return LibnxError_BadInput;
            }

            auto dataNameItem = mapping->getByKey("DataName");
            auto hashValueItem = mapping->getByKey("HashValue");

            if (!dataNameItem || !hashValueItem){
                return LibnxError_BadInput;
            }

            const char* dataName = dataNameItem->getString();
            auto hashValue = hashValueItem->getInt();

            if (*hashValue == 276498232) {
                log("[META] FOUND 276498232 -> " + std::string(dataName));
            }

            if (!dataName){
                return LibnxError_BadInput;
            }

            out->dataNames[*hashValue] = dataName;
        }
    }

     // get the data types
    for (Rune::Save::BgData bgdata : bgFiles){
        // parse bgdata file
        byml::Reader reader(
            byml::Buffer{bgdata.data, bgdata.size}
        );

        // output datatype
        Rune::Save::DataType dataType = bgdata.dataType;
        const char* dataTypeName = Rune::Save::DataTypeName(dataType);
        
        // get datatype
        if (!reader.isValid()){
            log("bgdata: ["+std::string(dataTypeName)+"] is invalid");
            return LibnxError_BadInput;
        }

        auto root = reader.getHash();
        if (!root){
            log("bgdata: ["+std::string(dataTypeName)+"] no root hash");
            return LibnxError_BadInput;
        }

        auto dataTypeListItem = root->getByKey(dataTypeName);
        if (!dataTypeListItem){
            log("bgdata: ["+std::string(dataTypeName)+"] no data type list item");
            return LibnxError_BadInput;
        }

        // get hashvalue
        auto dataTypeList = dataTypeListItem->getArray();
        if (!dataTypeList){
            log("bgdata: ["+std::string(dataTypeName)+"] no data type list array");
            return LibnxError_BadInput;
        }

        for (const auto& item : *dataTypeList){
            auto mapping = item.getHash();
            if (!mapping){
                return LibnxError_BadInput;
            }

            auto hashValueItem = mapping->getByKey("HashValue");
            if (!hashValueItem){
                return LibnxError_BadInput;
            }

            auto hashValue = hashValueItem->getInt();
            if (!hashValue){
                return LibnxError_BadInput;
            }

            // add datatype to metadata
            out->dataTypes[*hashValue] = dataType;
        }
    }

    runeMetadata = out;
    return 0;
}

/**
 * @brief Get the DataName from an ID
 */
Result Rune::Save::GetDataName(const Rune::Save::Metadata& metadata, uint32_t id, std::string* outDataName){
    // convert unsigned to signed
    int32_t hashValue = static_cast<int32_t>(id);

    // look up in the metadata
    auto it = metadata.dataNames.find(hashValue);
    if (it == metadata.dataNames.end()){
        return LibnxError_NotFound;
    }

    *outDataName = it->second;
    return 0;
}

/**
 * @brief Get the data type of a DataName
 */
Result Rune::Save::GetDataType(const Rune::Save::Metadata& metadata, uint32_t id, Rune::Save::DataType* outDataType){
    int32_t hashValue = static_cast<int32_t>(id);

    auto it = metadata.dataTypes.find(hashValue);
    if (it == metadata.dataTypes.end()){
        return LibnxError_NotFound;
    }

    *outDataType = it->second;
    return 0;
}

/**
 * @brief Gets the name of a Rune::Save::DataType
 */
const char* Rune::Save::DataTypeName(Rune::Save::DataType type) {
    for (const auto& dataType : Rune::Save::DataTypes) {
        if (dataType.type == type) {
            return dataType.name;
        }
    }

    return "unknown";
}

/**
 * @brief Get a Rune::Save::Entry from a Rune::Save::File by the entry's DataName
 */
Rune::Save::Entry* Rune::Save::GetEntryFromDataName(Rune::Save::File& saveFile, const std::string& dataName){
    log("[Rune::Save] - Getting entry from dataName '"+dataName+"'...");


    for (Rune::Save::Entry& saveEntry : saveFile.entries){
        if (saveEntry.dataName == dataName){
            log("[Rune::Save] - Entry found for key '"+dataName+"'! Returning entry.");
            return &saveEntry;
        }
    }

    log("[Rune::Save] - No value in save data found for key '"+dataName+"'. Returning null.");
    return nullptr;
}