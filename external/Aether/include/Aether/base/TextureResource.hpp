#ifndef AETHER_TEXTURERESOURCE_CPP
#define AETHER_TEXTURERESOURCE_CPP

#include <SDL2/SDL_render.h>

#include <memory>

namespace Aether {
    class Renderer;

    class TextureResource {
        private:
            SDL_Texture* texture_;
            Renderer* renderer_;

            unsigned int width_;
            unsigned int height_;

        public:
            TextureResource(Renderer* renderer, SDL_Texture* texture, const int w, const int h);

            int width() const;
            int height() const;

            SDL_Texture* texture();

            ~TextureResource();
    };
    
    using TextureHandle = std::shared_ptr<Aether::TextureResource>;
}

#endif