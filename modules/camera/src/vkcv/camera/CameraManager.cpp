#include <iostream>
#include "vkcv/camera/CameraManager.hpp"

namespace vkcv{

    CameraManager::CameraManager(Window &window, float width, float height)
    : m_window(window)
    {
        bindCameraToEvents();
        m_activeCameraIndex = 0;
        m_lastX = window.getWidth() / 2.0f;
        m_lastY = window.getHeight() / 2.0f;

    }

    CameraManager::~CameraManager() {}

    void CameraManager::bindCameraToEvents() {
        m_keyHandle = m_window.e_key.add( [&](int key, int scancode, int action, int mods) { this->keyCallback(key, scancode, action, mods); });
        m_mouseMoveHandle = m_window.e_mouseMove.add( [&]( double offsetX, double offsetY) {this->mouseMoveCallback( offsetX, offsetY);} );
        m_mouseScrollHandle =  m_window.e_mouseScroll.add([&](double offsetX, double offsetY) {this->scrollCallback( offsetX, offsetY);} );
        m_mouseButtonHandle = m_window.e_mouseButton.add([&] (int button, int action, int mods) {this->mouseButtonCallback( button,  action,  mods);});
        m_resizeHandle = m_window.e_resize.add([&](int width, int height) {this->resizeCallback(width, height);});
    }

    void CameraManager::resizeCallback(int width, int height) {
        for (size_t i = 0; i < m_cameras.size(); i++) {
            getCamera(i).setRatio(static_cast<float>(width) / static_cast<float>(height));;
        }
    }

    void CameraManager::mouseButtonCallback(int button, int action, int mods){
        if(button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS){
            glfwSetInputMode(m_window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if(button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE){
            glfwSetInputMode(m_window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        getControllerByType(getControllerType(getActiveCameraIndex())).mouseButtonCallback(button, action, mods, getActiveCamera());
    }

    void CameraManager::mouseMoveCallback(double x, double y){
        auto xoffset = static_cast<float>(x - m_lastX);
		auto yoffset = static_cast<float>(y - m_lastY);
        
        m_lastX = x;
        m_lastY = y;

        getControllerByType(getControllerType(getActiveCameraIndex())).mouseMoveCallback(xoffset, yoffset, getActiveCamera());
    }

    void CameraManager::scrollCallback(double offsetX, double offsetY) {

        getControllerByType(getControllerType(getActiveCameraIndex())).scrollCallback(offsetX, offsetY, getActiveCamera());
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
                }
            default:
                getControllerByType(getControllerType(getActiveCameraIndex())).keyCallback(key, scancode, action, mods, getActiveCamera());
        }
        
    }

    int CameraManager::addCamera() {
        Camera camera;
        m_cameras.push_back(camera);  // TODO: is there another way we can do this?
        m_cameras.back().setPerspective(glm::radians(60.0f), m_window.getWidth() / m_window.getHeight(), 0.1f, 10.0f);
        m_cameraControllerTypes.push_back(ControllerType::NONE);
        setActiveCamera(m_cameras.size() - 1);
        return m_cameras.size() - 1;
    }

    int CameraManager::addCamera(ControllerType controllerType) {
        Camera camera;
        m_cameras.push_back(camera);  // TODO: is there another way we can do this?
        m_cameras.back().setPerspective(glm::radians(60.0f), m_window.getWidth() / m_window.getHeight(), 0.1f, 10.0f);
        m_cameraControllerTypes.push_back(controllerType);
        return m_cameras.size() - 1;
    }

    Camera& CameraManager::getCamera(uint32_t cameraIndex) {
        if (cameraIndex < 0 || cameraIndex > m_cameras.size() - 1) {
            throw std::runtime_error("Invalid camera index.");
        }
        return m_cameras[cameraIndex];
    }

    Camera& CameraManager::getActiveCamera() {
        return m_cameras[getActiveCameraIndex()];
    }

    void CameraManager::setActiveCamera(uint32_t cameraIndex) {
        if (cameraIndex < 0 || cameraIndex > m_cameras.size() - 1) {
            throw std::runtime_error("Invalid camera index.");
        }
        m_activeCameraIndex = cameraIndex;
    }

    uint32_t CameraManager::getActiveCameraIndex() {
        return m_activeCameraIndex;
    }

    void CameraManager::setControllerType(uint32_t cameraIndex, ControllerType controllerType) {
        if (cameraIndex < 0 || cameraIndex > m_cameras.size() - 1) {
            throw std::runtime_error("Invalid camera index.");
        }
        m_cameraControllerTypes[cameraIndex] = controllerType;
    }

    ControllerType CameraManager::getControllerType(uint32_t cameraIndex) {
        if (cameraIndex < 0 || cameraIndex > m_cameras.size() - 1) {
            throw std::runtime_error("Invalid camera index.");
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
            getControllerByType(getControllerType(getActiveCameraIndex())).updateCamera(deltaTime, getActiveCamera());
        }
}