#pragma once

#include "TrackballCamera.hpp"
#include "vkcv/Window.hpp"
#include <GLFW/glfw3.h>
#include <functional>

namespace vkcv{

    class CameraManager{
    private:
        std::function<void(int, int, int, int)> m_keyHandle;
        std::function<void(double, double)> m_mouseMoveHandle;
        std::function<void(double, double)> m_mouseScrollHandle;
        std::function<void(int, int, int)> m_mouseButtonHandle;

        Window &m_window;
        Camera m_camera;
        TrackballCamera m_trackball;    // TODO: maybe there is a better way for switching between cameras?
        float m_width;
        float m_height;

        bool m_rotationActive = false;
        double m_lastX ;
        double m_lastY ;

        /**
         * @brief Binds the camera object to the window event handles
         */
        void bindCamera();

        /**
         * @brief A callback function for key events. Currently, 3D camera movement via W, A, S, D, E, Q and window closure via Escape are supported
         * @param[in] key The keyboard key
         * @param[in] scancode The platform-specific scancode
         * @param[in] action The key action
         * @param[in] mods The modifier bits
         */
        void keyCallback(int key, int scancode, int action, int mods);

        /**
         * @brief A callback function for mouse scrolling events. Currently, this leads to changes in the field of view of the camera object
         * @param[in] offsetX The offset in horizontal direction
         * @param[in] offsetY The offset in vertical direction
         */
        void scrollCallback( double offsetX, double offsetY);

        /**
         * @brief A callback function for mouse movement events. Currently, this leads to panning the view of the camera, if @fn mouseButtonCallback(int button, int action, int mods) enabled panning.
         * @param[in] offsetX The offset in horizontal direction
         * @param[in] offsetY The offset in vertical direction
         */
        void mouseMoveCallback( double offsetX, double offsetY);

        /**
         * @brief A callback function for mouse button events. Currently, the right mouse button enables panning the view of the camera as long as it is pressed.
         * @param[in] button The mouse button
         * @param[in] action The button action
         * @param[in] mods The modifier bits
         */
        void mouseButtonCallback(int button, int action, int mods);

    public:

        /**
         * @brief The constructor
         * @param[in] window The window
         * @param[in] width The width of the window
         * @param[in] height The height of the window
         * @param[in] up The up vector of the camera. Per default: @code{.cpp} up = glm::vec3(0.0f, -1.0f, 0.0f) @endcode
         * @param[in] position The position of the camera. Per default: @code{.cpp} position = glm::vec3(0.0f,0.0f,0.0f) @endcode
         */
        CameraManager(Window &window, float width, float height, glm::vec3 up = glm::vec3(0.0f,-1.0f,0.0f), glm::vec3 position = glm::vec3(0.0f,0.0f,0.0f));

        /**
         * @brief Gets the camera object
         * @return The camera object
         */
        Camera& getCamera();

        /**
         * @brief Gets the trackball camera object
         * @return The trackball camera object
         */
        TrackballCamera& getTrackballCamera();
    };
}
