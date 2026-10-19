#include <Widgets/WidgetBuilder.hpp>
#include <Widgets/EditorWidget.hpp>
#include <Widgets/WidgetOverlay.hpp>

#include <Application/Keyboard.hpp>

#include <Save/Save.hpp>
#include <Config.hpp>

#include <string>

int SPINNER_SPACING = 20;
int SPINNER_WIDTH = 180;

std::vector<std::string> VECTOR_LETTERS = {"X", "Y", "Z", "A"};

void Rune::WidgetBuilder::Vector(EditorWidget* widget){
    Rune::Save::Entry* saveEntry = widget->getSaveEntry();
    widget->addEditButton(
        getReadableSaveKeyName(saveEntry->dataName),
        Rune::Save::ValueToString(saveEntry->value)
    );
    widget->addDescription(
        getSaveKeyDescription(saveEntry->dataName)
    );
    widget->getEditButton()->onPress([widget](){
        WidgetOverlay* ovl           = widget->getWidgetOverlay();
        Rune::Save::Entry* saveEntry = widget->getSaveEntry();

        std::string readableName  = getReadableSaveKeyName(saveEntry->dataName);
        std::string readableValue = Rune::Save::ValueToString(saveEntry->value);
        ovl->setTitleLabel(readableName);

        // build the number spinners
        auto value = std::get<std::vector<float>>(saveEntry->value);
        int vectorSize = value.size();

        std::vector<Aether::Spinner*> spinners;

        for (int i = 0; i < vectorSize; i++){
            Aether::Spinner* newSpinner = new Aether::Spinner(
                i*(SPINNER_WIDTH+SPINNER_SPACING),
                0,
                SPINNER_WIDTH
            );

            newSpinner->setValue(value.at(i));
            spinners.push_back(newSpinner);

            widget->addElementWithKey(newSpinner, "Spinner_"+std::to_string(i));
            newSpinner->setLabel(VECTOR_LETTERS.at(i));

            // Open the keyboard when the spinner is activated to get an input
            newSpinner->onPress([widget, i](){
                float outFloat;
                Result rc = Rune::Keyboard::getNumberInput(&outFloat, "Enter a number...", 
                    std::get<float>(widget->getMetaEntry()->minValue), 
                    std::get<float>(widget->getMetaEntry()->maxValue)
                );

                if (R_SUCCEEDED(rc)){
                    AetherElement spinnerElm;
                    rc = widget->getElementAtKey(spinnerElm, "Spinner_"+std::to_string(i));
                    Aether::Spinner* spinner = std::get<Aether::Spinner*>(spinnerElm);

                    spinner->setValue((int)outFloat);
                }
            });
        }

        // reposition them
        int middleIdx  = ((vectorSize/2)-1) + vectorSize%2;
        int ovlCenterX = (ovl->panel()->x()) + (ovl->panel()->w()/2);

        Aether::Spinner* centerSpinner = spinners.at(middleIdx);

        int xOffset;
        if (vectorSize%2){
            xOffset = ovlCenterX - (centerSpinner->x()) - (centerSpinner->w()/2);
        } else {
            Aether::Spinner* nextSpinner = spinners.at(middleIdx+1);
            xOffset = ovlCenterX - (centerSpinner->x()) - (centerSpinner->w()) - (SPINNER_SPACING/2);
        }

        int centerY = (ovl->panel()->y())+(ovl->panel()->h()/2)-(centerSpinner->h()/2);
        for (Aether::Spinner* spinner : spinners){
            spinner->setXY(
                spinner->x()+xOffset,
                centerY
            );

            ovl->addTempElement(spinner);
        }

        ovl->onCloseSignal->connectWithId([widget](size_t id, bool isSave){
            widget->getWidgetOverlay()->onCloseSignal->disconnect(id);

            if (isSave){
                Rune::Save::Entry* saveEntry = widget->getSaveEntry();
                auto oldValue = std::get<std::vector<float>>(saveEntry->value);

                // get the vector size dynamically
                int vectorSize = oldValue.size();
                std::vector<float> newValue;

                // get all spinners and build new value
                for (int i = 0; i < vectorSize; i++){
                    AetherElement spinnerElm;
                    Result rc = widget->getElementAtKey(spinnerElm, "Spinner_"+std::to_string(i));

                    Aether::Spinner* spinner = std::get<Aether::Spinner*>(spinnerElm);
                    newValue.push_back(spinner->value());
                }

                log("[Rune::WidgetBuilder::String] - Inner edit button value: '"+Rune::Save::ValueToString(newValue)+"'");

                widget->getEditButton()->setValue(Rune::Save::ValueToString(newValue));
                widget->getSaveEntry()->value = newValue;
            }
        });
        widget->getWindow()->addOverlay(ovl);
    });
}