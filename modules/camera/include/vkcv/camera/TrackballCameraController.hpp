#pragma once
/**
 * @authors Vanessa Karolek, Josch Morgenstern, Sebastian Gaida,Tobias Frisch
 * @file include/vkcv/camera/TrackballCameraController.hpp
 * @brief TrackballCameraController class of the camera module for the vkcv framework. This class inherits from the base
 * class @#CameraController and enables camera objects to be orbited around a specific center point.
 */

#include "CameraController.hpp"

namespace vkcv::camera {

    /**
     * @addtogroup vkcv_camera
     * @{
     */

    /**
     * @brief Used to orbit a camera around its center point.
     */
    class TrackballCameraController final : public CameraController {
    private:
        bool m_rotationActive;
        float m_cameraSpeed;
        float m_scrollSensitivity;
        float m_pitch;
        float m_yaw;

        /**
         * @brief Updates the current radius of @p camera in respect to the @p offset.
         * @param[in] offset The offset between the old and new radius.
         * @param[in] camera The camera object.
         */
        void updateRadius(double offset, Camera &camera);

    public:

        /**
         * @brief The default constructor of the #TrackballCameraController.
         */
        TrackballCameraController();

        /**
         * @brief The destructor of the #TrackballCameraController (default behavior).
         */
        ~TrackballCameraController() = default;

        /**
         * @brief Pans the view of @p camera according to the pitch and yaw values and additional offsets @p xOffset
         * and @p yOffset.
         * @param[in] xOffset The offset added to the yaw value.
         * @param[in] yOffset The offset added to the pitch value.
         * @param[in] camera The camera object.
         */
        void panView(double xOffset, double yOffset, Camera &camera);

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