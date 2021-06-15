#pragma once

#include "Camera.hpp"
#include "vkcv/Window.hpp"

namespace vkcv {

    /**
     * @brief Used as a base class for defining camera controller classes with different behaviors, e.g. the
     * #PilotCameraController.
     */
    class CameraController {

    public:

        /**
         * @brief The constructor of the #CameraController (default behavior).
         */
        CameraController() = default;

        /**
         * @brief Updates @p camera in respect to @p deltaTime.
         * @param[in] deltaTime The time that has passed since last update.
         * @param[in] camera The camera object.
         */
        virtual void updateCamera(double deltaTime, Camera &camera);

        /**
         * @brief A callback function for key events.
         * @param[in] key The keyboard key.
         * @param[in] scancode The platform-specific scancode.
         * @param[in] action The key action.
         * @param[in] mods The modifier bits.
         * @param[in] camera The camera object.
         */
        virtual void keyCallback(int key, int scancode, int action, int mods, Camera &camera);

        /**
         * @brief A callback function for mouse scrolling events.
         * @param[in] offsetX The offset in horizontal direction.
         * @param[in] offsetY The offset in vertical direction.
         * @param[in] camera The camera object.
         */
        virtual void scrollCallback( double offsetX, double offsetY, Camera &camera);

        /**
         * @brief A callback function for mouse movement events.
         * @param[in] x The horizontal mouse position.
         * @param[in] y The vertical mouse position.
         * @param[in] camera The camera object.
         */
        virtual void mouseMoveCallback(double offsetX, double offsetY, Camera &camera);

        /**
         * @brief A callback function for mouse button events.
         * @param[in] button The mouse button.
         * @param[in] action The button action.
         * @param[in] mods The modifier bits.
         * @param[in] camera The camera object.
         */
        virtual void mouseButtonCallback(int button, int action, int mods, Camera &camera);
    };

}