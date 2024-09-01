
#include "vkcv/camera/XRCameraController.hpp"

namespace vkcv::camera {

  XRCameraController::XRCameraController() :
    m_instance(XR_NULL_HANDLE),
    m_system_id(XR_NULL_SYSTEM_ID),
    m_session(XR_NULL_HANDLE)
  {
    XrInstanceCreateInfo instanceInfo {XR_TYPE_INSTANCE_CREATE_INFO};
    instanceInfo.next = nullptr;
    instanceInfo.enabledExtensionCount = 0;
    instanceInfo.enabledExtensionNames = nullptr;

    strcpy(instanceInfo.applicationInfo.applicationName, VKCV_FRAMEWORK_NAME);

    if (XR_SUCCESS != xrCreateInstance(&instanceInfo, &m_instance)) {
      vkcv::log(vkcv::LogLevel::ERROR, "Creating an XR instance failed");
    }

    XrSystemGetInfo systemInfo {XR_TYPE_SYSTEM_GET_INFO};
    systemInfo.formFactor = m_options->Parsed.FormFactor;
    
    if (XR_SUCCESS != xrGetSystem(m_instance, &systemInfo, &m_system_id)) {
      vkcv::log(vkcv::LogLevel::ERROR, "Getting an XR system id failed");
    }

    XrSessionCreateInfo sessionInfo {XR_TYPE_SESSION_CREATE_INFO};
    sessionInfo.next = nullptr;
    sessionInfo.systemId = m_system_id;

    if (XR_SUCCESS != xrCreateSession(m_instance, &sessionInfo, &m_session)) {
      vkcv::log(vkcv::LogLevel::ERROR, "Creating an XR session failed");
    }
  }

  XRCameraController::~XRCameraController() {
    if (m_session) {
      xrDestroySession(m_session);
    }

    if (m_instance) {
      xrDestroyInstance(m_instance);
    }
  }

  void XRCameraController::updateCamera(double deltaTime, Camera &camera) {
    // TODO
  }

  void XRCameraController::keyCallback(int key, int scancode, int action, int mods, Camera &camera) {
    // TODO
  }

  void XRCameraController::scrollCallback(double offsetX, double offsetY, Camera &camera) {
    // TODO
  }

  void XRCameraController::mouseMoveCallback(double xoffset, double yoffset, Camera &camera) {
    // TODO
  }

  void XRCameraController::mouseButtonCallback(int button, int action, int mods, Camera &camera) {
    // TODO
  }

  void XRCameraController::gamepadCallback(int gamepadIndex, Camera &camera, double frametime) {
    // TODO
  }

}
