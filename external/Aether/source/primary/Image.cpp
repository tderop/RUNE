#include "Aether/primary/Image.hpp"
#include "Aether/base/TextureResource.hpp"

// #include <switch.h>

namespace Aether {
    void Image::cacheTexture(std::string_view path){

    }

    std::string Image::makeKey(const std::string& path, const int w, const int h){
        return path+"|"+std::to_string(w)+"x"+std::to_string(h);
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
        // Check if suitable to perform texture cache operations
        if (type != Render::Wait && kCachingEnabled && 
            this->type == Type::File && !this->path.empty()){
            
            // Generate the key from the path and dimensions
            this->cacheKey_ = makeKey(this->path, this->scaleWidth_, this->scaleHeight_);

            if (Image::textureCache_.hasKey(this->cacheKey_)){
                // Already found in cache, use this texture and change to cached texture mode
                this->texture_ = textureCache_.getData(this->cacheKey_);
                this->type = Type::Cached;
            }
            else if (type == Render::Async && Image::pending_.count(this->cacheKey_)){
                // This is already being loaded, poll in update
                this->waitingOnCache_ = true;
                return;
            }
            else if (Image::pending_.insert(this->cacheKey_).second){
                // Not found in cache and not being loaded, so insert this key to pending and begin work
                this->ownsPending_ = true;
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

    void Image::onTextureReady(){
        // Lose ownership of the pending set and remove the cache key from the list
        if (this->ownsPending_){
            Image::pending_.erase(this->cacheKey_);
            this->ownsPending_ = false;
        }

        if (!kCachingEnabled || this->type != Type::File || this->cacheKey_.empty() 
            || this->drawable->type() != Drawable::Type::Texture){

            return;
        }

        const int w = this->drawable->width();
        const int h = this->drawable->height();

        TextureHandle tex = std::make_shared<TextureResource>(
            this->renderer, this->drawable->releaseTexture(), w, h
        );

        if (!Image::textureCache_.hasKey(this->cacheKey_)){
            Image::textureCache_.addData(this->cacheKey_, tex);
        }

        // swap the owning drawable for a shared so cache owns texture
        delete this->drawable;
        this->drawable = new Drawable(this->renderer, tex);
        this->texture_ = tex;
        this->type = Type::Cached;
    }

    void Image::update(unsigned int dt){
        if (this->waitingOnCache_){
            if (Image::textureCache_.hasKey(this->cacheKey_)){
                this->waitingOnCache_ = false;
                this->texture_ = textureCache_.getData(this->cacheKey_);
                this->type = Type::Cached;
                Texture::renderSync();
            }
            else if (!Image::pending_.count(this->cacheKey_)){
                // loader finished without caching, load here instead
                this->waitingOnCache_ = false;
                this->renderAsync();
            }
        }
        Texture::update(dt);
    }

    Image::~Image() {
        if (this->ownsPending_){
            Image::pending_.erase(this->cacheKey_);
        }
    }
};