#pragma once

#include "TrackballCamera.hpp"
#include "vkcv/Window.hpp"
#include <GLFW/glfw3.h>
#include <functional>

namespace vkcv{

    class CameraManager{
    private:
        std::function <void(int, int, int ,int )> m_keyHandle;
        std::function <void(double, double )> m_mouseMoveHandle;
        std::function <void(double, double )> m_mouseScrollHandle;

        Window &m_window;
        Camera m_camera;
        float m_width;
        float m_height;
        std::shared_ptr<vkcv::TrackballCamera> m_trackball;
        glm::vec3 m_up;
        glm::vec3 m_position;
        glm::vec3 m_front;
        glm::vec3 m_center;
        float m_radius;
        float m_cameraSpeed;
        float m_roll;
        float m_pitch;
        float m_yaw;

        bool m_firstMouse = true;
        double m_lastX ;
        double m_lastY ;

        void bindCamera();
        void keyCallback(int key, int scancode, int action, int mods);
        void scrollCallback( double offsetX, double offsetY);
        void mouseMoveCallback( double offsetX, double offsetY);

    public:
        CameraManager(Window &window, float width, float height);
        CameraManager(Window &window, float width, float height, glm::vec3 up, glm::vec3 position, glm::vec3 front);

        Camera getCamera();
    };
}
