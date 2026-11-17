#include <Editor/WidgetBuilder.hpp>
#include <Editor/EditorWidget.hpp>
#include <Editor/WidgetOverlay.hpp> // maybe should be all one header idk

#include <Application/Keyboard.hpp>
#include <Application/Audio.hpp>

#include <Config.hpp>
#include <Save/Save.hpp>

#include <Utils/Trove.hpp>
#include <Utils/Logger.hpp>

#include <Json/Json.hpp>

#include <string>

const int UPGRADE_VALUE = 4; // 200 units per upgrade
const float SEGMENTS_PER_HEART = 4.0f;

const int HEARTS_PER_ROW = 15;
const int HEART_IMG_SIZE = 22;
const int HEART_IMG_PADDING = 5;

const int MAX_HEARTS = 30;

/**
 * @brief Convert an int value (e.g. 12) to an understandable value (e.g. 3 hearts)
 */
std::string intToHearts(float rawHealth){
    // Calculate how many full hearts are present
    int hearts = rawHealth / (SEGMENTS_PER_HEART);

    /**
     * Calculate how many remainder hearts there are
     * (twas originally like this because I was going to add support for modifying hearts at a quarter-heart level,
     * but that felt a bit unnecessary for someone that may just want to have max hearts for example)
     */
    float remainderHearts = (rawHealth / (SEGMENTS_PER_HEART))-hearts;
    int quarters = remainderHearts*SEGMENTS_PER_HEART;

    // Determine the result string
    std::string resultStr = std::to_string(hearts);

    // If the value is fractional, show the value, the fraction, and hearts
    if (quarters != 0 && quarters != 4){
        resultStr += " + "+std::to_string(quarters)+"/"+std::to_string(SEGMENTS_PER_HEART)+" hearts";
    } else { // Otherwise just do a 1 check to determine whether it should be hearts or heart
        resultStr += (hearts != 1) ? " hearts" : " heart";
    }

    return resultStr;
}

void Rune::WidgetBuilder::Hearts(EditorWidget* widget){
    Rune::Save::Entry* saveEntry = widget->getSaveEntry();

    // Add the edit button
    widget->addEditButton(
        Rune::Save::GetReadableKeyName(saveEntry->dataName),
        intToHearts(std::get<int32_t>(saveEntry->value))
    );  

    // Add the description comment
    widget->addDescription(
        Rune::Save::GetKeyDescription(saveEntry->dataName)
    );

    // Set the on press function
    widget->getEditButton()->onPress([widget]() mutable{
        // Get the edit overlay and the save entry
        WidgetOverlay* editOverlay = widget->getWidgetOverlay();
        Rune::Save::Entry* saveEntry = widget->getSaveEntry();

        // Set the title label of the overlay
        editOverlay->setTitleLabel("Edit Value - "+Rune::Save::GetReadableKeyName(saveEntry->dataName));
        
        // Build the spinner used to select the heart count
        Aether::Spinner* numSpinner = new Aether::Spinner(
            editOverlay->panel()->x() + editOverlay->panel()->w()/2,
            editOverlay->panel()->y() + editOverlay->panel()->h()/2 - 24,
            180
        );
        numSpinner->onValueChange([](){
            Rune::Audio::playSound("cursor");
        });

        // Anchor the number spinner to the centre of the overlay
        numSpinner->setXY(
            numSpinner->x() - numSpinner->w()/2 ,
            numSpinner->y() - numSpinner->h()/2
        );

        // Show heart containers when the number spinner value is changed
        std::vector<Aether::Image*>* hearts = new std::vector<Aether::Image*>;

        numSpinner->onValueChange([numSpinner, editOverlay, hearts](){
            Logger::Important("Updating heart count");

            // The initial X position of the first heart on each row
            int initX = numSpinner->x() + numSpinner->w() + 36;
            // The initial Y position of every heart on the first row
            int initY = numSpinner->valueContainer()->y() + numSpinner->valueContainer()->h()/2 - HEART_IMG_SIZE - HEART_IMG_PADDING/2;

            // Get the current value of the number spinner
            int value = numSpinner->value();

            // if (!value){
            //     // If the value is zero, do nothing
            //     return;
            // }

            // The currently processing heart row (each row consists of 15 hearts, every time 15 hearts have been made this is incremented)
            int row = hearts->size()/HEARTS_PER_ROW;

            // Currently processing heart column
            int col = (hearts->size()%HEARTS_PER_ROW);

            // reformat hearts

            // If the number spinner value is greater than the current number of hearts, generate more hearts
            if (value > (int)hearts->size()){
                // Create the number of hearts equal to the difference between hearts.size() and value
                size_t toGenerate = value-(hearts->size());
                for (size_t i = 0; i < toGenerate; ++i){
                    Aether::Image* newHeart = new Aether::Image(
                        initX+(col*(HEART_IMG_SIZE+HEART_IMG_PADDING)),
                        initY+(row*(HEART_IMG_SIZE+HEART_IMG_PADDING)),
                        ASSET_PATHS.at("heart"),
                        Aether::Render::Wait
                    );

                    // Set the size to 24x24
                    newHeart->setScaleDimensions(HEART_IMG_SIZE, HEART_IMG_SIZE);
                    newHeart->renderAsync();

                    // Add new hearts
                    editOverlay->addTempElement(newHeart);
                    hearts->push_back(newHeart);

                    // Recalculate row and column
                    row = hearts->size()/HEARTS_PER_ROW;
                    col = (hearts->size()%HEARTS_PER_ROW);
                }
            } else {
                // Remove hearts until the cursor (i) reaches the last heart that should be rendered
                for (int i = (hearts->size()-1); i > value-1; i--){
                    // Remove the heart image at this index
                    Aether::Image* heartImg = hearts->at(i);
                    editOverlay->removeElement(heartImg);

                    // Remove the heart from the vector
                    hearts->erase(hearts->begin()+i);
                }
            }
        });

        // Set the label of the number spinner to say "Heart Containers"
        numSpinner->setLabel("Heart Containers");

        // Set the value of the number spinner to the current value divided by four
        numSpinner->setValue(std::min(
            MAX_HEARTS,
            std::get<int32_t>(saveEntry->value)/UPGRADE_VALUE
        ));

        // Get the stamina metaEntry (used for max/min values)
        Rune::Save::MetaEntry* metaEntry = widget->getMetaEntry();

        // Set the maximum value of the number spinner to the max hearts (30), and the minimum value (will always be zero, idk why Nintendo would decide to implement negative hearts one day)
        numSpinner->setMax(MAX_HEARTS);
        numSpinner->setMin(0);

        // Some extra info about the health value

        // Set the developer note
        editOverlay->setDeveloperNote(Rune::Save::GetKeyDevNote(saveEntry->dataName));

        // When the number spinner is pressed, show a keyboard and ask for a number input
        numSpinner->onPress(
            [widget, numSpinner](){
                float outFloat;
                Result rc = Rune::Keyboard::getNumberInput(
                    &outFloat, "Enter a number value...", 
                    std::get<int32_t>(widget->getMetaEntry()->minValue),
                    numSpinner->max()
                );

                if (R_SUCCEEDED(rc)){
                    numSpinner->setValue((int)outFloat);
                } else {
                    Logger::Write("[WidgetBuilder::Float] - An error ocurred when taking number value input: "+std::to_string(rc));
                }
            }
        );
        
        // add spinner
        editOverlay->addTempElement(numSpinner);

        // focus the spinner
        editOverlay->setFocused(numSpinner);

        // save on close
        editOverlay->onCloseSignal->connectWithId([widget, numSpinner, hearts](size_t id, bool isSave){
            widget->getWidgetOverlay()->onCloseSignal->disconnect(id);

            for (size_t i = 0; i < hearts->size(); ++i){
                Aether::Image* heartImg = hearts->at(i);
                widget->getWidgetOverlay()->removeElement(heartImg);
            }

            // Free memory
            delete hearts;

            if (isSave){
                // And after rigorously overcommenting my code instead of actually developing, I found a bug I didn't even intend to find
                // This was originally concatenating a string with an int (""+numSpinner->value())
                // I shall go and fix this in the stamina editor too since this is an edited copy-paste widget of that one but with (arguably) 'better' comments
                Logger::Write("[WidgetOverlay::onCloseSignal] - WidgetOverlay closed; updating value to "
                    +std::to_string(numSpinner->value()));
                
                // Calculate what the value should really be in save data (multiply value by 4 as save stores it in quarters)
                int upgrades = numSpinner->value();
                int rawHealth = upgrades*UPGRADE_VALUE;

                // Set the value to the readable value (120 -> 30 hearts)
                widget->getEditButton()->setValue(intToHearts(rawHealth));
                widget->getSaveEntry()->value = rawHealth; // Update the value in save data
            }
        });

        // show overlay
        widget->getWindow()->addOverlay(editOverlay);
    });
}