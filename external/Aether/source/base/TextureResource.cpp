#include "Aether/base/TextureResource.hpp"
#include "Aether/Renderer.hpp"

#include <SDL2/SDL_render.h>

namespace Aether {
    TextureResource::TextureResource(
        Renderer* renderer,
        SDL_Texture* texture, 
        const int w, const int h){
        
        this->texture_= texture;
        this->width_ = w;
        this->height_ = h;
        this->renderer_ = renderer;
    }

    SDL_Texture* TextureResource::texture(){
        return this->texture_;
    }

    int TextureResource::width() const{
        return this->width_;
    }

    int TextureResource::height() const{
        return this->height_;
    }

    TextureResource::~TextureResource(){
        if (this->renderer_ && this->texture_){
            this->renderer_->destroyTexture(this->texture_, true);
        }
    }
} // namespace Aether