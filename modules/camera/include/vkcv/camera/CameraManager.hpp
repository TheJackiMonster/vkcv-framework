#pragma once

#include "TrackballCamera.hpp"
#include "vkcv/Window.hpp"
#include <GLFW/glfw3.h>
#include <functional>

namespace vkcv{

    class CameraManager{
    private:
        std::function<void(int, int, int, int)> e_keyHandle;
        std::function<void(double, double)> e_mouseMoveHandle;
        std::function<void(double, double)> e_mouseScrollHandle;
        std::function<void(int, int, int)> e_mouseButtonHandle;
        std::function<void(int, int)> e_resizeHandle;

        Window &m_window;
        Camera m_camera;
        float m_width;
        float m_height;
//        std::shared_ptr<vkcv::TrackballCamera> m_trackball;

        bool m_roationActive = false;
        double m_lastX ;
        double m_lastY ;

        void bindCamera();
        void keyCallback(int key, int scancode, int action, int mods);
        void scrollCallback( double offsetX, double offsetY);
        void mouseMoveCallback( double offsetX, double offsetY);
        void mouseButtonCallback(int button, int action, int mods);
        void resizeCallback(int width, int height);

    public:
        CameraManager(Window &window, float width, float height, glm::vec3 up = glm::vec3(0.0f,-1.0f,0.0f), glm::vec3 position = glm::vec3(0.0f,0.0f,0.0f));

        Camera &getCamera();
    };
}
