#include "vkcv/camera/TrackballCameraController.hpp"

namespace vkcv {

    TrackballCameraController::TrackballCameraController() {
        m_rotationActive = false;
        m_radius = 3.0f;
        m_cameraSpeed = 2.5f;
        m_scrollSensitivity = 0.05f;
    }

    void TrackballCameraController::setRadius(const float radius) {
        if (radius < 0.1f) {
            m_radius = 0.1f;
        }
        else {
            m_radius = radius;
        }
    }

    void TrackballCameraController::panView(double xOffset, double yOffset) {
        // handle yaw rotation
        float yaw = m_camera->getYaw() + xOffset * m_cameraSpeed;
        if (yaw < 0.0f) {
            yaw += 360.0f;
        }
        else if (yaw > 360.0f) {
            yaw -= 360.0f;
        }
        m_camera->setYaw(yaw);

        // handle pitch rotation
        float pitch = m_camera->getPitch() + yOffset * m_cameraSpeed;
        if (pitch < 0.0f) {
            pitch += 360.0f;
        }
        else if (pitch > 360.0f) {
            pitch -= 360.0f;
        }
        m_camera->setPitch(pitch);
    }

    glm::vec3 TrackballCameraController::updatePosition() {
        float yaw = m_camera->getYaw();
        float pitch = m_camera->getPitch();
        glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);

        glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), yAxis);
        glm::mat4 rotationX = glm::rotate(rotationY, glm::radians(pitch), xAxis);
        glm::vec3 translate = glm::vec3(0.0f, 0.0f, m_radius);
        translate = glm::vec3(rotationX * glm::vec4(translate, 0.0f));
        glm::vec3 center = m_camera->getCenter();
        glm::vec3 position = center +translate;
        m_camera->setPosition(position);
        glm::vec3 up = glm::vec3(rotationX * glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f));
        m_camera->setUp(up);
        return position;
    }

    glm::mat4 TrackballCameraController::updateView() {
        updatePosition();
        glm::vec3 position = m_camera->getPosition();
        glm::vec3 center = m_camera->getCenter();
        glm::vec3 up = m_camera->getUp();
        m_camera->lookAt(position, center, up);
        return m_camera->getView();
    }

    void TrackballCameraController::updateRadius(double offset) {
        setRadius(m_radius - offset * m_scrollSensitivity);
    }

    void TrackballCameraController::updateCamera(double deltaTime) {
        updateView();
    }

    void TrackballCameraController::keyCallback(int key, int scancode, int action, int mods) {}

    void TrackballCameraController::scrollCallback(double offsetX, double offsetY) {
        updateRadius(offsetY);
    }

    void TrackballCameraController::mouseMoveCallback(double x, double y) {
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

    void TrackballCameraController::mouseButtonCallback(int button, int action, int mods) {
        if(button == GLFW_MOUSE_BUTTON_2 && m_rotationActive == false && action == GLFW_PRESS){
            glfwSetInputMode(m_window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            m_rotationActive = true;
        }
        else if(button == GLFW_MOUSE_BUTTON_2 && m_rotationActive == true && action == GLFW_RELEASE){
            glfwSetInputMode(m_window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            m_rotationActive = false;
        }
    }
}