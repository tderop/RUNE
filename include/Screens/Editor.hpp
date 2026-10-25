#pragma once

#include <switch.h>
#include <vector>
#include <unordered_map>

#include <Aether/Aether.hpp>
#include <Input/Input.hpp>
#include <Save/Save.hpp>

#include <Application/Application.hpp>
#include <Widgets/EditorWidget.hpp>
#include <Widgets/WidgetOverlay.hpp>

/**
 * @brief A structure representing an editor tab. Used for building the editor user interface.
 */
struct EditorTab{
    std::vector<std::string> menus;
    std::unordered_map<std::string, std::vector<std::string>> menuEntries;
};

/**
 * @brief A type representing the main structure used to build the editor tabs.
 */
using EditorTabData = std::unordered_map<std::string, EditorTab>;

namespace Rune{
    namespace Editor{
        extern EditorTabData TabData;
    }
} // namespace Rune

/**
 * IMPORTANT KEYS
 * CurrentMamo (-830800173): 
 */

// struct TempElement{
//     Aether::Element* parentElm;
//     Aether::Element* elm;
// };

class EditorScreen : public Aether::Screen{
private:
    Rune::Application* app;

    Rune::Save::Slot slotCopy;

    Rune::Save::Slot* saveSlot = nullptr;
    Rune::Save::File* saveFile = new Rune::Save::File();
    Rune::Save::File* captionFile = new Rune::Save::File();

    std::vector<std::string> tabs;
    Trove* pageTrove;
    Trove* screenTrove;

    std::vector<EditorWidget*> tempWidgets;

    std::vector<Aether::ListButton*> tabButtons;
    Aether::Rectangle* topPanel;

    std::string selectedTab;
    int selectedMenu;

    std::vector<Aether::BorderButton*> menuLabels;
    Aether::Rectangle* menuLabelSelIndicator = nullptr;

    Aether::PopupList* popupList;
    WidgetOverlay* widgetOverlay;
    Aether::List* editorScroll;

public:
    // standard Aether class methods

    /**
     * @brief Code run by the screen when it is shown.
     */
    void onLoad() override;

    /**
     * @brief Code run by the screen when it is hidden.
     */
    void onUnload() override;

    EditorScreen(Rune::Application* app);
    ~EditorScreen();

    /**
     * @brief Build all of the screen's base elements. Should only be called once.
     */
    void buildScreen();

    /**
     * @brief Show a tab by its name. A tab is an option shown on the left menu (Stats, Equipment, Dev Editor...)
     */
    void showTab(std::string tabName);

    /**
     * @brief Show a menu by its index. The maximum value of menuIdx depends on the tab being viewed. (For example, the menus for the Stats tab are Stats, Coordinates, and Completionism)
     */
    void showMenu(int menuIdx);

    /**
     * @brief Navigate to the next menu on the current tab.
     */
    void nextMenu(InputSystem& inputService, InputState state);

    /**
     * @brief Navigate to the previous menu on the current tab.
     */
    void prevMenu(InputSystem& inputService, InputState state);

    /**
     * @brief Generate a widget for interacting with a particular key in save data. These are dev-editor style widgets, used for modifying decoded, raw values.
     */
    EditorWidget* getWidgetFromSaveKey(const std::string& saveKey);

    /**
     * @brief Generates, adds and displays a widget from its save key.
     */
    void makeWidget(const std::string &saveKey);

    /**
     * @brief Get the screen's popup list. Mostly used by widgets to show the popup list in order to be modified.
     */
    Aether::PopupList* getPopupList(); 

    /**
     * @brief Set the save slot currently being modified by the editor. Without a save slot, the editor will fail to work - for obvious reasons.
     */
    void editSaveSlot(Rune::Save::Slot* saveSlot);

    /**
     * @brief Reposition the blue-green rectangle used to indicate which tab is currently selected.
     */
    void positionTabSelectIndicator(Aether::Rectangle* selectIndicator, Aether::ListButton* targetBtn);

    /**
     * @brief Delete all currently visible dev-editor style widgets in the editor.
     */
    void cleanEditorWidgets();
};