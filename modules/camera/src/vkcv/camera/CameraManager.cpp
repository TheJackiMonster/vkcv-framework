#include <iostream>
#include "vkcv/camera/CameraManager.hpp"

namespace vkcv{

    CameraManager::CameraManager(Window &window, float width, float height, glm::vec3 up, glm::vec3 position):
    m_window(window), m_width(width), m_height(height)
    {
        m_camera.setUp(up);
        m_camera.setPosition(position);
        m_camera.setPerspective( glm::radians(60.0f), m_width / m_height, 0.1f, 10.f);
        m_lastX = width/2.0;
        m_lastY = height/2.0;
        bindCamera();
    }

    void CameraManager::bindCamera(){
        e_keyHandle = m_window.e_key.add( [&](int key, int scancode, int action, int mods) { this->keyCallback(key, scancode, action, mods); });
        e_mouseMoveHandle = m_window.e_mouseMove.add( [&]( double offsetX, double offsetY) {this->mouseMoveCallback( offsetX, offsetY);} );
        e_mouseScrollHandle =  m_window.e_mouseScroll.add([&](double offsetX, double offsetY) {this->scrollCallback( offsetX, offsetY);} );
        e_mouseButtonHandle = m_window.e_mouseButton.add([&] (int button, int action, int mods) {this->mouseButtonCallback( button,  action,  mods);});
        e_resizeHandle = m_window.e_resize.add([&] (int width, int height) {this->resizeCallback( width, height);});
    }

    void CameraManager::mouseButtonCallback(int button, int action, int mods){
        if(button == GLFW_MOUSE_BUTTON_2 && m_roationActive == false && action == GLFW_PRESS){
            glfwSetInputMode(m_window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            m_roationActive = true;
        }else if(button == GLFW_MOUSE_BUTTON_2 && m_roationActive == true && action == GLFW_RELEASE){
            glfwSetInputMode(m_window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            m_roationActive = false;
        }
    }

    void CameraManager::mouseMoveCallback(double x, double y){

        float xoffset = x - m_lastX;
        float yoffset = m_lastY - y;
        m_lastX = x;
        m_lastY = y;

        if(!m_roationActive){
            return;
        }

        float sensitivity = 0.05f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        m_camera.panView( xoffset , yoffset );
    }

    void CameraManager::scrollCallback(double offsetX, double offsetY) {
        m_camera.changeFov(offsetY);
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
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(m_window.getWindow(), 1);
                break;
            default:
                break;
        }
    }

    void CameraManager::resizeCallback(int width, int height){
            m_camera.updateRatio(width, height);
    }

    Camera &CameraManager::getCamera(){
        return m_camera;
    }

}