#pragma once

#include <switch.h>
#include <list>
#include <functional>
#include <unordered_map>

// forward declaration for connection
template<typename... Args>
class Signal;

template<typename... Args>
class Connection
{
private:
    Signal<Args...>* event;
    size_t id;
    bool connected = true;

public:
    Connection(Signal<Args...>* event, size_t id) : event(event), id(id) {}
    void disconnect();

    bool isConnected(){
        return connected;
    }
};

// executed when bound to a fired event through a connection
template<typename... Args>
struct Callback
{
    size_t id;
    std::function<void(Args...)> function;
};

// events which can have functions bound to them using .connect
template<typename... Args>
class Signal
{
private:
    std::unordered_map<int, Callback<Args...>> callbacks;
    std::list<Connection<Args...>> connections;

    size_t nextID = 0;

public:
    Connection<Args...>& connect(std::function<void(Args...)> callback){
        size_t id = nextID++;

        callbacks[id] = {
            id,
            callback
        };

        // build the item in the list, a list to also ensure the data does not change position
        connections.emplace_back(this, id);
        return connections.back(); // by reference
    }

    Connection<Args...>& connectWithId(std::function<void(size_t, Args...)> callback){
        size_t id = nextID++;
        callbacks[id] = {
            id,
            [callback, id](Args... args){
                callback(id, args...);
            }
        };

        connections.emplace_back(this, id);
        return connections.back();
    }

    // disconnect a connection by its ID, to be called by connections
    void disconnect(size_t connID){
        callbacks.erase(connID);

        // let the memory roam free
        // auto it = connections.begin();
        // while (it != connections.end()){
        //     if ((*it).id == connID){
        //         it = connections.erase(it);
        //     } else {
        //         it++;
        //     }
        // }
    }

    // fire the signal and run all bound functions
    void fire(Args... args){
        std::unordered_map<int, Callback<Args...>> snapshot = callbacks;
        for (const auto& [id, callback] : snapshot)
        {
            callback.function(args...);
        }
    }

    size_t getConnectionCount(){
        return callbacks.size(); 
    }

    // remove all connections
    void clear(){
        // disconnect all connections
        for (Connection connection : connections){
            connection.disconnect();
        }

        // clear all connections and callbacks
        connections.clear();
        callbacks.clear();
    }
};

template<typename... Args>
void Connection<Args...>::disconnect(){
    if (not connected){
        // log("[Connection] - Connection with ID "+std::to_string(this->id)+" is not currently connected.");
        return;
    }

    // log("[Connection] - Disconnecting connection with ID: "+std::to_string(this->id));
    connected = false;
    event->disconnect(id);
}