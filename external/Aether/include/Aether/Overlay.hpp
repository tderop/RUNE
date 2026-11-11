#ifndef AETHER_OVERLAY_HPP
#define AETHER_OVERLAY_HPP

#include "Aether/Screen.hpp"

namespace Aether {
    /**
     * @brief An object representing an overlay.
     *
     * An Overlay is a screen that draws a transparent black background before any
     * children elements. While Aether handles displaying and closing overlays it
     * does not delete them. That part is left to you!
     */
    class Overlay : public Screen {
        private:
            friend Window;

            /** @brief Store value to check if overlay should be closed or not. */
            bool close_;
            /** @brief The function that runs when the overlay has been opened */
            std::function<void()> onOpenFunc_;
            /** @brief The function that runs when the overlay has been closed */
            std::function<void()> onCloseFunc_;
            
            /**
             * @brief Mark overlay to be able to used (readded) again
             * @note This shouldn't ever needed to be called apart from internally!
             */
            void reuse();
            void opened();
        public:
            /**
             * @brief Construct a new Overlay object
             */
            Overlay();

            /**
             * @brief Mark overlay to be closed
             */
            void close();

            /**
             * @brief Check if overlay is marked to be closed
             *
             * @return true if marked to close
             * @return false otherwise
             */
            bool shouldClose();

            /**
             * @brief Draws background then renders overlay
             */
            void render();

            /**
             * @brief Set a function to be run when the overlay is opened
             * @param f The function to run
             */
            void onOpen(std::function<void()> f);

            /**
             * @brief Set a function to be run when the overlay is closed
             * @param f The function to run
             */
            void onClose(std::function<void()> f);
    };
};

#endif