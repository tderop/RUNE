#include <cstdint>

#include <Aether/Aether.hpp>
#include <Utils/Trove.hpp>
#include <Utils/Utils.hpp>

Trove::Trove(){
    log("[Trove::Trove] - Trove created.");
};

void Trove::addElement(AetherElement element){
    // log("[TROVE] - Adding element to trove...");
    this->elements.push_back(element); // pretend to do it 
    // this->numbers.push_back(1); // try generic push back

    // log("[TROVE] - Successfully added element to trove!");
}

void Trove::clear() {
    // log("[TROVE] - Removing elements...");
    // log("[TROVE] - About to inspect vector...");
    // log("[TROVE] - Element count: " + std::to_string(this->elements.size()));
    // log("[TROVE] - Vector inspected.");

    for (AetherElement elm : this->elements) {
        // log("[TROVE] - Entered loop.");

        bool isNull = std::visit([](auto&& arg)->bool{
            return arg == nullptr;
        }, elm);

        if (isNull) {
            // log("[TROVE] - nullptr.");
            continue;
        }

        // log("[TROVE] - Getting parent...");

        Aether::Element* parent = std::visit([](auto&& arg)->Aether::Element*{
            return arg->parent();
        }, elm);

        // log("[TROVE] - Got parent.");

        if (parent == nullptr) {
            // log("[TROVE] - No parent.");
            continue;
        }

        // log("[TROVE] - Removing...");
        
        Aether::Element* element = std::visit([](auto&& arg)->Aether::Element*{
            return arg;
        }, elm);

        parent->removeElement(element);
        // log("[TROVE] - Removed.");
    }

    // log("[TROVE] - Clearing vector...");
    this->elements.clear();

    // log("[TROVE] - Done.");
}

Trove::~Trove(){
    // this->clear();
}