#pragma once
/**
 * @authors Tobias Frisch
 * @file include/vkcv/camera/CameraHandle.hpp
 * @brief CameraHandle class of the camera module for the vkcv framework.
 */

#include <vkcv/Handles.hpp>

namespace vkcv::camera {
	
	/**
     * @addtogroup vkcv_camera
     * @{
     */
	
	class CameraManager;
	
	/**
	 * @brief Handle class for cameras.
	 */
	class CameraHandle : public Handle {
		friend class CameraManager;
	
	private:
		using Handle::Handle;
	};
	
	/** @} */
	
}