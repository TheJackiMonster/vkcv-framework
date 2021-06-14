#include "vkcv/camera/CameraController.hpp"
#include <iostream>

namespace vkcv {
    
    void CameraController::updateCamera(double deltaTime, Camera &camera) {
    }

    void CameraController::keyCallback(int key, int scancode, int action, int mods, Camera &camera) {}

    void CameraController::scrollCallback( double offsetX, double offsetY, Camera &camera) {}

    void CameraController::mouseMoveCallback(double offsetX, double offsetY, Camera &camera) {}

    void CameraController::mouseButtonCallback(int button, int action, int mods, Camera &camera) {}

}