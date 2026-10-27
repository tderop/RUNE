#pragma once

#include <switch.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <variant>
#include <byml/value.h>

// consts
const std::string SAVE_DISTRICT_KEY = "SaveDistrictName";
const std::string SAVE_LOCATION_KEY = "SaveLocationName";

const std::string LAST_SAVE_TIME_UPPER_KEY = "LastSaveTime_Upper";
const std::string LAST_SAVE_TIME_LOWER_KEY = "LastSaveTime_Lower";
const std::string IS_AUTOSAVE_KEY = "IsSaveByAuto";
const std::string GAME_CLEAR_KEY = "GameClear";
const std::string IS_MASTER_MODE_KEY = "AoC_HardMode_Enabled";

// Backup path if failed to mount BotW save data
extern const char *testSavePath;

namespace Rune
{
    namespace Save
    {
        /**
         * @brief A structure representing the header section of a `.sav` file.
         */
        struct Header
        {
            uint32_t version;
            uint32_t marker;
            uint32_t formatVersion; // best guess. The meaning of this value is disputed.
        };

        /**
         * @brief Save file versions
         */
        enum Version
        {
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
        enum DataType
        {
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
        
        struct DataTypeInfo
        {
            DataType type;
            const char *name;
        };

        inline constexpr DataTypeInfo DataTypes[] = {
            {RuneBoolArray, "bool_array_data"},
            {RuneBool, "bool_data"},

            {RuneF32Array, "f32_array_data"},
            {RuneF32, "f32_data"},

            {RuneRevivalBool, "bool_data"},
            {RuneRevivalS32, "s32_data"},

            {RuneS32Array, "s32_array_data"},
            {RuneS32, "s32_data"},
            {RuneString32, "string_data"},
            {RuneString64Array, "string64_array_data"},
            {RuneString64, "string64_data"},
            {RuneString256Array, "string256_array_data"},
            {RuneString256, "string256_data"},

            {RuneVector2fArray, "vector2f_array_data"},
            {RuneVector2f, "vector2f_data"},
            {RuneVector3fArray, "vector3f_array_data"},
            {RuneVector3f, "vector3f_data"},
            {RuneVector4f, "vector4f_data"}
        };

        /**
         * @brief Gets the name of a RuneSaveDataType
         */
        const char *DataTypeName(DataType type);

        struct BgsvData
        {
            const uint8_t *data;
            size_t size;
        };

        struct BgData
        {
            const uint8_t *data;
            size_t size;
            DataType dataType;
        };

        /**
         * @brief An abstraction representing a BotW save slot
         */
        struct Slot
        {
            int index; // 0-7

            std::string path; // e.g. 'save:/0'
            std::string gameDataPath;
            std::string captionDataPath;
            std::string captionImgPath;
        };

        /**
         * @brief An abstraction of a raw entry in a `.sav` file.
         */
        struct RawEntry
        {
            uint32_t id;
            uint32_t value;

            RawEntry(uint32_t id_, uint32_t value_): id(id_), value(value_) {}
            RawEntry() = default;
        };
        static_assert(sizeof(RawEntry) == 8); // the size of a save entry MUST be 8

        using SaveValue = std::variant<
            bool,
            int32_t,
            float,
            std::string,

            std::vector<uint8_t>, // bool vectors (thank you so much C++ for being so amazing)
            std::vector<int32_t>,
            std::vector<float>, // vectors
            std::vector<std::string>,

            std::vector<std::vector<float>> // vector arrays

            // meta types
            // byml::ItemData::Variant
        >;

        struct MetaEntry {
            uint32_t id;
            std::string dataName;
            std::string editor;
            DataType dataType;

            SaveValue initValue;
            SaveValue maxValue;
            SaveValue minValue;
        };

        // struct Metadata
        // {
        //     std::unordered_map<int32_t, std::string> dataNames;
        //     std::unordered_map<int32_t, DataType> dataTypes;
        // };

        using Metadata = std::unordered_map<int32_t, MetaEntry>;

        /**
         * @brief An abstraction of a decoded entry in a `.sav` file.
         */
        struct Entry
        {
            uint32_t id;
            std::string dataName;
            DataType dataType;
            SaveValue value;

            // reduce memory footage by making access methods
            // SaveValue initValue;
            // SaveValue maxValue; // for building bounds checks
            // SaveValue minValue;
        };

        /**
         * @brief An abstraction of a raw `.sav` file.
         */
        struct RawFile
        {
            std::string path;
            Header header;
            std::vector<RawEntry> entries;
        };

        /**
         * @brief An abstraction of a decoded `.sav` file
         */
        struct File
        {
            std::string path;
            Header header;
            std::vector<Entry> entries;
        };

        /**
         * @brief Decode a raw save file.
         */
        Result Decode(File *out, const RawFile &raw, const Metadata &metadata);

        /**
         * @brief Convert a RuneSaveValue to a displayable string
         */
        std::string ValueToString(const SaveValue &value, bool showStringQuotes=false);

        /**
         * @brief Get the save version string from a RuneSaveVersion
         */
        std::string GetVersionString(Version saveVersion);

        /**
         * @brief Get the BotwSaveHeader from the `.sav` file path.
         */
        Result GetSaveHeader(Header *out, const std::string &savePath);

        /**
         * @brief Mount the current user's BotW save data as a filesystem at "save:/"
         */
        Result MountSaveData();

        /**
         * @brief Unpacks all of the save slots in a BotW save file.
         */
        Result GetSaveSlots(std::vector<Slot> *out);

        /**
         * @brief Get a RuneSaveRawFile containing all entries from a provided save file path.
         */
        Result GetSaveRawFile(RawFile *out, std::string filePath);

        /**
         * @brief Get the RuneSaveFile from a RuneSaveRawFile
         */
        Result GetSaveFile(File *out, RawFile rawSaveFile);

        /**
         * @brief Preload the save metadata for understanding data IDs in a save file
         */
        Result LoadMetadata(Metadata *out);

        /**
         * @brief Get the meta entry for a save value by its ID.
         * It's recommended to initialise your out MetaEntry to nullptr in the case that the entry is not found, guards can be developed.
         */
        Result GetMetaEntry(const Rune::Save::Metadata& metadata, uint32_t id, MetaEntry* outMetaEntry);

        /**
         * @brief Get the DataName from an ID
         */
        Result GetDataName(const Metadata &metadata, uint32_t id, std::string *outDataName);

        /**
         * @brief Get the data type of a DataName
         */
        Result GetDataType(const Metadata &metadata, uint32_t id, DataType *outDataType);

        /**
         * @brief Get a RuneSaveEntry from a RuneSaveFile by the entry's DataName
         */ 
        Entry* GetEntryFromDataName(File &saveFile, const std::string &dataName);

        /**
         * @brief Convert a decoded save file to a raw save file.
         */
        RawFile* Encode(File* saveFile);

        /**
         * @brief Compile a save file back from RawFile to .sav
         * YOU ARE RESPONSIBLE FOR MANAGING THE MEMORY OF RAWSAVEFILE!!!
         */
        Result CompileTo(RawFile* rawSaveFile, const std::string &outputPath);

        /**
         * @brief Get a readable name for an internal dataName
         */
        std::string GetReadableKeyName(std::string& saveKey);

        /**
         * @brief Get a description from an internal dataName
         */
        std::string GetKeyDescription(std::string& saveKey);
    }
}

extern Rune::Save::Metadata *runeMetadata;