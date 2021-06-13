#include "vkcv/camera/PilotCameraController.hpp"
#include <iostream>

namespace vkcv {

    PilotCameraController::PilotCameraController() {
        m_forward = false;
        m_backward = false;
        m_upward = false;
        m_downward = false;
        m_left = false;
        m_right = false;

        m_rotationActive = false;

        m_cameraSpeed = 2.0f;

        m_fov_nsteps = 100;
        m_fov_min = 10;
        m_fov_max = 120;

        m_lastX = 0.0;
        m_lastY = 0.0;
    }

    void PilotCameraController::changeFov(double offset){
        float fov = m_camera->getFov();
        float fov_range = m_fov_max - m_fov_min;
        float fov_stepsize = glm::radians(fov_range)/m_fov_nsteps;
        fov -= (float) offset*fov_stepsize;
        if (fov < glm::radians(m_fov_min)) {
            fov = glm::radians(m_fov_min);
        }
        if (fov > glm::radians(m_fov_max)) {
            fov = glm::radians(m_fov_max);
        }
        m_camera->setFov(fov);
    }

    void PilotCameraController::panView(double xOffset, double yOffset) {
        // handle yaw rotation
        float yaw = m_camera->getYaw() + xOffset;
        if (yaw < -180.0f) {
            yaw += 360.0f;
        }
        else if (yaw > 180.0f) {
            yaw -= 360.0f;
        }
        m_camera->setYaw(yaw);

        // handle pitch rotation
        float pitch = m_camera->getPitch() - yOffset;
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }
        m_camera->setPitch(pitch);
    }

    glm::mat4 PilotCameraController::updateView(double deltaTime){
        updatePosition(deltaTime);
        glm::vec3 position = m_camera->getPosition();
        glm::vec3 front = m_camera->getFront();
        glm::vec3 up = m_camera->getUp();
        m_camera->lookAt(position, position + front, up);
        return m_camera->getView();
    }

    glm::vec3 PilotCameraController::updatePosition(double deltaTime ){
        glm::vec3 position = m_camera->getPosition();
        glm::vec3 front = m_camera->getFront();
        glm::vec3 up = m_camera->getUp();
        position += (m_cameraSpeed * front * static_cast<float> (m_forward) * static_cast<float>(deltaTime));
        position -= (m_cameraSpeed * front * static_cast<float> (m_backward) * static_cast<float>(deltaTime));
        position += (glm::normalize(glm::cross(front, up)) * m_cameraSpeed * static_cast<float> (m_left) * static_cast<float>(deltaTime));
        position -= (glm::normalize(glm::cross(front, up)) * m_cameraSpeed * static_cast<float> (m_right) * static_cast<float>(deltaTime));
        position -= up * m_cameraSpeed * static_cast<float> (m_upward) * static_cast<float>(deltaTime);
        position += up * m_cameraSpeed * static_cast<float> (m_downward) * static_cast<float>(deltaTime);
        m_camera->setPosition(position);
        return position;
    }

    void PilotCameraController::setCamera(Camera &camera) {
        m_camera = &camera;
        m_camera->setYaw(180.0f);
    }

    void PilotCameraController::updateCamera(double deltaTime) {
        updateView(deltaTime);
    }

    void PilotCameraController::keyCallback(int key, int scancode, int action, int mods) {
        switch (key) {
            case GLFW_KEY_W:
                moveForward(action);
                break;
            case GLFW_KEY_S:
                moveBackward(action);
                break;
            case GLFW_KEY_A:
                moveLeft(action);
                break;
            case GLFW_KEY_D:
                moveRight(action);
                break;
            case GLFW_KEY_E:
                moveUpward(action);
                break;
            case GLFW_KEY_Q:
                moveDownward(action);
                break;
            default:
                break;
        }
    }

    void PilotCameraController::scrollCallback(double offsetX, double offsetY) {
        changeFov(offsetY);
    }

    void PilotCameraController::mouseMoveCallback(double x, double y) {
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

        panView(xoffset , yoffset);
    }

    void PilotCameraController::mouseButtonCallback(int button, int action, int mods) {
        if(button == GLFW_MOUSE_BUTTON_2 && m_rotationActive == false && action == GLFW_PRESS){
            glfwSetInputMode(m_window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            m_rotationActive = true;
        }
        else if(button == GLFW_MOUSE_BUTTON_2 && m_rotationActive == true && action == GLFW_RELEASE){
            glfwSetInputMode(m_window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            m_rotationActive = false;
        }
    }


    void PilotCameraController::moveForward(int action){
        m_forward = static_cast<bool>(action);
    }

    void PilotCameraController::moveBackward(int action){
        m_backward = static_cast<bool>(action);
    }

    void PilotCameraController::moveLeft(int action){
        m_left = static_cast<bool>(action);
    }

    void PilotCameraController::moveRight(int action){
        m_right = static_cast<bool>(action);
    }

    void PilotCameraController::moveUpward(int action){
        m_upward = static_cast<bool>(action);
    }

    void PilotCameraController::moveDownward(int action){
        m_downward = static_cast<bool>(action);
    }

}