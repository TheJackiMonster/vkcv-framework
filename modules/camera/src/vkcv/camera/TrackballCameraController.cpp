#include "vkcv/camera/TrackballCameraController.hpp"
#include <GLFW/glfw3.h>

namespace vkcv::camera {

    TrackballCameraController::TrackballCameraController() {
        m_rotationActive = false;
        m_radius = 3.0f;
        m_cameraSpeed = 2.5f;
        m_scrollSensitivity = 0.2f;
    }

    void TrackballCameraController::setRadius(const float radius) {
        if (radius < 0.1f) {
            m_radius = 0.1f;
        }
        else {
            m_radius = radius;
        }
    }

    void TrackballCameraController::panView(double xOffset, double yOffset, Camera &camera) {
        // handle yaw rotation
        float yaw = camera.getYaw() + xOffset * m_cameraSpeed;
        if (yaw < 0.0f) {
            yaw += 360.0f;
        }
        else if (yaw > 360.0f) {
            yaw -= 360.0f;
        }
        camera.setYaw(yaw);

        // handle pitch rotation
        float pitch = camera.getPitch() + yOffset * m_cameraSpeed;
        if (pitch < 0.0f) {
            pitch += 360.0f;
        }
        else if (pitch > 360.0f) {
            pitch -= 360.0f;
        }
        camera.setPitch(pitch);
    }

    void TrackballCameraController::updateRadius(double offset, Camera &camera) {
        glm::vec3 cameraPosition = camera.getPosition();
        glm::vec3 cameraCenter = camera.getCenter();
        float radius = glm::length(cameraCenter - cameraPosition);  // get current camera radius
        setRadius(radius - offset * m_scrollSensitivity);
    }

    void TrackballCameraController::updateCamera(double deltaTime, Camera &camera) {
		float yaw = camera.getYaw();
		float pitch = camera.getPitch();
		
		const glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		const glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
	
		const glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), yAxis);
		const glm::mat4 rotationX = glm::rotate(rotationY, -glm::radians(pitch), xAxis);
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
        if(!m_rotationActive){
            return;
        }

        float sensitivity = 0.025f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        panView(xoffset , yoffset, camera);
    }

    void TrackballCameraController::mouseButtonCallback(int button, int action, int mods, Camera &camera) {
        if(button == GLFW_MOUSE_BUTTON_2 && m_rotationActive == false && action == GLFW_PRESS){
            m_rotationActive = true;
        }
        else if(button == GLFW_MOUSE_BUTTON_2 && m_rotationActive == true && action == GLFW_RELEASE){
            m_rotationActive = false;
        }
    }

    void TrackballCameraController::gamepadCallback(int gamepadIndex, Camera &camera) {
        GLFWgamepadstate gamepadState;
        glfwGetGamepadState(gamepadIndex, &gamepadState);

        float sensitivity = 0.025f;
        double threshold = 0.03;    // todo: needs further investigation!

        // handle rotations
        double stickRightX = static_cast<double>(gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]);
        double stickRightY = static_cast<double>(gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
        if ((std::less<double>{}(stickRightX, -threshold) || std::greater<double>{}(stickRightX, threshold))
            && (std::less<double>{}(stickRightY, -threshold) || std::greater<double>{}(stickRightY, threshold))) {
            panView(stickRightX * sensitivity, stickRightY * sensitivity, camera);
        }

        // handle translation
        double stickLeftY = static_cast<double>(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
        if (std::less<double>{}(stickLeftY, -threshold) || std::greater<double>{}(stickLeftY, threshold)) {
            updateRadius(-stickLeftY * sensitivity, camera);
        }
    }
}