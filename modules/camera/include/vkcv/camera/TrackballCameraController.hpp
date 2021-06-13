#pragma once

#include "CameraController.hpp"

namespace vkcv {

    /**
     * @brief Used to orbit a camera around its center point.
     */
    class TrackballCameraController final : public CameraController {
    private:
        bool m_rotationActive;

        float m_cameraSpeed;
        float m_scrollSensitivity;
        float m_radius;


        /**
         * @brief Updates the position of the camera.
         * @return The updated camera position.
         */
        glm::vec3 updatePosition();

        /**
         * @brief Updates the view matrix of the camera.
         * @return The updated view matrix of the camera.
         */
        glm::mat4 updateView();

        /**
         * @brief Updates the current radius in respect to the @p offset.
         * @param offset The offset between the old and new radius.
         */
        void updateRadius(double offset);

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
         * @brief Sets @p radius as the new radius for orbiting around the camera's center point.
         * @param radius The new radius.
         */
        void setRadius(const float radius);

        /**
         * @brief Pans the view of the camera according to the pitch and yaw values and additional offsets @p xOffset
         * and @p yOffset.
         * @param[in] xOffset The offset added to the yaw value.
         * @param[in] yOffset The offset added to the pitch value.
         */
        void panView(double xOffset, double yOffset);

        /**
        * @brief Updates the camera object in respect to @p deltaTime.
        * @param deltaTime The time that has passed since last update.
        */
        void updateCamera(double deltaTime);

        /**
         * @brief A callback function for key events. Currently, the trackball camera does not support camera movement.
         * It can only orbit around its center point.
         * @param[in] key The keyboard key.
         * @param[in] scancode The platform-specific scancode.
         * @param[in] action The key action.
         * @param[in] mods The modifier bits.
         */
        void keyCallback(int key, int scancode, int action, int mods);

        /**
         * @brief A callback function for mouse scrolling events. Currently, this leads to changes in the field of view
         * of the camera object.
         * @param[in] offsetX The offset in horizontal direction.
         * @param[in] offsetY The offset in vertical direction.
         */
        void scrollCallback(double offsetX, double offsetY);

        /**
         * @brief A callback function for mouse movement events. Currently, this leads to panning the view of the
         * camera, if #mouseButtonCallback(int button, int action, int mods) enabled panning.
         * @param[in] x The horizontal mouse position.
         * @param[in] y The vertical mouse position.
         */
        void mouseMoveCallback(double x, double y);

        /**
         * @brief A callback function for mouse button events. Currently, the right mouse button enables panning the
         * view of the camera as long as it is pressed.
         * @param[in] button The mouse button.
         * @param[in] action The button action.
         * @param[in] mods The modifier bits.
         */
        void mouseButtonCallback(int button, int action, int mods);

    };

}