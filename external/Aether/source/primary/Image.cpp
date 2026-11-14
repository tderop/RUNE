#include "Aether/primary/Image.hpp"
#include "Aether/base/TextureResource.hpp"

// #include <switch.h>

namespace Aether {
    void Image::cacheTexture(std::string_view path){

    }

    Image::Image(const int x, const int y, unsigned char * data, const size_t size, const Render type) : Texture(x, y) {
        this->scaleWidth_ = 0;
        this->scaleHeight_ = 0;
        this->mem.ptr = data;
        this->mem.size = size;
        this->type = Type::Pointer;
        this->performRender(type);
    }

    Image::Image(const int x, const int y, const std::string & path, const Render type) : Texture(x, y) {
        this->scaleWidth_ = 0;
        this->scaleHeight_ = 0;
        this->path = path;
        this->type = Type::File;
        this->performRender(type);
    }

    Image::Image(const int x, const int y, const std::vector<unsigned char> & data, const Render type) : Texture(x, y) {
        this->scaleWidth_ = 0;
        this->scaleHeight_ = 0;
        this->copy = data;
        this->type = Type::Vector;
        this->performRender(type);
    }

    Image::Image(const int x, const int y, TextureHandle tex, const Render type) : Texture(x, y) {
        this->scaleWidth_ = 0;
        this->scaleHeight_ = 0;
        this->texture_ = tex;
        this->type = Type::Cached;
        this->performRender(type);
    }

    Drawable * Image::renderDrawable() {
        switch (this->type) {
            case Type::File:
                return this->renderer->renderImageSurface(this->path, this->scaleWidth_, this->scaleHeight_);

            case Type::Pointer: {
                std::vector<unsigned char> buf;
                buf.assign(this->mem.ptr, this->mem.ptr + this->mem.size);
                return this->renderer->renderImageSurface(buf, this->scaleWidth_, this->scaleHeight_);
            }

            case Type::Cached: {
                return new Drawable(this->renderer, this->texture_);
            }

            case Type::Vector:
                return this->renderer->renderImageSurface(this->copy, this->scaleWidth_, this->scaleHeight_);
        }

        return new Drawable();
    }

    void Image::performRender(const Render type) {
        if (type != Render::Wait && Aether::Image::kCachingEnabled){
            // Read cache in the case of a file-type image to prevent storage reads
            if (this->type == Type::File && !this->path.empty()){
                TextureHandle cachedTexture = nullptr;
                std::string key = this->path+std::to_string(this->scaleWidth_)
                    +std::to_string(this->scaleHeight_);

                // if texture is cached, use that instead of reading from storage
                if (Image::textureCache_.hasKey(key)){

                    // std::string msg = "Cache hit: "+key;
                    // svcOutputDebugString(msg.c_str(), msg.size());

                    cachedTexture = Image::textureCache_.getData(key);
                    this->texture_ = cachedTexture;
                    this->type = Type::Cached;
                }

                // If the texture isn't cached, then generate and cache it
                else {
                    std::unique_ptr<Drawable> drawable(renderer->renderImageSurface(path, this->scaleWidth_, 
                        this->scaleHeight_));
                    bool success = drawable->convertToTexture();
                    if (success){
                        // If the texture was generated successfully, make a new texture handle, and cache it
                        TextureHandle texture = std::make_shared<TextureResource>(this->renderer, drawable->releaseTexture(), 
                            drawable->width(), drawable->height());
                        Image::textureCache_.addData(key, texture);

                        // Change type to texture and use the new texture
                        this->texture_ = texture;
                        this->type = Type::Cached;
                    }
                }
        }
        }

        if (type == Render::Sync) {
            Texture::renderSync();

        } else if (type == Render::Async) {
            Texture::renderAsync();
        }
    }

    void Image::renderSync(){
        this->performRender(Render::Sync);
    }

    void Image::renderAsync(){
        this->performRender(Render::Async);
    }

    bool Image::setScaleDimensions(const size_t width, const size_t height) {
        // We can't render images over 8000px
        if (width >= 8000 || height >= 8000) {
            return false;
        }

        this->scaleWidth_ = width;
        this->scaleHeight_ = height;
        this->setW(scaleWidth_);
        this->setH(scaleHeight_);

        return true;
    }

    size_t Image::scaleWidth() {
        return this->scaleWidth_;
    }

    size_t Image::scaleHeight() {
        return this->scaleHeight_;
    }

    int Image::right() {
        return this->x()+this->scaleWidth();
    }

    int Image::bottom() {
        return this->y()+this->scaleHeight();
    }

    int Image::centreX() {
        return this->x()+this->scaleWidth()/2;
    }

    int Image::centreY() {
        return this->y()+this->scaleHeight()/2;
    }

    Image::~Image() {

    }
};