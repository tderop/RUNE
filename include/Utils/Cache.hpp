#pragma once

#include <unordered_map>
#include <optional>
#include <string>
#include <mutex>
#include <list>
#include <any>

/**
 * @brief For caching repeatedly accessed data. Has a max size to prevent infinite memory leakage.
 * @note Cache hits will move the element to the front of the usage order, meaning the data at that key will be refreshed to be deleted less soon.
 */
class Cache{
    private:
        std::list<std::string> _usageOrder;
        std::unordered_map<std::string, std::pair<std::any, std::list<std::string>::iterator>> _cache;
        std::mutex _mutex;

        size_t _capacity;
    
    public:
        /**
         * @brief Create a new cache. It is recommended to use `std::unique_ptr` and `std::make_unique` or the shared equivalents instead of `new`
         * when caching allocated data to clean that memory upon cache destruction.
         */
        Cache(size_t capacity=50);
        ~Cache();

        /**
         * @brief Get the data stored at a key in the cache
         * @param key the key in cache where the data may be stored
         * @return The data stored in that key in cache
         */
        template <typename T>
        std::optional<T> get(const std::string& key){
            std::lock_guard<std::mutex> lock(this->_mutex);
            
            // Find the element in cache
            auto it = this->_cache.find(key);
            if (it == this->_cache.end()){
                // Cache miss, return nullopt
                return std::nullopt;
            }

            // Attempt to cast to requested value
            // on fail, return nullopt
            T* valuePtr = std::any_cast<T>(&it->second.first);
            if (valuePtr == nullptr){
                return std::nullopt;
            }

            // Cache hit, update position to front
            this->_usageOrder.erase(it->second.second);
            this->_usageOrder.push_front(key);
            it->second.second = this->_usageOrder.begin();

            return *valuePtr;
        }

        /**
         * @brief Put data in cache
         * @param key the key to store the data at in the cache
         * @param data the data to store
         */
        void put(const std::string& key, std::any data);

        /**
         * @brief Clear the cache
         */
        void clear();

        /**
         * @brief Set the max capacity
         * @param capacity the new capacity of the cache
         */
        void setCapacity(size_t capacity);
}; 