#pragma once

#include <switch.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <variant>

// consts
const std::string SAVE_DISTRICT_KEY = "SaveDistrictName";
const std::string SAVE_LOCATION_KEY = "SaveLocationName";

const std::string LAST_SAVE_TIME_UPPER_KEY = "LastSaveTime_Upper";
const std::string LAST_SAVE_TIME_LOWER_KEY = "LastSaveTime_Lower";
const std::string IS_AUTOSAVE_KEY = "IsSaveByAuto";
const std::string GAME_CLEAR_KEY = "GameClear";
const std::string IS_MASTER_MODE_KEY = "AoC_HardMode_Enabled";

// Backup path if failed to mount BotW save data
extern const char* testSavePath;

/**
 * @brief A structure representing the header section of a `.sav` file.
 */
struct RuneSaveHeader {
    uint32_t version;
    uint32_t marker;
    uint32_t formatVersion; // best guess. The meaning of this value is disputed.
};

/**
 * @brief Save file versions
 */
enum RuneSaveVersion{
    RuneSaveVersion_100 = 0x24E2,
    RuneSaveVersion_110 = 0x24EE,
    RuneSaveVersion_120 = 0x2588,
    RuneSaveVersion_130 = 0x29C0,
    RuneSaveVersion_131 = 0x2A46,
    RuneSaveVersion_133 = 0x3EF8,
    RuneSaveVersion_134 = 0x3EF9,
    RuneSaveVersion_140 = 0x471A,
    RuneSaveVersion_141 = 0x471A,
    RuneSaveVersion_150 = 0x471B,
    RuneSaveVersion_160 = 0x471E
};

/**
 * @brief BotW save data types
 */
enum RuneSaveDataType{
    RuneBoolArray = 0x0000,
    RuneBool = 0x0001,

    RuneF32Array = 0x0002,
    RuneF32 = 0x0003,

    RuneRevivalBool = 0x0004,
    RuneRevivalS32 = 0x0005,

    RuneS32Array = 0x0006,
    RuneS32 = 0x0007,
    RuneString32 = 0x0008,
    RuneString64Array = 0x0009,
    RuneString64 = 0x000A,
    RuneString256Array = 0x000B,
    RuneString256 = 0x000C,

    RuneVector2fArray = 0x000D,
    RuneVector2f = 0x000E,
    RuneVector3fArray = 0x000F,
    RuneVector3f = 0x0010,
    RuneVector4f = 0x0011
};

struct RuneSaveDataTypeInfo {
    RuneSaveDataType type;
    const char* name;
};

inline constexpr RuneSaveDataTypeInfo RuneSaveDataTypes[] = {
    { RuneBoolArray, "bool_array_data" },
    { RuneBool, "bool_data" },

    { RuneF32Array, "f32_array_data" },
    { RuneF32, "f32_data" },

    { RuneRevivalBool, "bool_data" },
    { RuneRevivalS32,"s32_data" },

    { RuneS32Array, "s32_array_data" },
    { RuneS32, "s32_data" },
    { RuneString32, "string_data" },
    { RuneString64Array, "string64_array_data" },
    { RuneString64, "string64_data" },
    { RuneString256Array, "string256_array_data" },
    { RuneString256, "string256_data" },

    { RuneVector2fArray, "vector2f_array_data" },
    { RuneVector2f, "vector2f_data" },
    { RuneVector3fArray, "vector3f_array_data" },
    { RuneVector3f, "vector3f_data" },
    { RuneVector4f, "vector4f_data" }
};

/**
 * @brief Gets the name of a RuneSaveDataType
 */
const char* runeSaveDataTypeName(RuneSaveDataType type);

struct RuneBgsvData{
    const uint8_t* data;
    size_t size;
};

struct RuneBgData{
    const uint8_t* data;
    size_t size;
    RuneSaveDataType dataType;
};

struct RuneSaveMetadata{
    std::unordered_map<int32_t, std::string> dataNames;
    std::unordered_map<int32_t, RuneSaveDataType> dataTypes;
};

extern RuneSaveMetadata* runeMetadata;

/**
 * @brief An abstraction representing a BotW save slot
 */
struct RuneSaveSlot{
    int index; // 0-7

    std::string path; // e.g. 'save:/0'
    std::string gameDataPath;
    std::string captionDataPath;
    std::string captionImgPath;
};

/**
 * @brief An abstraction of a raw entry in a `.sav` file.
 */
struct RuneSaveRawEntry{
    uint32_t id;
    uint32_t value;
};
static_assert(sizeof(RuneSaveRawEntry)==8); // the size of a save entry MUST be 8

using RuneSaveValue = std::variant<
    bool,
    int32_t,
    float,
    std::string,

    std::vector<bool>,
    std::vector<int32_t>,
    std::vector<float>, // vectors
    std::vector<std::string>,

    std::vector<std::vector<float>> // vector arrays
>;
/** 
 * @brief An abstraction of a decoded entry in a `.sav` file.
 */
struct RuneSaveEntry{
    int32_t id;
    std::string dataName;
    RuneSaveDataType dataType;
    RuneSaveValue value;
};

/**
 * @brief An abstraction of a raw `.sav` file.
 */
struct RuneSaveRawFile{
    RuneSaveHeader header;
    std::vector<RuneSaveRawEntry> entries;
};

/**
 * @brief An abstraction of a decoded `.sav` file
 */
struct RuneSaveFile{
    RuneSaveHeader header;
    std::vector<RuneSaveEntry> entries;
};

/**
 * @brief Decode a raw save file.
 */
Result runeSaveDecode(RuneSaveFile* out, const RuneSaveRawFile& raw, const RuneSaveMetadata& metadata);

/**
 * @brief Convert a RuneSaveValue to a displayable string
 */
std::string runeSaveValueToString(const RuneSaveValue& value, bool showStringQuotes);

/**
 * @brief Get the save version string from a RuneSaveVersion
 */
std::string runeSaveGetVersionString(RuneSaveVersion saveVersion);

/**
 * @brief Get the BotwSaveHeader from the `.sav` file path.
 */
Result runeSaveGetSaveHeader(RuneSaveHeader* out, const std::string& savePath);

/**
 * @brief Mount the current user's BotW save data as a filesystem at "save:/"
 */
Result runeSaveMountSaveData();

/**
 * @brief Unpacks all of the save slots in a BotW save file.
 */
Result runeSaveGetSaveSlots(std::vector<RuneSaveSlot>* out);

/**
 * @brief Get a RuneSaveRawFile containing all entries from a provided save file path.
 */
Result runeSaveGetSaveRawFile(RuneSaveRawFile* out, std::string& filePath);

/**
 * @brief Get the RuneSaveFile from a RuneSaveRawFile
 */
Result runeSaveGetSaveFile(RuneSaveFile* out, RuneSaveRawFile rawSaveFile);

/**
 * @brief Preload the save metadata for understanding data IDs in a save file
 */
Result runeSaveLoadMetadata(RuneSaveMetadata* out);

/**
 * @brief Get the DataName from an ID
 */
Result runeSaveGetDataName(const RuneSaveMetadata& metadata, uint32_t id, std::string* outDataName);

/**
 * @brief Get the data type of a DataName
 */
Result runeSaveGetDataType(const RuneSaveMetadata& metadata, uint32_t id, RuneSaveDataType* outDataType);

/**
 * @brief Get a RuneSaveEntry from a RuneSaveFile by the entry's DataName
 */
RuneSaveEntry* runeSaveGetEntryFromDataName(RuneSaveFile& saveFile, const std::string& dataName);