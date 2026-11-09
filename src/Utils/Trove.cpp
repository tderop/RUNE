#include <cstdint>
#include <algorithm>

#include <Aether/Aether.hpp>
#include <Utils/Trove.hpp>
#include <Utils/Logger.hpp>

Trove::Trove(){
    Logger::Write("[Trove::Trove] - Trove created.");
};

void Trove::addElement(AetherElement element){
    // Logger::Write("[TROVE] - Adding element to trove...");
    this->elements.push_back(element); // pretend to do it 
    // this->numbers.push_back(1); // try generic push back

    // Logger::Write("[TROVE] - Successfully added element to trove!");
}

void Trove::removeElement(Aether::Element* element){
    this->elements.erase(
        std::remove_if(this->elements.begin(), this->elements.end(),
            [element](const AetherElement& tracked){
                return std::visit([element](auto* ptr){
                    return static_cast<Aether::Element*>(ptr) == element;
                }, tracked);
            }),
        this->elements.end()
    );
}

void Trove::giveTask(std::function<void()> task){
    this->tasks.push_back(task);
}

void Trove::clear() {
    // Logger::Write("[TROVE] - Removing elements...");
    // Logger::Write("[TROVE] - About to inspect vector...");
    // Logger::Write("[TROVE] - Element count: " + std::to_string(this->elements.size()));
    // Logger::Write("[TROVE] - Vector inspected.");

    for (AetherElement elm : this->elements) {
        // Logger::Write("[TROVE] - Entered loop.");

        bool isNull = std::visit([](auto&& arg)->bool{
            return arg == nullptr;
        }, elm);

        if (isNull) {
            // Logger::Write("[TROVE] - nullptr.");
            continue;
        }

        // Logger::Write("[TROVE] - Getting parent...");

        Aether::Element* parent = std::visit([](auto&& arg)->Aether::Element*{
            return arg->parent();
        }, elm);

        // Logger::Write("[TROVE] - Got parent.");

        if (parent == nullptr) {
            // Logger::Write("[TROVE] - No parent.");
            continue;
        }

        // Logger::Write("[TROVE] - Removing...");
        
        Aether::Element* element = std::visit([](auto&& arg)->Aether::Element*{
            return arg;
        }, elm);

        parent->removeElement(element);
        // Logger::Write("[TROVE] - Removed.");
    }

    // Logger::Write("[TROVE] - Clearing vector...");
    this->elements.clear();

    // Run all tasks
    for (const auto& task : this->tasks){
        task();
    }

    // Clear all tasks
    this->tasks.clear();

    // Logger::Write("[TROVE] - Done.");
}

Trove::~Trove(){
    // this->clear();
}