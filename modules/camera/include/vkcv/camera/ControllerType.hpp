#pragma once
/**
 * @authors Tobias Frisch
 * @file include/vkcv/camera/ControllerType.hpp
 * @brief ControllerType enum of the camera module for the vkcv framework.
 */

namespace vkcv::camera {
	
	/**
     * @addtogroup vkcv_camera
     * @{
     */
	
	/**
     * @brief Used for specifying existing types of camera controllers when adding a new controller object to the
     * #CameraManager.
     */
	enum class ControllerType {
		NONE,
		PILOT,
		TRACKBALL
	};
	
	/** @} */
	
}
