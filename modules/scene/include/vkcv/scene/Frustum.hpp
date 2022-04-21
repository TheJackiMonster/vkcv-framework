#pragma once

#include <glm/mat4x4.hpp>
#include "vkcv/scene/Bounds.hpp"

namespace vkcv::scene {

    /**
     * @addtogroup vkcv_scene
     * @{
     */
	
	/**
     * Transform a given axis aligned bounding box into frustum coordinates
     * using a given 4x4 transformation matrix to return a new axis aligned
     * bounding box containing that transformed box.
     * @param[in] transform Frustum defining 4x4 matrix
     * @param[in] bounds Axis aligned bounding box
     * @return Bounds containing box in frustum coordinates
     */
	Bounds transformBounds(const glm::mat4& transform, const Bounds& bounds);

    /**
     * Transform a given axis aligned bounding box into frustum coordinates
     * using a given 4x4 transformation matrix to return a new axis aligned
     * bounding box containing that transformed box.
     * @param[in] transform Frustum defining 4x4 matrix
     * @param[in] bounds Axis aligned bounding box
     * @param[out] negative_w Flag if w-coordinate is negative
     * @return Bounds containing box in frustum coordinates
     */
	Bounds transformBounds(const glm::mat4& transform, const Bounds& bounds, bool& negative_w);

    /**
     * Check if an axis aligned bounding box is intersecting a given frustum
     * defined by a 4x4 transformation matrix.
     * @param[in] transform Frustum defining 4x4 matrix
     * @param[in] bounds Axis aligned bounding box
     * @return true if the box and frustum are intersecting, otherwise false
     */
	bool checkFrustum(const glm::mat4& transform, const Bounds& bounds);

    /** @} */
	
}
