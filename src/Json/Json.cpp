#include <Json/Json.hpp>
#include <Utils/Utils.hpp>

#include <fstream>
#include <string>
#include <unordered_map>

#include <switch.h>

namespace Rune{
    namespace JsonData{
        std::unordered_map<std::string, std::string> JsonPaths = {
            {   
                "EditorData",
                "romfs:/json/editor_data.jsonc"
            },
            {
                "ActorNames",
                "romfs:/json/actor_names.json"
            }
        };

        std::unordered_map<std::string, Json> JsonObjects;
    }
} // namespace Rune

void Rune::JsonData::Init(){
    // open the JSON files and initialise the JSON objects
    for (auto [dataId, jsonPath] : Rune::JsonData::JsonPaths){
        std::ifstream file(jsonPath);
        if (!file.is_open()){
            log("[Rune::JsonData::Init] - Failed to open JSON file! JSON ID: '"+dataId+"', JSON PATH: '"+jsonPath+"'");
            continue;
        }

        log("[Rune::JsonData::Init] - Loading JSON ID: '"+dataId+"', JSON PATH: '"+jsonPath+"'");
        Rune::JsonData::JsonObjects[dataId] = Rune::Json::parse(file, nullptr, false, true);        
    }
}

Result Rune::JsonData::GetJsonById(Rune::Json* out, const std::string &id){
    log("[Rune::JsonData::GetJsonById] - Getting JSON by ID: '"+id+"'");
    auto editDataJsonEntry = Rune::JsonData::JsonObjects.find(id);

    if (editDataJsonEntry == Rune::JsonData::JsonObjects.end()){
        log("[Rune::JsonData::GetJsonById] - Failed to find JSON data for '"+id+"'!");
        return LibnxError_NotFound;
    }

    log("[Rune::JsonData::GetJsonById] - Found JSON data for ID: '"+id+"'! Copying data to out.");
    *out = editDataJsonEntry->second; // copy data to pointer

    log("[Rune::JsonData::GetJsonById] - Data copied. Returning success.");
    return 0;
}