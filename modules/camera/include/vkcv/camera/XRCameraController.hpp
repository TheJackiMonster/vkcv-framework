#pragma once
/**
 * @authors Tobias Frisch
 * @file include/vkcv/camera/XRCameraController.hpp
 * @brief XRCameraController class of the camera module for the vkcv framework. This class inherits from the base
 * class @#CameraController and enables camera objects to be orbited around a specific center point.
 */

#include "CameraController.hpp"

#include <openxr/openxr.h>

namespace vkcv::camera {

    /**
     * @addtogroup vkcv_camera
     * @{
     */

    /**
     * @brief Used to control the camera via an HMD.
     */
    class XRCameraController final : public CameraController {
    private:
        XrInstance m_instance;
        XrSystemId m_system_id;
        XrSession m_session;
        XrSpace m_space;

    public:

        /**
         * @brief The default constructor of the #XRCameraController.
         */
        XRCameraController();

        /**
         * @brief The destructor of the #XRCameraController (default behavior).
         */
        ~XRCameraController();

        /**
         * @brief Updates @p camera in respect to @p deltaTime.
         * @param[in] deltaTime The time that has passed since last update.
         * @param[in] camera The camera object
         */
        void updateCamera(double deltaTime, Camera &camera) override;

        /**
         * @brief A callback function for key events. Currently, the trackball camera does not support camera movement.
         * It can only orbit around its center point.
         * @param[in] key The keyboard key.
         * @param[in] scancode The platform-specific scancode.
         * @param[in] action The key action.
         * @param[in] mods The modifier bits.
         * @param[in] camera The camera object.
         */
        void keyCallback(int key, int scancode, int action, int mods, Camera &camera) override;

        /**
         * @brief A callback function for mouse scrolling events. Currently, this leads to changes in the field of view
         * of the camera object.
         * @param[in] offsetX The offset in horizontal direction.
         * @param[in] offsetY The offset in vertical direction.
         * @param[in] camera The camera object.
         */
        void scrollCallback(double offsetX, double offsetY, Camera &camera) override;

        /**
         * @brief A callback function for mouse movement events. Currently, this leads to panning the view of the
         * camera, if #mouseButtonCallback(int button, int action, int mods) enabled panning.
         * @param[in] xoffset The horizontal mouse position.
         * @param[in] yoffset The vertical mouse position.
         * @param[in] camera The camera object.
         */
        void mouseMoveCallback(double xoffset, double yoffset, Camera &camera) override;

        /**
         * @brief A callback function for mouse button events. Currently, the right mouse button enables panning the
         * view of the camera as long as it is pressed.
         * @param[in] button The mouse button.
         * @param[in] action The button action.
         * @param[in] mods The modifier bits.
         * @param[in] camera The camera object.
         */
        void mouseButtonCallback(int button, int action, int mods, Camera &camera) override;

        /**
         * @brief A callback function for gamepad input events.
         * @param gamepadIndex The gamepad index.
         * @param camera The camera object.
         * @param frametime The current frametime.
         */
        void gamepadCallback(int gamepadIndex, Camera &camera, double frametime) override;
    };

    /** @} */

}