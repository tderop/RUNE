#pragma once

// libnx
#include <switch.h>

// C++
#include <functional>

/**
 * RUNE's RomFS module is designed to prevent the redistribution of assets owned by Nintendo Co., Ltd.
 * Rune::Romfs is designed to extract and decode files from the console's installed copy of Breath of the Wild
 * Therefore, thumbnails and other copyright content are obtained legally. However, this means users without
 * BotW installed will have a degraded experience - one without item thumbnails or menu sounds.
 */

 /**
  * Relevant paths:
  * botw:/Sound/Resource/M_UI.bars <-- the audio files
  * botw:/UI/StockItem/<wildcard>.sbitemico <-- the item thumbnails
  */

namespace Rune{
    namespace Romfs{
        inline const std::string kSbItemIcoFolder = "botw:/UI/StockItem/";
        inline const std::string kMenuSoundBars   = "botw:/Sound/Resource/M_UI.bars";

        struct Bars{

        };

        struct BNTX{
            uint32_t size;
            uint32_t textureCount;
            uint32_t textureTblOffset;
        };

        struct BRTI{
            uint16_t tileMode;
            uint16_t mipCount;
            uint32_t pixelFormat;
            uint32_t width;
            uint32_t height;
            uint32_t depth;
            uint32_t textureLayout;
            uint32_t imgDataSize;
            uint32_t channelSelectors;
            uint64_t textureNameOffset;
            uint64_t mipTblOffset;
            uint64_t imgDataOffset;

            std::string textureName;
        };

        /**
         * @brief Extract all the assets from BotW required by RUNE for an improved user experience.
         * @return Error code
         */
        Result ExtractAssets(std::function<void(size_t, size_t)> onProgress=[](size_t, size_t){});

        /**
         * @brief Decompress a Yaz0-compressed file.
         */
        Result DecompressYaz0(std::vector<uint8_t>& out, const std::string& path);

        /**
         * @brief Decode a SBITEMICO file
         * @param path The path to the SBITEMICO file
         * @return Error code
         */
        Result ExtractSbitemIcon(const std::string& path);

        // /**
        //  * @brief Get a thumbnail PNG from an SBITEMICO file
        //  * @param sbitemico A decoded SBITEMICO file
        //  * @param output The output path, where the PNG should be stored
        //  * @return Error code
        //  */
        // Result getThumb(SbItemIco& sbitemico, const std::string& output);

        /**
         * @brief Decode a BARS file
         * @param out Where the decoded BARS data should be stored
         * @param path The path to the BARS file
         * @return Error code
         */
        Result DecodeBars(Bars* out, const std::string& path);

        /**
         * @brief Get a WAV from a path within a .bars archive
         * @param bars A decoded BARS file
         * @param barsPath The path within the BARS file that should be decoded to a WAV file
         * @param output The path where the WAV should be stored
         * @return Error code
         */
        Result GetAudio(Bars& bars, const std::string& barsPath, const std::string& output);

        /**
         * @brief Mount BotW's RomFS data at botw:/
         * @return Error code
         */
        Result MountBotwRomfs();

        /**
         * @brief Unmount BotW's RomFS data
         * @return Error code
         */
        Result UnmountBotwRomfs();

        // Constants
        inline const std::string kExtractedPath = "sdmc:/switch/RUNE/extracted/";
        inline const std::string kThumbnailPath = kExtractedPath+"/thumb";
        inline const std::string kSoundPath = kExtractedPath+"/sound";
    } // namespace Romfs
} // namespace Rune