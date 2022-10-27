#include "vkcv/camera/TrackballCameraController.hpp"
#include <GLFW/glfw3.h>

namespace vkcv::camera {

    TrackballCameraController::TrackballCameraController() {
        m_rotationActive = false;
        m_cameraSpeed = 2.5f;
        m_scrollSensitivity = 0.2f;
		m_pitch = 0.0f;
		m_yaw = 0.0f;
    }

    void TrackballCameraController::panView(double xOffset, double yOffset, Camera &camera) {
		// update only if there is (valid) input
		if (xOffset == 0.0 && yOffset == 0.0) {
			return;
		}
	
		m_yaw += static_cast<float>(xOffset) * 90.0f * m_cameraSpeed;
		m_pitch += static_cast<float>(yOffset) * 90.0f * m_cameraSpeed;
	}

    void TrackballCameraController::updateRadius(double offset, Camera &camera) {
        // update only if there is (valid) input
        if (offset == 0.0) {
            return;
        }
		
		camera.setPosition(
				camera.getPosition() +
				camera.getFront() * static_cast<float>(offset)
		);
    }

    void TrackballCameraController::updateCamera(double deltaTime, Camera &camera) {
		const auto center = camera.getCenter();
		const auto distance = center - camera.getPosition();
		const float radius = glm::length(distance);
	
		glm::vec3 front = distance / radius;
		glm::vec3 up = camera.getUp();
		glm::vec3 left;
	
		const auto rotationY = glm::rotate(
				glm::identity<glm::mat4>(),
				glm::radians(m_yaw),
				up
		);
	
		front = glm::vec3(rotationY * glm::vec4(front, 0.0f));
		left = glm::normalize(glm::cross(up, front));
	
		const auto rotationX = glm::rotate(
				rotationY,
				glm::radians(m_pitch),
				left
		);
	
		up = glm::vec3(rotationX * glm::vec4(up, 0.0f));
		front = glm::normalize(glm::cross(up, left));
		
		m_yaw = 0.0f;
		m_pitch = 0.0f;
	
		camera.lookAt(center + front * radius, center, up);
	}

    void TrackballCameraController::keyCallback(int key, int scancode, int action, int mods, Camera &camera) {}

    void TrackballCameraController::scrollCallback(double offsetX, double offsetY, Camera &camera) {
        updateRadius(offsetY * m_scrollSensitivity, camera);
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
        auto stickRightX = static_cast<double>(gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]);
        auto stickRightY = static_cast<double>(gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
        
        double rightXVal = glm::clamp((glm::abs(stickRightX)-threshold), 0.0, 1.0)
                * glm::sign(stickRightX) * sensitivity * frametime;
        double rightYVal = glm::clamp((glm::abs(stickRightY)-threshold), 0.0, 1.0)
                * glm::sign(stickRightY) * sensitivity * frametime;
        panView(rightXVal, rightYVal, camera);

        // handle translation
        auto stickLeftY = static_cast<double>(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
        double leftYVal = glm::clamp((glm::abs(stickLeftY)-threshold), 0.0, 1.0)
                * glm::sign(stickLeftY) * sensitivity * frametime;
        updateRadius(-leftYVal, camera);
    }
}