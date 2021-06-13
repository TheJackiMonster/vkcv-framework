#include <iostream>
#include "vkcv/camera/CameraManager.hpp"

namespace vkcv{

    CameraManager::CameraManager(Window &window, float width, float height)
    : m_window(window)
    {
        bindCameraToEvents();
    }

    CameraManager::~CameraManager() {
        // free memory of allocated pointers (specified with 'new')
        for (auto controller : m_controllers) {
            delete controller;
        }

        for (auto camera : m_cameras) {
            delete camera;
        }
    }

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
        m_controllers[m_activeControllerIndex]->mouseButtonCallback(button, action, mods);
    }

    void CameraManager::mouseMoveCallback(double x, double y){
        m_controllers[m_activeControllerIndex]->mouseMoveCallback(x, y);
    }

    void CameraManager::scrollCallback(double offsetX, double offsetY) {
        m_controllers[m_activeControllerIndex]->scrollCallback(offsetX, offsetY);
    }

    void CameraManager::keyCallback(int key, int scancode, int action, int mods)  {
        switch (action) {
            case GLFW_RELEASE:
                switch (key) {
                    case GLFW_KEY_TAB:
                        if (m_activeControllerIndex + 1 == m_controllers.size()) {
                            m_activeControllerIndex = 0;
                        }
                        else {
                            m_activeControllerIndex++;
                        }
                        return;
                    case GLFW_KEY_ESCAPE:
                        glfwSetWindowShouldClose(m_window.getWindow(), 1);
                        return;
                }
            default:
                m_controllers[m_activeControllerIndex]->keyCallback(key, scancode, action, mods);
        }
    }

    int CameraManager::addCamera() {
        m_cameras.push_back(new Camera());  // TODO: is there another way we can do this?
        m_cameras.back()->setPerspective(glm::radians(60.0f), m_window.getWidth() / m_window.getHeight(), 0.1f, 10.0f);
        return m_cameras.size() - 1;
    }

    Camera& CameraManager::getCamera(uint32_t cameraIndex) {
        return *m_cameras[cameraIndex];
    }


    int CameraManager::addController(ControllerType controllerType, uint32_t cameraIndex) {
        switch(controllerType) {
            case ControllerType::PILOT: {
                m_controllers.push_back(new PilotCameraController());   // TODO: is there another way we can do this?
                break;
            }
            case ControllerType::TRACKBALL: {
                m_controllers.push_back(new TrackballCameraController());
                break;
            }
            case ControllerType::TRACE: {
                // TODO: implement (Josch)
            }
        }

        m_controllers.back()->setWindow(m_window);

        int controllerIndex = m_controllers.size() - 1;
        bindCameraToController(cameraIndex, controllerIndex);

        if (controllerIndex == 0) {
            setActiveController(0);
        }

        return controllerIndex;
    }

    CameraController& CameraManager::getController(uint32_t controllerIndex) {
        return *m_controllers[controllerIndex];
    }

    void CameraManager::bindCameraToController(uint32_t cameraIndex, uint32_t controllerIndex) {
        m_controllers[controllerIndex]->setCamera(*m_cameras[cameraIndex]);
    }

    CameraController& CameraManager::getActiveController() {
        return *m_controllers[m_activeControllerIndex];
    }

    void CameraManager::setActiveController(uint32_t controllerIndex) {
        m_activeControllerIndex = controllerIndex;
    }

    void CameraManager::update(double deltaTime) {
        m_controllers[m_activeControllerIndex]->updateCamera(deltaTime);
    }

    void CameraManager::update(double deltaTime, uint32_t controllerIndex) {
        m_controllers[controllerIndex]->updateCamera(deltaTime);
    }
}