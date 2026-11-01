#include <Utils/Cache.hpp>

#include <optional>
#include <string>
#include <mutex>
#include <any>

Rune::Cache::Cache(size_t capacity){
    this->_capacity = capacity;
}

Rune::Cache::~Cache(){
    this->clear();
}

void Rune::Cache::put(const std::string& key, std::any data){
    std::lock_guard<std::mutex> lock(this->_mutex);

    auto it = this->_cache.find(key);
    if (it != this->_cache.end()){
        // Erase the key from its previous position in usageOrder
        this->_usageOrder.erase(it->second.second);
        // Move the key to the front to refresh its lifetime
        this->_usageOrder.push_front(key);

        // Update the iterator position
        it->second.second = this->_usageOrder.begin();
        // Assign the new data
        it->second.first = data;

        // data updated, no need to create new value
        return;
    }

    // If the cache is at max capacity, remove oldest element
    if (this->_cache.size() >= this->_capacity){
        // Get the oldest key
        std::string oldestKey = this->_usageOrder.back();
        // Remove the oldest data from the cache
        this->_usageOrder.pop_back();
        this->_cache.erase(oldestKey);
    }

    // Add the key to the usage order
    _usageOrder.push_front(key);
    // Add the data to the cache alongside its position in the usage list
    this->_cache[key] = {data, _usageOrder.begin()};
}

void Rune::Cache::clear(){
    // Empty the cache
    this->_cache.clear();

    // Empty the usage list
    this->_usageOrder.clear();
}

void Rune::Cache::setCapacity(size_t capacity){
    this->_capacity = capacity;
}