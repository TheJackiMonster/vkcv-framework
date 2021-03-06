
#include "vkcv/camera/CameraManager.hpp"
#include <vkcv/Logger.hpp>

namespace vkcv::camera {

    CameraManager::CameraManager(Window& window)
    : m_window(window)
    {
        bindCameraToEvents();
        m_activeCameraIndex = 0;
        m_lastX = static_cast<float>(window.getWidth()) / 2.0f;
        m_lastY = static_cast<float>(window.getHeight()) / 2.0f;
        m_inputDelayTimer = glfwGetTime() + 0.2;
        m_frameTime = 0;
    }

    CameraManager::~CameraManager() {
    	m_window.e_key.remove(m_keyHandle);
		m_window.e_mouseMove.remove(m_mouseMoveHandle);
		m_window.e_mouseScroll.remove(m_mouseScrollHandle);
		m_window.e_mouseButton.remove(m_mouseButtonHandle);
		m_window.e_resize.remove(m_resizeHandle);
		m_window.e_gamepad.remove(m_gamepadHandle);
    }

    void CameraManager::bindCameraToEvents() {
        m_keyHandle = m_window.e_key.add( [&](int key, int scancode, int action, int mods) { this->keyCallback(key, scancode, action, mods); });
        m_mouseMoveHandle = m_window.e_mouseMove.add( [&]( double offsetX, double offsetY) {this->mouseMoveCallback( offsetX, offsetY);} );
        m_mouseScrollHandle =  m_window.e_mouseScroll.add([&](double offsetX, double offsetY) {this->scrollCallback( offsetX, offsetY);} );
        m_mouseButtonHandle = m_window.e_mouseButton.add([&] (int button, int action, int mods) {this->mouseButtonCallback( button,  action,  mods);});
        m_resizeHandle = m_window.e_resize.add([&](int width, int height) {this->resizeCallback(width, height);});
        m_gamepadHandle = m_window.e_gamepad.add([&](int gamepadIndex) {this->gamepadCallback(gamepadIndex);});
    }

    void CameraManager::resizeCallback(int width, int height) {
        if (glfwGetWindowAttrib(m_window.getWindow(), GLFW_ICONIFIED) == GLFW_FALSE) {
            for (size_t i = 0; i < m_cameras.size(); i++) {
                getCamera(i).setRatio(static_cast<float>(width) / static_cast<float>(height));;
            }
        }
    }

    void CameraManager::mouseButtonCallback(int button, int action, int mods){
        if(button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS){
            glfwSetInputMode(m_window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if(button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE){
            glfwSetInputMode(m_window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
		getActiveController().mouseButtonCallback(button, action, mods, getActiveCamera());
    }

    void CameraManager::mouseMoveCallback(double x, double y){
        auto xoffset = static_cast<float>(x - m_lastX) / m_window.getWidth();
		auto yoffset = static_cast<float>(y - m_lastY) / m_window.getHeight();
        m_lastX = x;
        m_lastY = y;
		getActiveController().mouseMoveCallback(xoffset, yoffset, getActiveCamera());
    }

    void CameraManager::scrollCallback(double offsetX, double offsetY) {
		getActiveController().scrollCallback(offsetX, offsetY, getActiveCamera());
    }

    void CameraManager::keyCallback(int key, int scancode, int action, int mods)  {
        switch (action) {
            case GLFW_RELEASE:
                switch (key) {
                    case GLFW_KEY_TAB:
                        if (m_activeCameraIndex + 1 == m_cameras.size()) {
                            m_activeCameraIndex = 0;
                        }
                        else {
                            m_activeCameraIndex++;
                        }
                        return;
                    case GLFW_KEY_ESCAPE:
                        glfwSetWindowShouldClose(m_window.getWindow(), 1);
                        return;
					default:
						break;
                }
            default:
				getActiveController().keyCallback(key, scancode, action, mods, getActiveCamera());
                break;
        }
    }

    void CameraManager::gamepadCallback(int gamepadIndex) {
        // handle camera switching
        GLFWgamepadstate gamepadState;
        glfwGetGamepadState(gamepadIndex, &gamepadState);

        double time = glfwGetTime();
        if (time - m_inputDelayTimer > 0.2) {
            int switchDirection = gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] - gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT];
            m_activeCameraIndex += switchDirection;
            if (std::greater<int>{}(m_activeCameraIndex, m_cameras.size() - 1)) {
                m_activeCameraIndex = 0;
            }
            else if (std::less<int>{}(m_activeCameraIndex, 0)) {
                m_activeCameraIndex = m_cameras.size() - 1;
            }
            uint32_t triggered = abs(switchDirection);
            m_inputDelayTimer = (1-triggered)*m_inputDelayTimer + triggered * time; // Only reset timer, if dpad was pressed - is this cheaper than if-clause?
        }

        getActiveController().gamepadCallback(gamepadIndex, getActiveCamera(), m_frameTime);     // handle camera rotation, translation
    }

    CameraController& CameraManager::getActiveController() {
    	const ControllerType type = getControllerType(getActiveCameraIndex());
    	return getControllerByType(type);
    }
	
	uint32_t CameraManager::addCamera(ControllerType controllerType) {
    	const float ratio = static_cast<float>(m_window.getWidth()) / static_cast<float>(m_window.getHeight());
    	
        Camera camera;
        camera.setPerspective(glm::radians(60.0f), ratio, 0.1f, 10.0f);
        return addCamera(controllerType, camera);
    }
    
    uint32_t CameraManager::addCamera(ControllerType controllerType, const Camera &camera) {
    	const uint32_t index = static_cast<uint32_t>(m_cameras.size());
    	m_cameras.push_back(camera);
		m_cameraControllerTypes.push_back(controllerType);
		return index;
    }

    Camera& CameraManager::getCamera(uint32_t cameraIndex) {
        if (cameraIndex < 0 || cameraIndex > m_cameras.size() - 1) {
        	vkcv_log(LogLevel::ERROR, "Invalid camera index: The index must range from 0 to %lu", m_cameras.size());
        	return getActiveCamera();
        }
        
        return m_cameras[cameraIndex];
    }

    Camera& CameraManager::getActiveCamera() {
        return m_cameras[getActiveCameraIndex()];
    }

    void CameraManager::setActiveCamera(uint32_t cameraIndex) {
        if (cameraIndex < 0 || cameraIndex > m_cameras.size() - 1) {
			vkcv_log(LogLevel::ERROR, "Invalid camera index: The index must range from 0 to %lu", m_cameras.size());
			return;
        }
        
        m_activeCameraIndex = cameraIndex;
    }

    uint32_t CameraManager::getActiveCameraIndex() const {
        return m_activeCameraIndex;
    }

    void CameraManager::setControllerType(uint32_t cameraIndex, ControllerType controllerType) {
        if (cameraIndex < 0 || cameraIndex > m_cameras.size() - 1) {
			vkcv_log(LogLevel::ERROR, "Invalid camera index: The index must range from 0 to %lu", m_cameras.size());
			return;
        }
        
        m_cameraControllerTypes[cameraIndex] = controllerType;
    }

    ControllerType CameraManager::getControllerType(uint32_t cameraIndex) {
        if (cameraIndex < 0 || cameraIndex > m_cameras.size() - 1) {
			vkcv_log(LogLevel::ERROR, "Invalid camera index: The index must range from 0 to %lu", m_cameras.size());
			return ControllerType::NONE;
        }
        
        return m_cameraControllerTypes[cameraIndex];
    }

    CameraController& CameraManager::getControllerByType(ControllerType controllerType) {
        switch(controllerType) {
            case ControllerType::PILOT:
                return m_pilotController;
            case ControllerType::TRACKBALL:
                return m_trackController;
            default:
                return m_pilotController;
        }
    }

    void CameraManager::update(double deltaTime) {
        m_frameTime = deltaTime;
        if (glfwGetWindowAttrib(m_window.getWindow(), GLFW_FOCUSED) == GLFW_TRUE) {
            getActiveController().updateCamera(deltaTime, getActiveCamera());
        }
	}
	
}
