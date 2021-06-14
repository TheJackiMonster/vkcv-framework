#pragma once

#include <vkcv/camera/CameraController.hpp>

namespace vkcv {

    /**
     * @brief Used to move around a camera object in world space.
     */
    class PilotCameraController final : public CameraController {
    private:
        // camera movement indicators
        bool m_forward;
        bool m_backward;
        bool m_upward;
        bool m_downward;
        bool m_left;
        bool m_right;

        bool m_rotationActive;

        float m_cameraSpeed;

        int m_fov_nsteps;
        float m_fov_min;
        float m_fov_max;


        /**
         * @brief Updates the position of the camera with respect to @p deltaTime.
         * @param[in] deltaTime The time that has passed since last update.
         * @return The updated camera position.
         */
        glm::vec3 updatePosition(double deltaTime, Camera &camera);

        /**
         * @brief Updates the view matrix of the camera with respect to @p deltaTime.
         * @param deltaTime The time that has passed since last update.
         * @return The updated view matrix of the camera.
         */
        glm::mat4 updateView(double deltaTime, Camera &camera);

        /**
         * @brief Indicates forward movement of the camera depending on the performed @p action.
         * @param[in] action The performed action.
         */
        void moveForward(int action);

        /**
         * @brief Indicates backward movement of the camera depending on the performed @p action.
         * @param[in] action The performed action.
         */
        void moveBackward(int action);

        /**
         * @brief Indicates left movement of the camera depending on the performed @p action.
         * @param[in] action The performed action.
         */
        void moveLeft(int action);

        /**
         * @brief Indicates right movement of the camera depending on the performed @p action.
         * @param[in] action The performed action.
         */
        void moveRight(int action);

        /**
         * @brief Indicates upward movement of the camera depending on the performed @p action.
         * @param[in] action The performed action.
         */
        void moveUpward(int action);

        /**
         * @brief Indicates downward movement of the camera depending on the performed @p action.
         * @param[in] action The performed action.
         */
        void moveDownward(int action);

    public:

        /**
         * @brief The default constructor of the #PilotCameraController.
         */
        PilotCameraController();

        /**
         * @brief The destructor of the #PilotCameraController (default behavior).
         */
        ~PilotCameraController() = default;

        /**
         * @brief Changes the field of view of the camera with an @p offset in degrees.
         * @param[in] offset The offset in degrees.
         */
        void changeFov(double offset, Camera &camera);

        /**
         * @brief Pans the view of the camera according to the pitch and yaw values and additional offsets @p xOffset
         * and @p yOffset.
         * @param[in] xOffset The offset added to the yaw value.
         * @param[in] yOffset The offset added to the pitch value.
         */
        void panView(double xOffset, double yOffset, Camera &camera);

        /**
         * @brief Updates the camera object in respect to @p deltaTime.
         * @param deltaTime The time that has passed since last update.
         */
        void updateCamera(double deltaTime, Camera &camera);

        /**
         * @brief A callback function for key events. Currently, 3D camera movement via W, A, S, D, E, Q are supported.
         * @param[in] key The keyboard key.
         * @param[in] scancode The platform-specific scancode.
         * @param[in] action The key action.
         * @param[in] mods The modifier bits.
         */
        void keyCallback(int key, int scancode, int action, int mods, Camera &camera);

        /**
         * @brief A callback function for mouse scrolling events. Currently, this leads to changes in the field of view
         * of the camera object.
         * @param[in] offsetX The offset in horizontal direction.
         * @param[in] offsetY The offset in vertical direction.
         */
        void scrollCallback(double offsetX, double offsetY, Camera &camera);

        /**
         * @brief A callback function for mouse movement events. Currently, this leads to panning the view of the camera,
         * if #mouseButtonCallback(int button, int action, int mods) enabled panning.
         * @param[in] x The horizontal mouse position
         * @param[in] y The vertical mouse position
         */
        void mouseMoveCallback(double x, double y, Camera &camera);

        /**
         * @brief A callback function for mouse button events. Currently, the right mouse button enables panning the
         * view of the camera as long as it is pressed.
         * @param[in] button The mouse button
         * @param[in] action The button action
         * @param[in] mods The modifier bits
         */
        void mouseButtonCallback(int button, int action, int mods, Camera &camera);
    };

}