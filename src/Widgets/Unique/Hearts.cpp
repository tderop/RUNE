#include <Widgets/WidgetBuilder.hpp>
#include <Widgets/EditorWidget.hpp>
#include <Widgets/WidgetOverlay.hpp> // maybe should be all one header idk

#include <Application/Keyboard.hpp>
#include <Config.hpp>

#include <Utils/Trove.hpp>

#include <string>

const int UPGRADE_VALUE = 4; // 200 units per upgrade
const float SEGMENTS_PER_HEART = 4.0f;

const int HEARTS_PER_ROW = 15;
const int HEART_IMG_SIZE = 22;
const int HEART_IMG_PADDING = 5;

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
        getReadableSaveKeyName(saveEntry->dataName),
        intToHearts(std::get<int32_t>(saveEntry->value))
    );  

    // Add the description comment
    widget->addDescription(
        getSaveKeyDescription(saveEntry->dataName)
    );

    // Set the on press function
    widget->getEditButton()->onPress([widget]() mutable{
        // Get the edit overlay and the save entry
        WidgetOverlay* editOverlay = widget->getWidgetOverlay();
        Rune::Save::Entry* saveEntry = widget->getSaveEntry();

        // Set the title label of the overlay
        editOverlay->setTitleLabel("Edit Value - "+getReadableSaveKeyName(saveEntry->dataName));
        
        // Build the spinner used to select the heart count
        Aether::Spinner* numSpinner = new Aether::Spinner(
            editOverlay->panel()->x() + editOverlay->panel()->w()/2,
            editOverlay->panel()->y() + editOverlay->panel()->h()/2 - 24,
            180
        );

        // Anchor the number spinner to the centre of the overlay
        numSpinner->setXY(
            numSpinner->x() - numSpinner->w()/2 + 36,
            numSpinner->y() - numSpinner->h()/2
        );

        // Show heart containers when the number spinner value is changed
        Trove* heartTrove = new Trove();
        numSpinner->onValueChange([numSpinner, editOverlay, heartTrove](){
            // Delete all hearts on screen
            heartTrove->clear();

            // Get the current value of the number spinner
            int value = numSpinner->value();

            if (!value){
                // If the value is zero, do nothing
                return;
            }

            // The currently processing heart row (each row consists of 15 hearts, every time 15 hearts have been made this is incremented)
            int row = 0;

            // Currently processing heart column
            int col = 0;

            // The initial X position of the first heart on each row
            int xPos = numSpinner->x() + numSpinner->w() + 36;
            // The initial Y position of every heart on the first row
            int yPos = numSpinner->valueContainer()->y() + numSpinner->valueContainer()->h()/2 - HEART_IMG_SIZE - HEART_IMG_PADDING/2;

            for (int h = 1; h <= value; ++h){
                // Make the heart image
                Aether::Image* heart = new Aether::Image(
                    xPos+(col*(HEART_IMG_SIZE+HEART_IMG_PADDING)),
                    yPos+(row*(HEART_IMG_SIZE+HEART_IMG_PADDING)),
                    ASSET_PATHS.at("heart"),
                    Aether::Render::Wait
                );

                // log(std::to_string(heart->y()));

                // Set the size to 24x24
                heart->setScaleDimensions(HEART_IMG_SIZE, HEART_IMG_SIZE);
                heart->renderSync();

                // Add the heart to the edit overlay, and also add it to the heart trove
                editOverlay->addElement(heart);
                heartTrove->addElement(heart);

                // Increment column
                col++;

                // Increment row every 15 hearts, and reset row
                if (h%HEARTS_PER_ROW == 0 && h != 0){
                    row++;
                    col=0;
                }
            }
        });

        // Set the label of the number spinner to say "Heart Containers"
        numSpinner->setLabel("Heart Containers");

        // Set the value of the number spinner to the current value divided by four
        numSpinner->setValue(std::min(
            30,
            std::get<int32_t>(saveEntry->value)/UPGRADE_VALUE
        ));

        // Get the stamina metaEntry (used for max/min values)
        Rune::Save::MetaEntry* metaEntry = widget->getMetaEntry();

        // Set the maximum value of the number spinner to the max hearts (30), and the minimum value (will always be zero, idk why Nintendo would decide to implement negative hearts one day)
        numSpinner->setMax(30);
        numSpinner->setMin(0);

        // Some extra info about the health value
        Aether::TextBlock* helperText = new Aether::TextBlock(
            editOverlay->panel()->x()+36,
            editOverlay->panel()->y()+72+36,
            "Health is an integer value which ranges from 12 (1 heart) to 120 (30 hearts), with each Heart Container worth 4 units.\n\n"
            "The spinner adjusts health by 4 per step. The game caps your maximum health to 30 hearts, and prevents you from having less than 3. You cannot go above 30 hearts, but you can go below 3!\n\n"
            "Setting this to zero makes you die every time you load this file.",
            14,
            480
        );
        helperText->setColour(MUTED_TEXT_COLOR);
        editOverlay->addTempElement(helperText);

        // A rectangle to make things look a little cleaner
        Aether::Rectangle* textSeparator = new Aether::Rectangle(
            helperText->x(),
            helperText->y()+helperText->textureHeight()+6,
            320,
            4,
            0
        );
        textSeparator->setColour(SEPARATOR_COLOUR);
        editOverlay->addTempElement(textSeparator);

        // A small alt text descriptor of why the text exists
        Aether::Text* devNoteText = new Aether::Text(
            helperText->x(),
            textSeparator->y()+textSeparator->h()+12,
            "Developer Note",
            16
        );
        editOverlay->addTempElement(devNoteText);

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
                    log("[WidgetBuilder::Float] - An error ocurred when taking number value input: "+std::to_string(rc));
                }
            }
        );
        
        // add spinner
        editOverlay->addTempElement(numSpinner);

        // focus the spinner
        editOverlay->setFocused(numSpinner);

        // save on close
        editOverlay->onCloseSignal->connectWithId([widget, numSpinner, heartTrove](size_t id, bool isSave){
            widget->getWidgetOverlay()->onCloseSignal->disconnect(id);

            heartTrove->clear();
            delete heartTrove;

            if (isSave){
                // And after rigorously overcommenting my code instead of actually developing, I found a bug I didn't even intend to find
                // This was originally concatenating a string with an int (""+numSpinner->value())
                // I shall go and fix this in the stamina editor too since this is an edited copy-paste widget of that one but with (arguably) 'better' comments
                log("[WidgetOverlay::onCloseSignal] - WidgetOverlay closed; updating value to "
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