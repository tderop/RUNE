#include <Application/Audio.hpp>
#include <Json/Json.hpp>
#include <Utils/Logger.hpp>

namespace Rune {    
    const char* Audio::Init(){
        const char* soundErr = Sound::Init();
        if (soundErr != nullptr){
            return soundErr;
        }

        // Get the JSON
        Rune::Json* soundJson = nullptr;
        Rune::JsonData::GetJsonById(soundJson, "SoundPaths");

        if (soundJson == nullptr){
            return "Failed to load sound path JSON";
        }

        // Load the sounds
        for (const auto& [id, pathJson] : soundJson->items()){
            if (!pathJson.is_string()){ continue; }
            std::filesystem::path soundPath = pathJson.get<std::string>();
            if (!std::filesystem::exists(soundPath)){ 
                Logger::Importantf("Failed to load sound at %s", soundPath.c_str());
                continue; 
            } // Do not add sounds that don't exist

            Sound* newSound = new Sound(soundPath);
            if (!newSound->isLoaded()){ 
                Logger::Importantf("Sound at %s is not loaded", soundPath.c_str());
                continue; 
            } // Do not add unloaded sounds

            Audio::db_[id] = newSound; // add sound to database if path exists and sound is loaded
        }
        
        return nullptr;
    }

    Result Audio::playSound(const std::string& id){
        // Fail immediately if no database data
        if (Audio::db_.empty()){ return LibnxError_NotInitialized; }

        // Check if the sound exists in the sound db
        auto soundIt = Audio::db_.find(id);
        if (soundIt == Audio::db_.end()){
            return LibnxError_NotFound;
        }

        // Play the sound and return its error code
        return soundIt->second->play();
    }
} // namespace Rune