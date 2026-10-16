#pragma once

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

        // /**
        //  * @brief Constructor for vector3 values.
        //  */
        // void Vector3(EditorWidget* widget);

        // /**
        //  * @brief Constructor for vector4 values.
        //  */
        // void Vector4(EditorWidget* widget);

    } // namespace WidgetBuilder
} // namespace Rune