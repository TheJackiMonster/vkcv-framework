
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
            for (auto& camera : m_cameras) {
				camera.setRatio(static_cast<float>(width) / static_cast<float>(height));;
            }
        }
    }

    void CameraManager::mouseButtonCallback(int button, int action, int mods) {
		const ControllerType type = getControllerType(getActiveCameraHandle());
		
        if ((button == GLFW_MOUSE_BUTTON_2) && (action == GLFW_PRESS)) {
            glfwSetInputMode(m_window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else
		if ((button == GLFW_MOUSE_BUTTON_2) && (action == GLFW_RELEASE)) {
            glfwSetInputMode(m_window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
	
		if (type == ControllerType::NONE) {
			return;
		}
		
		getControllerByType(type).mouseButtonCallback(button, action, mods, getActiveCamera());
    }

    void CameraManager::mouseMoveCallback(double x, double y) {
		const ControllerType type = getControllerType(getActiveCameraHandle());
		
        auto xoffset = static_cast<float>(x - m_lastX) / m_window.getWidth();
		auto yoffset = static_cast<float>(y - m_lastY) / m_window.getHeight();
        m_lastX = x;
        m_lastY = y;
	
		if (type == ControllerType::NONE) {
			return;
		}
		
		getControllerByType(type).mouseMoveCallback(xoffset, yoffset, getActiveCamera());
    }

    void CameraManager::scrollCallback(double offsetX, double offsetY) {
		const ControllerType type = getControllerType(getActiveCameraHandle());
	
		if (type == ControllerType::NONE) {
			return;
		}
		
		getControllerByType(type).scrollCallback(offsetX, offsetY, getActiveCamera());
    }

    void CameraManager::keyCallback(int key, int scancode, int action, int mods)  {
		const ControllerType type = getControllerType(getActiveCameraHandle());
		
        if (action == GLFW_RELEASE) {
			switch (key) {
				case GLFW_KEY_TAB:
					if (m_activeCameraIndex + 1 == m_cameras.size()) {
						m_activeCameraIndex = 0;
					} else {
						m_activeCameraIndex++;
					}
					return;
				case GLFW_KEY_ESCAPE:
					glfwSetWindowShouldClose(m_window.getWindow(), 1);
					return;
				default:
					break;
			}
        }
		
		if (type == ControllerType::NONE) {
			return;
		}
	
		getControllerByType(type).keyCallback(key, scancode, action, mods, getActiveCamera());
    }

    void CameraManager::gamepadCallback(int gamepadIndex) {
		const ControllerType type = getControllerType(getActiveCameraHandle());
		
        // handle camera switching
        GLFWgamepadstate gamepadState;
        glfwGetGamepadState(gamepadIndex, &gamepadState);

        double time = glfwGetTime();
        if (time - m_inputDelayTimer > 0.2) {
            int switchDirection = gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] - gamepadState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT];
            m_activeCameraIndex += switchDirection;
            
			if (std::greater<int>{}(m_activeCameraIndex, m_cameras.size() - 1)) {
                m_activeCameraIndex = 0;
            } else
			if (std::less<int>{}(m_activeCameraIndex, 0)) {
                m_activeCameraIndex = m_cameras.size() - 1;
            }
			
            uint32_t triggered = abs(switchDirection);
            m_inputDelayTimer = (1-triggered)*m_inputDelayTimer + triggered * time; // Only reset timer, if dpad was pressed - is this cheaper than if-clause?
        }
		
		if (type == ControllerType::NONE) {
			return;
		}
	
		getControllerByType(type).gamepadCallback(gamepadIndex, getActiveCamera(), m_frameTime);     // handle camera rotation, translation
    }
	
	CameraHandle CameraManager::addCamera(ControllerType controllerType) {
    	const float ratio = static_cast<float>(m_window.getWidth()) / static_cast<float>(m_window.getHeight());
    	
        Camera camera;
        camera.setPerspective(glm::radians(60.0f), ratio, 0.1f, 10.0f);
        return addCamera(controllerType, camera);
    }
	
	CameraHandle CameraManager::addCamera(ControllerType controllerType, const Camera &camera) {
    	const uint32_t index = static_cast<uint32_t>(m_cameras.size());
    	m_cameras.push_back(camera);
		m_cameraControllerTypes.push_back(controllerType);
		return CameraHandle(index);
    }

    Camera& CameraManager::getCamera(const CameraHandle& cameraHandle) {
        if (cameraHandle.getId() < 0 || cameraHandle.getId() >= m_cameras.size()) {
        	vkcv_log(LogLevel::ERROR, "Invalid camera index: The index must range from 0 to %lu", m_cameras.size());
        	return getActiveCamera();
        }
        
        return m_cameras[cameraHandle.getId()];
    }

    Camera& CameraManager::getActiveCamera() {
        return m_cameras[m_activeCameraIndex];
    }

    void CameraManager::setActiveCamera(const CameraHandle& cameraHandle) {
        if (cameraHandle.getId() < 0 || cameraHandle.getId() >= m_cameras.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid camera index: The index must range from 0 to %lu", m_cameras.size());
			return;
        }
        
        m_activeCameraIndex = cameraHandle.getId();
    }

    CameraHandle CameraManager::getActiveCameraHandle() const {
        return CameraHandle(m_activeCameraIndex);
    }

    void CameraManager::setControllerType(const CameraHandle& cameraHandle, ControllerType controllerType) {
        if (cameraHandle.getId() < 0 || cameraHandle.getId() >= m_cameras.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid camera index: The index must range from 0 to %lu", m_cameras.size());
			return;
        }
        
        m_cameraControllerTypes[cameraHandle.getId()] = controllerType;
    }

    ControllerType CameraManager::getControllerType(const CameraHandle& cameraHandle) {
        if (cameraHandle.getId() < 0 || cameraHandle.getId() >= m_cameras.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid camera index: The index must range from 0 to %lu", m_cameras.size());
			return ControllerType::NONE;
        }
        
        return m_cameraControllerTypes[cameraHandle.getId()];
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
		const ControllerType type = getControllerType(getActiveCameraHandle());
		
		if (type != ControllerType::NONE) {
			m_frameTime = deltaTime;
		} else {
			m_frameTime = 0.0;
			return;
		}
		
        if (glfwGetWindowAttrib(m_window.getWindow(), GLFW_FOCUSED) == GLFW_TRUE) {
			getControllerByType(type).updateCamera(m_frameTime, getActiveCamera());
        }
	}
	
}
