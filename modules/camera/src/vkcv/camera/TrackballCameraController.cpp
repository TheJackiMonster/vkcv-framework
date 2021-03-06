#include "vkcv/camera/TrackballCameraController.hpp"
#include <GLFW/glfw3.h>

namespace vkcv::camera {

    TrackballCameraController::TrackballCameraController() {
        m_rotationActive = false;
        m_radius = 3.0f;
        m_cameraSpeed = 2.5f;
        m_scrollSensitivity = 0.2f;
        m_pitch = 0.0f;
        m_yaw = 0.0f;
    }

    void TrackballCameraController::setRadius(const float radius) {
        m_radius = 0.1f * (radius < 0.1f) + radius * (1 - (radius < 0.1f));
    }

    void TrackballCameraController::panView(double xOffset, double yOffset, Camera &camera) {
        // update only if there is (valid) input
        if (xOffset == 0.0 && yOffset == 0.0) {
            return;
        }

        // handle yaw rotation
        m_yaw = m_yaw + static_cast<float>(xOffset) * 90.0f * m_cameraSpeed;

        // handle pitch rotation
        m_pitch = m_pitch + static_cast<float>(yOffset) * 90.0f * m_cameraSpeed;
    }

    void TrackballCameraController::updateRadius(double offset, Camera &camera) {
        // update only if there is (valid) input
        if (offset == 0.0) {
            return;
        }

        glm::vec3 cameraPosition = camera.getPosition();
        glm::vec3 cameraCenter = camera.getCenter();
        float radius = glm::length(cameraCenter - cameraPosition);  // get current camera radius
        setRadius(radius - static_cast<float>(offset) * m_scrollSensitivity);
    }

    void TrackballCameraController::updateCamera(double deltaTime, Camera &camera) {
		const glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		const glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
	
		const glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(m_yaw), yAxis);
		const glm::mat4 rotationX = glm::rotate(rotationY, -glm::radians(m_pitch), xAxis);
		const glm::vec3 translation = glm::vec3(
				rotationX * glm::vec4(0.0f, 0.0f, m_radius, 0.0f)
		);
		
		const glm::vec3 center = camera.getCenter();
		const glm::vec3 position = center + translation;
		const glm::vec3 up = glm::vec3(
				rotationX * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)
		);
		
		camera.lookAt(position, center, up);
    }

    void TrackballCameraController::keyCallback(int key, int scancode, int action, int mods, Camera &camera) {}

    void TrackballCameraController::scrollCallback(double offsetX, double offsetY, Camera &camera) {
        updateRadius(offsetY, camera);
    }

    void TrackballCameraController::mouseMoveCallback(double xoffset, double yoffset, Camera &camera) {
        xoffset *= static_cast<float>(m_rotationActive);
        yoffset *= static_cast<float>(m_rotationActive);

        panView(xoffset, yoffset, camera);
    }

    void TrackballCameraController::mouseButtonCallback(int button, int action, int mods, Camera &camera) {
        if(button == GLFW_MOUSE_BUTTON_2 && m_rotationActive == false && action == GLFW_PRESS){
            m_rotationActive = true;
        }
        else if(button == GLFW_MOUSE_BUTTON_2 && m_rotationActive == true && action == GLFW_RELEASE){
            m_rotationActive = false;
        }
    }

    void TrackballCameraController::gamepadCallback(int gamepadIndex, Camera &camera, double frametime) {
        GLFWgamepadstate gamepadState;
        glfwGetGamepadState(gamepadIndex, &gamepadState);

        float sensitivity = 1.0f;
        double threshold = 0.1;

        // handle rotations
        double stickRightX = static_cast<double>(gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]);
        double stickRightY = static_cast<double>(gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
        
        double rightXVal = glm::clamp((abs(stickRightX)-threshold), 0.0, 1.0)
                * std::copysign(1.0, stickRightX) * sensitivity * frametime;
        double rightYVal = glm::clamp((abs(stickRightY)-threshold), 0.0, 1.0)
                * std::copysign(1.0, stickRightY) * sensitivity * frametime;
        panView(rightXVal, rightYVal, camera);

        // handle translation
        double stickLeftY = static_cast<double>(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
        double leftYVal = glm::clamp((abs(stickLeftY)-threshold), 0.0, 1.0)
                * std::copysign(1.0, stickLeftY) * sensitivity * frametime;
        updateRadius(-leftYVal, camera);
    }
}