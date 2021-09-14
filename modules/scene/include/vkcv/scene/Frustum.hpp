#pragma once

#include <glm/mat4x4.hpp>
#include "vkcv/scene/Bounds.hpp"

namespace vkcv::scene {

    /**
     * @addtogroup vkcv_scene
     * @{
     */
	
	Bounds transformBounds(const glm::mat4& transform, const Bounds& bounds, bool* negative_w = nullptr);
	
	bool checkFrustum(const glm::mat4& transform, const Bounds& bounds);

    /** @} */
	
}
