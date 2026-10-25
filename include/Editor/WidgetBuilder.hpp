#pragma once

#include <unordered_map>
#include <string>

#include <Save/Save.hpp>

class EditorWidget;

namespace Rune {
    namespace WidgetBuilder {
        /**
         * @brief Constructor for boolean values.
         * Will apply to bool & revival bool
         */
        void Bool(EditorWidget* widget);

        /**
         * @brief Constructor for float32 values.
         * Will apply to f32
         */
        void Float(EditorWidget* widget);

        /**
         * @brief Constructor for signed32 values.
         * Will apply to s32
         */
        void Int(EditorWidget* widget);

        /**
         * @brief Constructor for string values.
         * Will apply to string32, string64, and string256. Make sure to truncate oversized values during save recompilation.
         */
        void String(EditorWidget* widget);

        /**
         * @brief Constructor for vector values.
         */
        void Vector(EditorWidget* widget);

        /**
         * @brief Constructor for the stamina editor.
         */
        void Stamina(EditorWidget* widget);

        /**
         * @brief Constructor for the hearts editor.
         */
        void Hearts(EditorWidget* widget); 

    } // namespace WidgetBuilder
} // namespace Rune

// Moved this logic into WidgetBuilder because it didn't really make sense for EditorWidget to have these definitions
using WidgetBuilderFunc = void(*)(EditorWidget*);
using WidgetBuilderMap = std::unordered_map<Rune::Save::DataType, WidgetBuilderFunc>;
using UniqueBuilderMap = std::unordered_map<std::string, WidgetBuilderFunc>;

/**
 * @brief A table mapping Rune::Save::DataTypes to their respective widget builder functions.
 */
inline const WidgetBuilderMap WIDGET_BUILDERS = {
    {Rune::Save::RuneBool, Rune::WidgetBuilder::Bool},
    {Rune::Save::RuneRevivalBool, Rune::WidgetBuilder::Bool},
    {Rune::Save::RuneS32, Rune::WidgetBuilder::Int},
    {Rune::Save::RuneF32, Rune::WidgetBuilder::Float},
    {Rune::Save::RuneString32, Rune::WidgetBuilder::String},
    {Rune::Save::RuneString64, Rune::WidgetBuilder::String},
    {Rune::Save::RuneString256, Rune::WidgetBuilder::String},
    {Rune::Save::RuneVector2f, Rune::WidgetBuilder::Vector},
    {Rune::Save::RuneVector3f, Rune::WidgetBuilder::Vector},
    {Rune::Save::RuneVector4f, Rune::WidgetBuilder::Vector}
};

/**
 * @brief A table mapping std::strings to their respective widget builder override functions.
 */
inline const UniqueBuilderMap UNIQUE_WIDGET_BUILDERS = {
    {"Stamina", Rune::WidgetBuilder::Stamina},
    {"Hearts", Rune::WidgetBuilder::Hearts}
};