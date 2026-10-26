#include <Editor/WidgetBuilder.hpp>
#include <Editor/EditorWidget.hpp>
#include <Editor/WidgetOverlay.hpp> // maybe should be all one header idk

#include <Application/Keyboard.hpp>
#include <Config.hpp>

#include <Utils/Trove.hpp>

#include <string>

const float UPGRADE_VALUE = 200.0f; // 200 units per upgrade
const int UPGRADES_PER_WHEEL = 5;

/**
 * @brief Convert a float value (e.g. 1200.000000) to an understandable value (e.g. 2 wheels + 2/5)
 */
std::string floatToStamina(float rawStamina){
    // Full wheels (e.g. 1400 will return 1 wheel)
    int wheels = rawStamina / (UPGRADE_VALUE*UPGRADES_PER_WHEEL);

    float remainderWheels = (rawStamina / (UPGRADE_VALUE*UPGRADES_PER_WHEEL))-wheels;
    int fifths = remainderWheels*UPGRADES_PER_WHEEL;

    std::string resultStr = std::to_string(wheels);
    if (fifths != 0 && fifths != 5){
        resultStr += " + "+std::to_string(fifths)+"/"+std::to_string(UPGRADES_PER_WHEEL)+" wheels";
    } else {
        resultStr += (wheels != 1) ? " wheels" : " wheel";
    }

    return resultStr;
}

void Rune::WidgetBuilder::Stamina(EditorWidget* widget){
    Rune::Save::Entry* saveEntry = widget->getSaveEntry();

    widget->addEditButton(
        Rune::Save::GetReadableKeyName(saveEntry->dataName),
        floatToStamina(std::get<float>(saveEntry->value))
    );  

    widget->addDescription(
        Rune::Save::GetKeyDescription(saveEntry->dataName)
    );

    widget->getEditButton()->onPress([widget]() mutable{
        WidgetOverlay* editOverlay = widget->getWidgetOverlay();
        Rune::Save::Entry* saveEntry = widget->getSaveEntry(); // get fresh reference

        editOverlay->setTitleLabel("Edit Value - "+Rune::Save::GetReadableKeyName(saveEntry->dataName));
        
        // build spinner
        Aether::Spinner* numSpinner = new Aether::Spinner(
            editOverlay->panel()->x() + editOverlay->panel()->w()/2,
            editOverlay->panel()->y() + editOverlay->panel()->h()/2 - 24,
            180
        );

        // anchor to centre
        numSpinner->setXY(
            numSpinner->x() - numSpinner->w()/2 + 36,
            numSpinner->y() - numSpinner->h()/2
        );
        /** */

        // Show stamina wheels when num spinner value is changed
        log("[Stamina] - Making stamina trove...");
        Trove* staminaTrove = new Trove();

        numSpinner->onValueChange([numSpinner, editOverlay, staminaTrove](){
            // log("[Stamina] - Clearing stamina trove...");
            staminaTrove->clear();

            // log("[Stamina] - Getting numSpinner value...");
            int value = numSpinner->value();

            if (!value){
                // log("[Stamina] - Value is zero, ignoring");
                return;
            }

            // Turn stamina vessels into full wheels
            // log("[Stamina] - Calculating segments for images");
            int fullWheels = value / UPGRADES_PER_WHEEL;
            int remainingSegments = value % UPGRADES_PER_WHEEL;

            // Show fullWheels
            int segmentOffset = numSpinner->x()+numSpinner->w()+36;
            if (fullWheels){
                // create full wheel
                // log("[Stamina] - Making full wheel image");
                Aether::Image* fullImg = new Aether::Image(
                    segmentOffset,
                    numSpinner->y(),
                    ASSET_PATHS.at("staminawheel_5"),
                    Aether::Render::Wait
                );

                fullImg->setScaleDimensions(64, 64);
                fullImg->renderSync();

                fullImg->setY(
                    numSpinner->valueContainer()->y()+(numSpinner->valueContainer()->h()/2)-(fullImg->textureHeight()/2)
                );

                editOverlay->addElement(fullImg);
                staminaTrove->addElement(fullImg);

                segmentOffset += fullImg->textureWidth() + 6;

                // add the quantity number
                // log("[Stamina] - Making full quantity image");
                Aether::Text* fullQuantity = new Aether::Text(
                    segmentOffset,
                    fullImg->y(),
                    "x"+std::to_string(fullWheels),
                    24
                );

                fullQuantity->setY(
                    fullImg->y()+fullImg->textureHeight()-fullQuantity->textureHeight()
                );

                fullQuantity->setColour(TEXT_COLOUR);
                editOverlay->addElement(fullQuantity);
                staminaTrove->addElement(fullQuantity);

                segmentOffset += fullQuantity->textureWidth() + 12;
            }

            if (remainingSegments){
                // log("[Stamina] - Finding stamina wheel asset");
                const std::string& assetPath = ASSET_PATHS.at("staminawheel_"+std::to_string(remainingSegments));

                // show segment image
                // log("[Stamina] - Making segment image");
                Aether::Image* segmentImg = new Aether::Image(
                    segmentOffset,
                    numSpinner->y(),
                    assetPath,
                    Aether::Render::Wait
                );

                segmentImg->setScaleDimensions(64, 64);
                segmentImg->renderSync();

                segmentImg->setY(
                    numSpinner->valueContainer()->y()+(numSpinner->valueContainer()->h()/2)-(segmentImg->textureHeight()/2)
                );

                editOverlay->addElement(segmentImg);
                staminaTrove->addElement(segmentImg);
            }

        });

        log("[Stamina] - Assigned onValueChange for numSpinner");
        numSpinner->setLabel("Stamina Vessels");

        log("[Stamina] - Setting numSpinner value...");
        numSpinner->setValue((int)(std::get<float>(saveEntry->value)/(UPGRADE_VALUE)));
        
        log("[Stamina] - onValueChange value set successfully!");

        Rune::Save::MetaEntry* metaEntry = widget->getMetaEntry();
        int maxUpgrades = std::get<float>(metaEntry->maxValue)/UPGRADE_VALUE;

        numSpinner->setMax(maxUpgrades);
        numSpinner->setMin(0);

        // some extra help
        Aether::TextBlock* helperText = new Aether::TextBlock(
            editOverlay->panel()->x()+36,
            editOverlay->panel()->y()+72+36,
            "Stamina is a float value which ranges from 1000 (1 wheel) to 3000 (3 wheels), with each Stamina Vessel worth 200 units.\n\n"
            "The spinner adjusts stamina by 200 per step. Normally, you can only obtain up to 10 upgrades, but the spinner supports up to 327 (65,535 units), allowing stamina to exceed the visual maximum!\n\n"
            "Setting this to zero - alike hearts - doesn't corrupt your save... but it does cause some weirdness.",
            14,
            480
        );
        helperText->setColour(MUTED_TEXT_COLOR);
        editOverlay->addTempElement(helperText);

        // Separator rectangle
        Aether::Rectangle* textSeparator = new Aether::Rectangle(
            helperText->x(),
            helperText->y()+helperText->textureHeight()+6,
            320,
            4,
            0
        );
        textSeparator->setColour(SEPARATOR_COLOUR);
        editOverlay->addTempElement(textSeparator);

        Aether::Text* devNoteText = new Aether::Text(
            helperText->x(),
            textSeparator->y()+textSeparator->h()+12,
            "Developer Note",
            16
        );
        editOverlay->addTempElement(devNoteText);

        numSpinner->onPress(
            [widget, numSpinner](){
                float outFloat;
                Result rc = Rune::Keyboard::getNumberInput(
                    &outFloat, "Enter a number value...", 
                    std::get<float>(widget->getMetaEntry()->minValue),
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
        editOverlay->onCloseSignal->connectWithId([widget, numSpinner, staminaTrove](size_t id, bool isSave){
            widget->getWidgetOverlay()->onCloseSignal->disconnect(id);

            staminaTrove->clear();
            delete staminaTrove;

            if (isSave){
                // update value
                log("[WidgetOverlay::onCloseSignal] - WidgetOverlay closed; updating value to "
                    +numSpinner->value());

                int upgrades = numSpinner->value();
                float rawStamina = upgrades*UPGRADE_VALUE;

                widget->getEditButton()->setValue(floatToStamina(rawStamina));
                widget->getSaveEntry()->value = rawStamina;
            }
        });

        // show overlay
        widget->getWindow()->addOverlay(editOverlay);
    });
}