#pragma once

#include <Aether/Aether.hpp>

#include <Editor/Menus/Inventory.hpp>
#include <Editor/EditorWidget.hpp>
#include <Editor/EditorMenu.hpp>
#include <Editor/WidgetOverlay.hpp>

#include <Application/Application.hpp>
#include <Save/Save.hpp>

#include <any>
#include <vector>

namespace Rune {
    namespace Inventory {
        class Tile {
            private: 
                /** @brief The widget metadata */
                std::vector<std::any> widgetMeta;
                /** @brief The save entry */
                Save::Entry* saveEntry;
                /** @brief The current window of the application */
                Aether::Window* window;
                /** @brief The widget used by this tile */
                EditorWidget* widget=nullptr;
                /** @brief The menu that owns the widget */
                EditorMenu* menu=nullptr;
                /** @brief The inventoryItem shared ptr */
                Item inventoryItem;
                /** @brief The original item state before it was modified */
                std::shared_ptr<Item> originalInventoryItem;

                /** @brief The Tile BorderButton */
                Aether::BorderButton* tileBtn=nullptr;
                /** @brief The Tile's thumbnail */
                Aether::Image* tileThumb=nullptr;
                /** @brief The Tile's label rect (if applicable) */
                Aether::Rectangle* tileLabelRect=nullptr;
                /** @brief The Tile's label text */
                Aether::Text* tileLabelText=nullptr;
                /** @brief The Tile's edit image */
                Aether::Image* tileEditImg=nullptr;
                /** @brief The Tile's buff star */
                Aether::Image* buffStar=nullptr;

                /** @brief The state of the widget overlay */
                bool ovlOpen=false;
                /** @brief The widget overlay for editing properties */
                WidgetOverlay* ovl=nullptr;
                /** @brief The background frame for the large image in the widget overlay */
                Aether::Rectangle* ovlImgFrame=nullptr;
                /** @brief The large image in the widget overlay */
                Aether::Image* ovlImg=nullptr;
                /** @brief The properties box in the widget overlay */
                Aether::Rectangle* ovlPropertiesBox=nullptr;
                /** @brief The swap button in the widget overlay */
                Aether::BorderButton* ovlSwapBtn=nullptr;
                /** @brief The delete button in the widget overlay */
                Aether::BorderButton* ovlDeleteBtn=nullptr;
                /** @brief The 'reset' button in the widget overlay */
                Aether::BorderButton* ovlResetBtn=nullptr;
                /** @brief The big text in the image frame showing the name of the item */
                Aether::Text* ovlImgText=nullptr;
                /** @brief Text showing there is nothing available to edit right now */
                Aether::Text* ovlNoEditText=nullptr;

                /** @brief The primary value stepper */
                Aether::Stepper* ovlPrimaryValueStepper=nullptr; // For most items
                /** @brief The secondary flag dropdown */
                Aether::Dropdown* ovlSecondaryFlagDropdown=nullptr; // For equipment buffs
                /** @brief The secondary value stepper */
                Aether::Stepper* ovlSecondaryValueStepper=nullptr; // For equipment buff values
                
                /** @brief The stepper for armor stars */
                Aether::Stepper* ovlArmorStarStepper=nullptr; // For armor
                /** @brief The vector of available property editing elements */
                std::vector<Aether::Element*> visiblePropertyEditors;

                /** @brief The star count indicator for armor editing */
                Aether::Text* ovlArmorStars=nullptr;

                /** @brief The save file */
                Save::File* saveFile=nullptr;
                /** @brief The connection ID for the overlay closed connection */
                // size_t connId=0;

                /** @brief The swap UI trove */
                // Trove* swapTrove=nullptr;
                /** @brief The main application (just for easier reference) */
                Application* app=nullptr;

                /** 
                 * @brief Update the tile format
                 * @param type The tile type
                 * @param style The style of the label
                 * @param str The text to show on the label
                 */
                void updateTileFormat(TileType type, TileStyle style, std::string str);

                /**
                 * @brief Update the thumbnail image
                 */
                void updateThumbnail();

                /**
                 * @brief Add the buff star to the tile if there is a modifier
                 */
                void updateBuffStar();

                /**
                 * @brief Build the core UI elements of the overlay
                 */
                void buildOvl();

                /**
                 * @brief Update the text elements of the overlay to 
                 * match inventoryItem
                 */
                void updateOvlText();

                /**
                 * @brief Update the overlay's image element(s) to
                 * match the state of inventoryItem
                 */
                void updateOvlImg();

                /**
                 * @brief Update the editor steppers, dropdowns etc. to match
                 * the state of inventoryItem 
                 */
                void updatePropertyEditors();

                /**
                 * @brief Set the state of the overlay
                 * @param open Whether the overlay should be open or not
                 */
                void setOvlState(bool open);

            public: 
                /**
                 * @brief Make a new Tile object
                 * @param widget The widget used by the Tile
                 */
                Tile(EditorWidget* widget);

                /**
                 * @brief The callback for handling when the tile button is pressed
                 */
                void onPress();

                /**
                 * @brief The callback run when the delete button is pressed in the editor
                 */
                void onDelete();

                /**
                 * @brief The callback run when the swap button is pressed in the editor
                 */
                void onSwap();

                /**
                 * @brief The callback to run when the reset button is pressed
                 */
                void onReset();

                /**
                 * @brief The callback that handles when the overlay is closed
                 */
                void onClose(bool isSave);

                ~Tile();
        };
    } // namespace Inventory
} // namespace Rune