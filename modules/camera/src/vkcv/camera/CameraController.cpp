#include "vkcv/camera/CameraController.hpp"

namespace vkcv {

    Camera& CameraController::getCamera() {
        return *m_camera;
    }

    void CameraController::setCamera(Camera &camera) {
        m_camera = &camera;
    }

    void CameraController::setWindow(Window &window) {
        m_window = &window;
        m_lastX = m_window->getWidth() / 2.0;
        m_lastY = m_window->getHeight() / 2.0;
    }

    void CameraController::updateCamera(double deltaTime) {}

    void CameraController::keyCallback(int key, int scancode, int action, int mods) {}

    void CameraController::scrollCallback( double offsetX, double offsetY) {}

    void CameraController::mouseMoveCallback(double offsetX, double offsetY) {}

    void CameraController::mouseButtonCallback(int button, int action, int mods) {}

}