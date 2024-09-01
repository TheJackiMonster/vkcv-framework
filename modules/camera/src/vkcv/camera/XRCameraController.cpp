
#include "vkcv/camera/XRCameraController.hpp"

namespace vkcv::camera {

  XRCameraController::XRCameraController() :
    m_instance(XR_NULL_HANDLE),
    m_system_id(XR_NULL_SYSTEM_ID),
    m_session(XR_NULL_HANDLE),
    m_space(XR_NULL_HANDLE)
  {
    XrInstanceCreateInfo instanceInfo {XR_TYPE_INSTANCE_CREATE_INFO};

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
    sessionInfo.systemId = m_system_id;

    if (XR_SUCCESS != xrCreateSession(m_instance, &sessionInfo, &m_session)) {
      vkcv::log(vkcv::LogLevel::ERROR, "Creating an XR session failed");
    }

    XrReferenceSpaceCreateInfo spaceInfo {XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
    spaceInfo.poseInReferenceSpace.orientation.w = 1.0f;
    spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;

    if (XR_SUCCESS != xrCreateReferenceSpace(m_session, &spaceInfo, &m_space)) {
      vkcv::log(vkcv::LogLevel::ERROR, "Creating an XR space failed");
    }
  }

  XRCameraController::~XRCameraController() {
    if (m_space) {
      xrDestroySpace(m_space);
    }

    if (m_session) {
      xrDestroySession(m_session);
    }

    if (m_instance) {
      xrDestroyInstance(m_instance);
    }
  }

  void XRCameraController::updateCamera(double deltaTime, Camera &camera) {
    std::vector<XrView> views;

    XrViewState viewState {XR_TYPE_VIEW_STATE};
    const uint32_t viewCapacity = views.size();
    uint32_t viewCount;

    XrViewLocateInfo viewInfo {XR_TYPE_VIEW_LOCATE_INFO};
    viewInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    viewInfo.space = m_space;

    XrResult result = xrLocateViews(
      m_session,
      &viewInfo,
      &viewState,
      &viewCount,
      views.data()
    );

    if ((XR_SUCCESS != result) || 
        ((viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) == 0) ||
        ((viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT) == 0)) {
      vkcv::log(vkcv::LogLevel::WARNING, "Unable to locate XR views");
      return;
    }

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
