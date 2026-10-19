// for handling encoding and recompilation
#include <cstring>

#include <Save/Save.hpp>

/**
 * oh boy
 */
Rune::Save::RawFile* Rune::Save::Encode(Rune::Save::File &saveFile){
    Rune::Save::RawFile* output = new Rune::Save::RawFile();

    // MOVE HEADER
    output->header = saveFile.header;

    // MAIN ITERATOR
    size_t cursor = 0;

    for (Entry entry : saveFile.entries){
        switch (entry.dataType){
            case RuneBool:
            case RuneRevivalBool: {
                // convert the bool to uint32_t
                uint32_t encodedValue = std::get<bool>(entry.value);
                output->entries.emplace_back(entry.id, encodedValue);

                break;
            }

            case RuneS32:
            case RuneRevivalS32: {
                // convert s32 to uint32_t
                uint32_t encodedValue = std::get<int32_t>(entry.value);
                output->entries.emplace_back(entry.id, encodedValue);

                break;
            }

            case RuneF32:{
                // convert F32 to uint32_t
                RawEntry rawEntry;
                rawEntry.id = entry.id;

                float decodedValue = std::get<float>(entry.value);
                std::memcpy(&rawEntry.value, &decodedValue, sizeof(decodedValue));

                output->entries[cursor] = rawEntry;
                break;
            }

            case RuneString32:
            case RuneString64:
            case RuneString256: {
                size_t stringSize;
                switch (entry.dataType){
                    case RuneString32:
                        stringSize = 32;
                        break;
                    case RuneString64:
                        stringSize = 64;
                        break;
                    case RuneString256:
                        stringSize = 256;
                        break;
                    default:
                        continue;
                }

                uint32_t* outValue;
                std::string value = std::get<std::string>(entry.value);

                // determine the total number of chunks required
                size_t chunkCount = stringSize / sizeof(uint32_t);
                
                // iterate through and encode each chunk
                for (size_t chunk = 0; chunk < chunkCount; chunk++){
                    uint32_t rawValue = 0;

                    // for each byte in the string, re-encode it
                    for (size_t byte = 0; byte < sizeof(uint32_t); byte++){
                        size_t stringIdx = (chunk*sizeof(uint32_t)) + byte;

                        char c;
                        if (stringIdx < value.size()){
                            c = value[stringIdx];
                        } else {
                            c = '\0'; // null term when over string length
                        }

                        // convert the char into an unsigned char
                        // promote the byte to 32 bit integer
                        // shift the bits in the byte into its final position
                        // merge this result with rawValue
                        rawValue |= static_cast<uint32_t>(static_cast<unsigned char>(c) << (byte * 8));
                    }

                    // ASSIGN ID!!!
                    output->entries[cursor + chunk].id    = entry.id;
                    output->entries[cursor + chunk].value = rawValue;
                }

                cursor += chunkCount - 1;
            }

            case RuneVector2f:
            case RuneVector3f:
            case RuneVector4f: {
                size_t vectorSize;
                switch (entry.dataType){
                    case RuneVector2f:
                        vectorSize = 2;
                        break;
                    case RuneVector3f:
                        vectorSize = 3;
                        break;
                    case RuneVector4f:
                        vectorSize = 4;
                        break;
                    default:
                        continue;
                }

                // Get the original decoded vector
                std::vector decodedValue = std::get<std::vector<float>>(entry.value);

                // Iterate through the decoded vector
                for (size_t componentIdx = 0; 
                    componentIdx < vectorSize; componentIdx++){
                    // Create a new entry for each component
                    RawEntry rawEntry;
                    rawEntry.id = entry.id;
                    
                    // Get the original decoded component
                    float decodedComponent = decodedValue.at(componentIdx);

                    // Move the decoded component into the uint32_t value of the encoded raw entry
                    std::memcpy(&rawEntry.value, &decodedComponent, sizeof(decodedComponent));

                    // Add the new component entry to the list of encoded raw entries
                    output->entries[cursor+componentIdx] = rawEntry;
                }

                // Advance the cursor
                cursor += vectorSize - 1;
                break;
            }
            default:
                continue;
        }

        cursor++;
    }

    return output;
}