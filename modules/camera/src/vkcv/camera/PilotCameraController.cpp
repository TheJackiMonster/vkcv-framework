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
        // update only if there is (valid) input
        if (offset == 0.0) {
            return;
        }

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
        // update only if there is (valid) input
        if (xOffset == 0.0 && yOffset == 0.0) {
            return;
        }

        // handle yaw rotation
        float yaw = camera.getYaw() + static_cast<float>(xOffset) * 90.0f * m_cameraSpeed;
        camera.setYaw(yaw);

        // handle pitch rotation
        float pitch = camera.getPitch() - static_cast<float>(yOffset) * 90.0f * m_cameraSpeed;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
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
            	m_forward = static_cast<bool>(action);
                break;
            case GLFW_KEY_S:
            	m_backward = static_cast<bool>(action);
                break;
            case GLFW_KEY_A:
            	m_left = static_cast<bool>(action);
                break;
            case GLFW_KEY_D:
            	m_right = static_cast<bool>(action);
                break;
            case GLFW_KEY_E:
            	m_upward = static_cast<bool>(action);
                break;
            case GLFW_KEY_Q:
            	m_downward = static_cast<bool>(action);
                break;
            default:
                break;
        }
    }

    void PilotCameraController::scrollCallback(double offsetX, double offsetY, Camera &camera) {
        changeFov(offsetY, camera);
    }

    void PilotCameraController::mouseMoveCallback(double xoffset, double yoffset, Camera &camera) {
    	xoffset *= static_cast<float>(m_rotationActive);
    	yoffset *= static_cast<float>(m_rotationActive);

        panView(xoffset, yoffset, camera);
    }

    void PilotCameraController::mouseButtonCallback(int button, int action, int mods, Camera &camera) {
    	if (button == GLFW_MOUSE_BUTTON_2) {
    		if (m_rotationActive != (action == GLFW_PRESS)) {
    			m_rotationActive = (action == GLFW_PRESS);
    		}
    	}
    }

    void PilotCameraController::gamepadCallback(int gamepadIndex, Camera &camera, double frametime) {
        GLFWgamepadstate gamepadState;
        glfwGetGamepadState(gamepadIndex, &gamepadState);

        float sensitivity = 1.0f;
        double threshold = 0.1;

        // handle rotations
        double stickRightX = static_cast<double>(gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]);
        double stickRightY = static_cast<double>(gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);

        double rightXVal = glm::clamp(std::abs(stickRightX) - threshold, 0.0, 1.0)
                * copysign(1.0, stickRightX) * sensitivity * frametime;
        double rightYVal = glm::clamp(std::abs(stickRightY) - threshold, 0.0, 1.0)
                * copysign(1.0, stickRightY) * sensitivity * frametime;
        panView(rightXVal, rightYVal, camera);

        // handle zooming
        double zoom = static_cast<double>((gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]
                - gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER])
                * sensitivity * frametime);
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

}