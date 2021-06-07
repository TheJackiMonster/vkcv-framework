#include <iostream>
#include "vkcv/camera/CameraManager.hpp"

namespace vkcv{

    CameraManager::CameraManager(Window &window, float width, float height, glm::vec3 up, glm::vec3 position):
    m_window(window), m_width(width), m_height(height)
    {
        m_camera.setUp(up);
        m_camera.setPosition(position);
        m_camera.setPerspective( glm::radians(60.0f), m_width / m_height, 0.1f, 10.f);
        m_trackball.setUp(up);
        m_trackball.setPosition(position);
        m_trackball.setPerspective( glm::radians(60.0f), m_width / m_height, 0.1f, 10.f);
        m_lastX = width/2.0;
        m_lastY = height/2.0;
        bindCamera();
    }

    void CameraManager::bindCamera(){
        m_keyHandle = m_window.e_key.add( [&](int key, int scancode, int action, int mods) { this->keyCallback(key, scancode, action, mods); });
        m_mouseMoveHandle = m_window.e_mouseMove.add( [&]( double offsetX, double offsetY) {this->mouseMoveCallback( offsetX, offsetY);} );
        m_mouseScrollHandle =  m_window.e_mouseScroll.add([&](double offsetX, double offsetY) {this->scrollCallback( offsetX, offsetY);} );
        m_mouseButtonHandle = m_window.e_mouseButton.add([&] (int button, int action, int mods) {this->mouseButtonCallback( button,  action,  mods);});
    }

    void CameraManager::mouseButtonCallback(int button, int action, int mods){
        if(button == GLFW_MOUSE_BUTTON_2 && m_rotationActive == false && action == GLFW_PRESS){
            glfwSetInputMode(m_window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            m_rotationActive = true;
        }else if(button == GLFW_MOUSE_BUTTON_2 && m_rotationActive == true && action == GLFW_RELEASE){
            glfwSetInputMode(m_window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            m_rotationActive = false;
        }
    }

    void CameraManager::mouseMoveCallback(double x, double y){
        float xoffset = x - m_lastX;
        float yoffset = m_lastY - y;
        m_lastX = x;
        m_lastY = y;

        if(!m_rotationActive){
            return;
        }

        float sensitivity = 0.05f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        m_camera.panView( xoffset , yoffset );
        m_trackball.panView( xoffset , yoffset );
    }

    void CameraManager::scrollCallback(double offsetX, double offsetY) {
        m_camera.changeFov(offsetY);
        m_trackball.changeFov(offsetY);
    }

    void CameraManager::keyCallback(int key, int scancode, int action, int mods) {
        switch (key) {
            case GLFW_KEY_W:
                m_camera.moveForward(action);
                break;
            case GLFW_KEY_S:
                m_camera.moveBackward(action);
                break;
            case GLFW_KEY_A:
                m_camera.moveLeft(action);
                break;
            case GLFW_KEY_D:
                m_camera.moveRight(action);
                break;
            case GLFW_KEY_E:
                m_camera.moveTop(action);
                break;
            case GLFW_KEY_Q:
                m_camera.moveBottom(action);
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(m_window.getWindow(), 1);
                break;
            default:
                break;
        }
    }

    Camera& CameraManager::getCamera(){
        return m_camera;
    }

    TrackballCamera& CameraManager::getTrackballCamera() {
        return m_trackball;
    }


}