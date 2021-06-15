#include "vkcv/camera/PilotCameraController.hpp"

#include <GLFW/glfw3.h>

namespace vkcv::camera {

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
    }

    void PilotCameraController::changeFov(double offset, Camera &camera){
        float fov = camera.getFov();
        float fov_range = m_fov_max - m_fov_min;
        float fov_stepsize = glm::radians(fov_range) / static_cast<float>(m_fov_nsteps);
        fov -= (float) offset*fov_stepsize;
        if (fov < glm::radians(m_fov_min)) {
            fov = glm::radians(m_fov_min);
        }
        if (fov > glm::radians(m_fov_max)) {
            fov = glm::radians(m_fov_max);
        }
        camera.setFov(fov);
    }

    void PilotCameraController::panView(double xOffset, double yOffset, Camera &camera) {
        // handle yaw rotation
        float yaw = camera.getYaw() + xOffset;
        if (yaw < -180.0f) {
            yaw += 360.0f;
        }
        else if (yaw > 180.0f) {
            yaw -= 360.0f;
        }
        camera.setYaw(yaw);

        // handle pitch rotation
        float pitch = camera.getPitch() - yOffset;
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }
        camera.setPitch(pitch);
    }
    
    constexpr float getDirectionFactor(bool positive, bool negative) {
    	return static_cast<float>(positive) - static_cast<float>(negative);
    }

    void PilotCameraController::updateCamera(double deltaTime, Camera &camera) {
		glm::vec3 position = camera.getPosition();
	
		const glm::vec3 front = camera.getFront();
		const glm::vec3 up = camera.getUp();
		const glm::vec3 left = glm::normalize(glm::cross(front, up));
	
		const float distance = m_cameraSpeed * static_cast<float>(deltaTime);
	
		position += distance * getDirectionFactor(m_forward, m_backward) * front;
		position += distance * getDirectionFactor(m_left, m_right) * left;
		position += distance * getDirectionFactor(m_upward, m_downward) * up;
	
		camera.lookAt(position, position + front, up);
    }

    void PilotCameraController::keyCallback(int key, int scancode, int action, int mods, Camera &camera) {
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

    void PilotCameraController::scrollCallback(double offsetX, double offsetY, Camera &camera) {
        changeFov(offsetY, camera);
    }

    void PilotCameraController::mouseMoveCallback(double xoffset, double yoffset, Camera &camera) {
        if(!m_rotationActive){
            return;
        }

        float sensitivity = 0.05f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        panView(xoffset , yoffset, camera);
    }

    void PilotCameraController::mouseButtonCallback(int button, int action, int mods, Camera &camera) {
        if(button == GLFW_MOUSE_BUTTON_2 && m_rotationActive == false && action == GLFW_PRESS){
            m_rotationActive = true;
        }
        else if(button == GLFW_MOUSE_BUTTON_2 && m_rotationActive == true && action == GLFW_RELEASE){
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