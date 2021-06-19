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

        m_gamepadX = 0.0f;
        m_gamepadY = 0.0f;
        m_gamepadZ = 0.0f;

        m_rotationActive = false;

        m_cameraSpeed = 2.5f;

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
	
		position += distance * (getDirectionFactor(m_forward, m_backward) + m_gamepadZ) * front;
		position += distance * (getDirectionFactor(m_left, m_right) + m_gamepadX) * left;
		position += distance * (getDirectionFactor(m_upward, m_downward) + m_gamepadY) * up;
	
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

    void PilotCameraController::gamepadCallback(int gamepadIndex, Camera &camera) {
        GLFWgamepadstate gamepadState;
        glfwGetGamepadState(gamepadIndex, &gamepadState);

        float sensitivity = 0.05f;
        double threshold = 0.1;    // todo: needs further investigation!

        // handle rotations
        double stickRightX = static_cast<double>(gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]);
        double stickRightY = static_cast<double>(gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);

        double rightXVal = glm::clamp(std::abs(stickRightX) - threshold, 0.0, 1.0)
                * copysign(1.0, stickRightX) * sensitivity;
        double rightYVal = glm::clamp(std::abs(stickRightY) - threshold, 0.0, 1.0)
                * copysign(1.0, stickRightY) * sensitivity;
        panView(rightXVal, rightYVal, camera);

        // handle zooming
        double zoom = static_cast<double>((gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]
                - gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER])
                * sensitivity * 0.5);
        changeFov(zoom, camera);

        // handle translation
        m_gamepadY = gamepadState.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] - gamepadState.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER];
        float stickLeftX = gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        float stickLeftY = gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        m_gamepadZ = glm::clamp(std::abs(stickLeftY) - threshold, 0.0, 1.0)
                     * -copysign(1.0, stickLeftY);
        m_gamepadX = glm::clamp(std::abs(stickLeftX) - threshold, 0.0, 1.0)
                     * -copysign(1.0, stickLeftX);
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