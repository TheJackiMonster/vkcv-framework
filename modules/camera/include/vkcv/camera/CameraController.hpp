#pragma once

#include "Camera.hpp"
#include "vkcv/Window.hpp"

namespace vkcv {

    /**
     * @brief Used as a base class for defining camera controller classes with different behaviors, e.g. the
     * #PilotCameraController.
     */
    class CameraController {
    protected:
        Camera* m_camera;
        Window* m_window;
        double m_lastX;
        double m_lastY;

    public:

        /**
         * @brief The constructor of the #CameraController (default behavior).
         */
        CameraController() = default;

        /**
         * @brief Gets the camera object.
         * @return The camera object.
         */
        Camera& getCamera();

        /**
         * @brief Sets @p camera as the new camera object.
         * @param camera The new camera object.
         */
        virtual void setCamera(Camera &camera);

        /**
         * @brief Sets @p window as the new window object.
         * @param window The new window object.
         */
        void setWindow(Window &window);

        /**
         * @brief Updates the camera object in respect to @p deltaTime.
         * @param deltaTime The time that has passed since last update.
         */
        virtual void updateCamera(double deltaTime);

        /**
         * @brief A callback function for key events.
         * @param[in] key The keyboard key.
         * @param[in] scancode The platform-specific scancode.
         * @param[in] action The key action.
         * @param[in] mods The modifier bits.
         */
        virtual void keyCallback(int key, int scancode, int action, int mods);

        /**
         * @brief A callback function for mouse scrolling events.
         * @param[in] offsetX The offset in horizontal direction.
         * @param[in] offsetY The offset in vertical direction.
         */
        virtual void scrollCallback( double offsetX, double offsetY);

        /**
         * @brief A callback function for mouse movement events.
         * @param[in] x The horizontal mouse position.
         * @param[in] y The vertical mouse position.
         */
        virtual void mouseMoveCallback(double offsetX, double offsetY);

        /**
         * @brief A callback function for mouse button events.
         * @param[in] button The mouse button.
         * @param[in] action The button action.
         * @param[in] mods The modifier bits.
         */
        virtual void mouseButtonCallback(int button, int action, int mods);
    };

}