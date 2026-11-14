#include <switch.h>
#include <Utils/Sound.hpp>

const char* Sound::Init(){
    int rc = SDL_Init(SDL_INIT_AUDIO);
    if (rc != 0){ // Returns 0 on success
        return SDL_GetError();
    }

    rc = Mix_Init(MIX_INIT_OPUS);
    if (rc == 0){ // Returns nonzero on success
        return SDL_GetError();
    }

    rc = Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);
    if (rc != 0){ // Returns 0 on success
        return SDL_GetError();
    }

    // Set initialised value and return blank
    Sound::initialized_ = true;
    return nullptr;
}

Sound::Sound(const std::string& path){
    this->setPath(path);
}

Result Sound::setPath(const std::string& path){
    // If uninitialised, don't try to load anything
    if (!Sound::initialized_){ return LibnxError_NotInitialized; }

    // If there is already an initialised audio, free resources
    if (this->audio_ != nullptr){
        Mix_FreeMusic(this->audio_);
        this->audio_ = nullptr;
    }

    // Attempt to load the audio
    Mix_Music* audio = Mix_LoadMUS(path.c_str());

    if (audio != nullptr){
        // Audio successfully loaded
        this->path_ = path;
        this->audio_ = audio;
    }
    else {
        // Audio not found or malformed
        this->audio_ = nullptr;
        return LibnxError_NotFound;
    }

    return 0;
}

bool Sound::isLoaded(){
    return (this->audio_ != nullptr);
}

Result Sound::play(){
    if (!this->isLoaded()){
        return LibnxError_NotInitialized;
    }

    // Call SDL2_mixer
    Mix_PlayMusic(this->audio_, 1);
    return 0;
}

void Sound::Exit(){ 
    Mix_CloseAudio();
    Mix_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

Sound::~Sound(){
    // Free the sound if allocated
    if (this->audio_ != nullptr){
        // Not running?
        Mix_FreeMusic(this->audio_);
        this->audio_ = nullptr;
    }
}