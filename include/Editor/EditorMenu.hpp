/**
 * @brief An object representing a RUNE "menu". 
 * A menu is a collection of widgets used to interact with the BotW save file through RUNE's UI.
 */
#pragma once

#include <string>
#include <vector>

#include <Editor/EditorWidget.hpp>
#include <Editor/WidgetOverlay.hpp>
#include <Utils/Trove.hpp>
#include <Save/Save.hpp>

class EditorMenu {
    private:
        /** @brief The name of the menu (e.g. 'Weapons', or 'Items', or 'Dev Editor') */
        std::string _menuName;

        /** @brief Names of entries used by this EditorMenu. Primarily for entry-based menus. */
        std::vector<std::string> _dataNames;

        /** @brief The main scrolling frame for the menu */
        Aether::List* _list = nullptr;

        /** @brief The save file */
        Rune::Save::File* _saveFile = nullptr;

        /** @brief The main application window */
        Aether::Window* _window = nullptr;

        /** @brief The current screen */
        Aether::Screen* _screen = nullptr;

        /** @brief The popupList */
        Aether::PopupList* _popupList = nullptr;

        /** @brief The widgetOverlay */
        WidgetOverlay* _widgetOverlay = nullptr;

        /** @brief The body container */
        Aether::Container* _body = nullptr;

        /** @brief The widgets held by this menu */
        std::vector<EditorWidget*> widgets;

        /** @brief The element trove (cleared when menu is destroyed) */
        Trove* trove = nullptr;

    public:
        /** @brief Create a new EditorMenu object */
        EditorMenu(std::string menuName, Aether::Window* window, 
            Aether::Screen* screen);

        /** @brief Destroy the EditorMenu */
        ~EditorMenu();

        // Disable copying (forgot to do this for a lot of other classes that own objects but meh doesn't matter too much)
        EditorMenu(const EditorMenu&) = delete;


        /** @brief Build the menu UI */
        void build(std::string builderName="");

        /** @brief Add a widget to the menu */
        void addWidget(EditorWidget* newWidget);

        /** @brief Add an element to this menu. Will be destroyed alongside the menu. */
        void addElement(AetherElement element);

        /** @brief Get the menu name */
        std::string menuName();

        /** @brief Set the dataNames */
        void setDataNames(std::vector<std::string> dataNames);

        /** @brief Get the dataNames */
        std::vector<std::string> dataNames();

        /** @brief Set the scrollingframe used by the menu (primarily for entry-based menus) */
        void setList(Aether::List* list);

        /** @brief Get the scrollingframe used by the menu */
        Aether::List* list();

        /** @brief Set the save file to be accessed by the menu */
        void setSaveFile(Rune::Save::File* saveFile);

        /** @brief Access the window */
        Aether::Window* window();

        /** @brief Set the popupList */
        void setPopupList(Aether::PopupList* popupList);

        /** @brief Get the popupList */
        Aether::PopupList* popupList();

        /** @brief Set the widgetOverlay */
        void setWidgetOverlay(WidgetOverlay* widgetOverlay);

        /** @brief Get the widgetOverlay */
        WidgetOverlay* widgetOverlay();

        /** @brief Get the save file pointer */
        Rune::Save::File* saveFile();

        /** @brief Get the menu body */
        Aether::Container* body();
};