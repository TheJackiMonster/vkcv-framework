#include "vkcv/camera/CameraManager.hpp"

namespace vkcv{

    //  m_window.e_mouseMove.add(this.onMouseMove);\

    CameraManager::CameraManager(Window &window, float width, float height):
    m_window(window), m_width(width), m_height(height)
    {

        m_camera.setPerspective( glm::radians(60.0f), m_width / m_height, 0.1f, 10.f);
        m_up = glm::vec3(0.0f, 1.0f, 0.0f);
        m_position = glm::vec3(0.0f, 0.0f, 0.0f);
        m_front = glm::vec3(0.0f, 0.0f, -1.0f);
        m_center = m_position + m_front;
        m_camera.lookAt(m_position, m_center, m_up);
        m_radius = 10.0f;
        m_cameraSpeed = 0.05f;
        m_roll = 0.0;
        m_pitch = 0.0;
        m_yaw = 0.0;
        bindCamera();
    }

    CameraManager::CameraManager(Window &window, float width, float height, glm::vec3 up, glm::vec3 position, glm::vec3 front):
    m_window(window), m_width(width), m_height(height), m_up(up), m_position(position), m_front(front)
    {
        m_camera.setPerspective( glm::radians(60.0f), m_width / m_height, 0.1f, 10.f);
        m_up = glm::vec3(0.0f, 1.0f, 0.0f);
        m_position = glm::vec3(0.0f, 0.0f, 0.0f);
        m_front = glm::vec3(0.0f, 0.0f, -1.0f);
        m_center = m_position + m_front;
        m_camera.lookAt(m_position, m_center, m_up);
        m_radius = 10.0f;
        m_cameraSpeed = 0.05f;
        m_roll = 0.0;
        m_pitch = 0.0;
        m_yaw = 0.0;
        bindCamera();
    }

    void CameraManager::bindCamera(){
        m_keyHandle = m_window.e_key.add( [&](int key, int scancode, int action, int mods) { this->keyCallback(key, scancode, action, mods); });
        m_mouseMoveHandle = m_window.e_mouseMove.add( [&]( double offsetX, double offsetY) {this->mouseMoveCallback( offsetX, offsetY);} );
        m_mouseScrollHandle =  m_window.e_mouseScroll.add([&](double offsetX, double offsetY) {this->scrollCallback( offsetX, offsetY);} );
        m_mouseButtonHandle = m_window.e_mouseButton.add([&] (int button, int action, int mods) {this->mouseButtonCallback( button,  action,  mods);});
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

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        m_yaw += xoffset;
        m_pitch += yoffset;

        if (m_pitch > 89.0f) {
            m_pitch = 89.0f;
        }
        if (m_pitch < -89.0f) {
            m_pitch = -89.0f;
        }

        glm::vec3 direction;
        direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        direction.y = sin(glm::radians(m_pitch));
        direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

        m_front = glm::normalize(direction);
        m_center = m_position + m_front;
        m_camera.lookAt(m_position, m_center, m_up);
    }

    void CameraManager::scrollCallback(double offsetX, double offsetY) {

        float fov = m_camera.getFov();
        fov -= (float) offsetY;
        if (fov < 1.0f) {
            fov = 1.0f;
        }
        if (fov > 45.0f) {
            fov = 45.0f;
        }
        m_camera.setFov(fov);
    }

    void CameraManager::keyCallback(int key, int scancode, int action, int mods) {
        switch (key) {
            case GLFW_KEY_W:
                //std::cout << "Move forward" << std::endl;
                m_position += m_cameraSpeed * m_front;
                m_center = m_position + m_front;
                m_camera.lookAt(m_position, m_center, m_up);
            case GLFW_KEY_S:
                //std::cout << "Move left" << std::endl;
                m_position -= m_cameraSpeed * m_front;
                m_center = m_position + m_front;
                m_position += m_cameraSpeed * m_front;
                m_camera.lookAt(m_position, m_center, m_up);
                break;
            case GLFW_KEY_A:
                //std::cout << "Move backward" << std::endl;
                m_position += m_cameraSpeed * m_front;
                m_position -= glm::normalize(glm::cross(m_front, m_up)) * m_cameraSpeed;
                m_center = m_position + m_front;
                m_camera.lookAt(m_position, m_center, m_up);
                break;
            case GLFW_KEY_D:
                //std::cout << "Move right" << std::endl;
                m_position += m_cameraSpeed * m_front;
                m_position += glm::normalize(glm::cross(m_front, m_up)) * m_cameraSpeed;
                m_center = m_position + m_front;
                m_camera.lookAt(m_position, m_center, m_up);
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(m_window.getWindow(), 1);
                break;
            default:
                break;
        }
    }
    Camera CameraManager::getCamera(){
        return m_camera;
    }

}