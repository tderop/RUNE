#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++20-extensions"

#include <switch.h>
#include <dirent.h>
#include <string>
#include <fstream>
#include <cstring>
#include <sstream>
#include <type_traits>

#include <Save/Save.hpp>
#include <Utils/Utils.hpp>
#include <Utils/Logger.hpp>
#include <Config.hpp>

#include <Json/Json.hpp>

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
        Logger::Write("[Rune::Save::GetSlots] - Failed to mount save data! Mounting test save; may be emulation development environment.");
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
Result Rune::Save::GetSaveRawFile(Rune::Save::RawFile* out, std::string filePath){
    std::ifstream saveFile(filePath, std::ios::binary);
    
    if (!saveFile.is_open()){
        Logger::Write("Failed to open save file path. ('"+filePath+"')");
        return LibnxError_NotFound;
    }

    out->path = filePath;

    // get the header
    Rune::Save::Header header;
    Result rc = Rune::Save::GetSaveHeader(&header, filePath);
    if (R_FAILED(rc)){
        Logger::Write("Failed to get save header.");
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
            Logger::Write("[SAVE] - Raw read finished, cursor stopped at position: "+std::to_string(pos));
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
Result Rune::Save::Decode(Rune::Save::File* out, const Rune::Save::RawFile& raw, const Rune::Save::Metadata& metadata, std::function<void(size_t cur, size_t max)> onProgress){
    // move trivial values
    out->path   = raw.path;
    out->header = raw.header;

    size_t maxProgress = raw.entries.size();

    for (size_t i = 0; i < maxProgress; i++){
        onProgress(i, maxProgress);

        const Rune::Save::RawEntry& rawEntry = raw.entries[i];
        Rune::Save::Entry entry;
        entry.id = rawEntry.id;

        // get the meta entry
        auto it = metadata.find(entry.id);
        if (it == metadata.end()){
            Logger::Write(
                "[DECODE] - Unknown ID: " +
                std::to_string(rawEntry.id) +
                " Value: " +
                std::to_string(rawEntry.value)
            );
            continue;
        }

        MetaEntry metaEntry = it->second;

        // assign trivial values
        entry.dataName = metaEntry.dataName;
        entry.dataType = metaEntry.dataType;

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

            // vectors
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
                std::vector<uint8_t> values;

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
            std::is_same_v<T, int32_t>
        ) {
            std::string str = std::to_string(value);
            size_t start = (value < 0) ? 1 : 0;
            for (size_t i = str.size(); i > start + 3; i -= 3){
                str.insert(i-3, 1, ',');
            }

            stream << str;
        } 
        else if constexpr(
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
 * @note Refactored 10.11.2026 to use my own JSON interpretation
 */
Result Rune::Save::LoadMetadata(Rune::Save::Metadata* out){
    // Get the meta json
    Rune::Json* metaJson = nullptr;
    Rune::JsonData::GetJsonById(metaJson, "Metadata");

    // If failed, return
    if (metaJson == nullptr){
        return -1;
    }

    // Iterate through and populate metadata
    for (const auto&[_, obj] : metaJson->at("entries").items()){
        Rune::Save::MetaEntry newEntry;

        // Fill the struct
        auto nameIt = obj.find("dataName");
        if (nameIt != obj.end() && nameIt->is_string()){
            newEntry.dataName = nameIt->get<std::string>();
        }

        auto typeIt = obj.find("dataType");
        if (typeIt != obj.end() && typeIt->is_number()){
            newEntry.dataType = static_cast<Rune::Save::DataType>(typeIt->get<int32_t>());
        }

        auto editorIt = obj.find("editor");
        if (editorIt != obj.end() && editorIt->is_string()){
            std::string v = editorIt->get<std::string>();
            if (!v.empty()){
                newEntry.editor = v;
            }
        }

        auto idIt = obj.find("id");
        if (idIt != obj.end() && idIt->is_number()){
            newEntry.id = idIt->get<uint32_t>();
        }

        // Helper for reading init, min, and max values
        auto readValue = [&obj, type = newEntry.dataType](const char* key)
            -> std::optional<Rune::Save::SaveValue> {
            const auto& json = obj.at(key);

            // Check if false (meaning no/empty value)
            if (json.is_boolean() && json.get<bool>() == false){
                return false;
            }

            switch (type) {
                case RuneBool:
                case RuneRevivalBool:
                    return json.get<bool>();

                case RuneS32:
                case RuneRevivalS32:
                    return json.get<int32_t>();

                case RuneF32:
                    return json.get<float>();

                case RuneString32:
                case RuneString64:
                case RuneString256:
                    return json.get<std::string>();

                case RuneBoolArray: {
                    std::vector<uint8_t> values;
                    for (const auto& item : json)
                        values.push_back(item.get<bool>());
                    return values;
                }

                case RuneS32Array:
                    return json.get<std::vector<int32_t>>();

                case RuneF32Array:
                case RuneVector2f:
                case RuneVector3f:
                case RuneVector4f:
                    return json.get<std::vector<float>>();

                case RuneString64Array:
                case RuneString256Array:
                    return json.get<std::vector<std::string>>();

                case RuneVector2fArray:
                case RuneVector3fArray:
                    return json.get<std::vector<std::vector<float>>>();

                default:
                    return std::nullopt;
            }
        };

        auto init = readValue("initValue");
        auto min = readValue("minValue");
        auto max = readValue("maxValue");

        if (!init.has_value() || !min.has_value() || !max.has_value()){
            return LibnxError_BadInput;
        }

        newEntry.initValue = init.value();
        newEntry.minValue = min.value();
        newEntry.maxValue = max.value();

        // Add the entry to the out
        if (newEntry.id != 0){
            (*out)[newEntry.id] = newEntry;
        }
    }

    runeMetadata = out;
    return 0;
}

/**
 * @brief Get the meta entry for a save value by its ID.
 * It's recommended to initialise your out MetaEntry to nullptr in the case that the entry is not found, guards can be developed.
 */
Result Rune::Save::GetMetaEntry(const Rune::Save::Metadata& metadata, uint32_t id, MetaEntry* outMetaEntry){
    Logger::Write("[Rune::Save::GetMetaEntry] - Getting hashValue from ID...");
    int32_t hashValue = static_cast<int32_t>(id);

    Logger::Write("[Rune::Save::GetMetaEntry] - Finding ID in metadata...");
    auto it = metadata.find(id);

    if (it == metadata.end()){
        Logger::Write("[Rune::Save::GetMetaEntry] - ID "+std::to_string(id)+" does not exist in metadata!");
        return LibnxError_NotFound;
    }

    Logger::Write("[Rune::Save::GetMetaEntry] - Setting out value...");
    *outMetaEntry = it->second;

    Logger::Write("[Rune::Save::GetMetaEntry] - Retrieved entry successfully.");
    return 0;
}

/**
 * @brief Get the DataName from an ID
 */
Result Rune::Save::GetDataName(const Rune::Save::Metadata& metadata, uint32_t id, std::string* outDataName){
    // get meta entry
    MetaEntry metaEntry;
    Result rc = Rune::Save::GetMetaEntry(metadata, id, &metaEntry);
    if (R_FAILED(rc)){
        return rc;
    }

    // set data name
    *outDataName = metaEntry.dataName;
    return 0;
}

/**
 * @brief Get the data type of a save entry by its ID.
 */
Result Rune::Save::GetDataType(const Rune::Save::Metadata& metadata, uint32_t id, Rune::Save::DataType* outDataType){
    // get meta entry
    MetaEntry metaEntry;
    Result rc = Rune::Save::GetMetaEntry(metadata, id, &metaEntry);
    if (R_FAILED(rc)){
        return rc;
    }

    // set data name
    *outDataType = metaEntry.dataType;
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
    // Logger::Write("[Rune::Save] - Getting entry from dataName '"+dataName+"'...");


    for (Rune::Save::Entry& saveEntry : saveFile.entries){
        if (saveEntry.dataName == dataName){
            // Logger::Write("[Rune::Save] - Entry found for key '"+dataName+"'! Returning entry.");
            return &saveEntry;
        }
    }

    Logger::Write("[Rune::Save] - No value in save data found for key '"+dataName+"'. Returning null.");
    return nullptr;
}

std::string Rune::Save::GetReadableKeyName(std::string& saveKey){
    Logger::Write("[Rune::Save::GetReadableKeyName] - Getting JSON data...");
    Rune::Json* editorDataJson = nullptr;
    if (R_FAILED(Rune::JsonData::GetJsonById(editorDataJson, "EditorData"))){
        Logger::Write("[Rune::Save::GetReadableKeyName] - Failed to load EditorData JSON.");
        return saveKey;
    }

    if (editorDataJson == nullptr){
        Logger::Write("[Rune::Save::GetReadableKeyName] - EditorData JSON is nullptr.");
        return saveKey;
    }

    Logger::Write("[Rune::Save::GetReadableKeyName] - Got EditorData JSON!");

    auto it = editorDataJson->find(saveKey);
    if (it == editorDataJson->end()){
        Logger::Write("[Rune::Save::GetReadableKeyName] - Failed to find save key '"+saveKey+"' in EditorData!");
        return saveKey;
    }

    if (!it->is_object()){
        Logger::Write("[Rune::Save::GetReadableKeyName] - Entry in EditorData for '"+saveKey+"' is not an object!");
        return saveKey;
    }

    auto readableName = it->find("Readable");
    if (readableName != it->end() && readableName->is_string()){
        Logger::Write("[Rune::Save::GetReadableKeyName] - Found readable name for '"+saveKey+"'!");
        return readableName->get<std::string>();
    }

    return saveKey;
}

std::string Rune::Save::GetKeyDevNote(std::string& saveKey){
    Rune::Json* editorDataJson = nullptr;
    if (R_FAILED(Rune::JsonData::GetJsonById(editorDataJson, "EditorData"))){
        return "";
    }

    if (editorDataJson == nullptr){
        return "";
    }

    auto it = editorDataJson->find(saveKey);
    if (it == editorDataJson->end()){
        return "";
    }

    if (!it->is_object()){
        return "";
    }

    auto devNoteEntry = it->find("DevNote");
    if (devNoteEntry != it->end() && devNoteEntry->is_string()){
        return devNoteEntry->get<std::string>();
    }

    return "";
}

std::string Rune::Save::GetKeyDescription(std::string& saveKey){
    Rune::Json* editorDataJson = nullptr;
    if (R_FAILED(Rune::JsonData::GetJsonById(editorDataJson, "EditorData"))){

        return "";
    }

    if (editorDataJson == nullptr){
        return "";
    }

    auto it = editorDataJson->find(saveKey);
    if (it == editorDataJson->end()){
        return "";
    }

    if (!it->is_object()){
        return "";
    }

    auto descriptionEntry = it->find("Description");
    if (descriptionEntry != it->end() && descriptionEntry->is_string()){
        return descriptionEntry->get<std::string>();
    }

    return "";
}

std::string Rune::Save::GetReadableLocationName(const std::string& locationId){
    // Get json
    Rune::Json* locationJson;
    Rune::JsonData::GetJsonById(locationJson, "LocationNames");

    // Search json
    auto entry = locationJson->find(locationId);
    if (entry != locationJson->end() && entry->is_string()){
        return entry->get<std::string>();
    }

    // Default to input if no location name was found
    return locationId;
}

#pragma GCC diagnostic pop