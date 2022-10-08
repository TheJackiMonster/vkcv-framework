
#include "vkcv/scene/Frustum.hpp"

namespace vkcv::scene {
	
	static glm::vec3 transformPoint(const glm::mat4& transform, const glm::vec3& point, bool& negative_w) {
		const glm::vec4 position = transform * glm::vec4(point, 1.0f);
		
		/*
		 * We divide by the absolute of the 4th coorditnate because
		 * clipping is weird and points have to move to the other
		 * side of the camera.
		 *
		 * We also need to collect if the 4th coordinate was negative
		 * to know if all corners are behind the camera. So these can
		 * be culled as well
		 */
		const float perspective = std::abs(position[3]);
		negative_w &= (position[3] < 0.0f);
		return glm::vec3(
				position[0] / perspective,
				position[1] / perspective,
				position[2] / perspective
		);
	}
	
	Bounds transformBounds(const glm::mat4& transform, const Bounds& bounds) {
		const auto corners = bounds.getCorners();
		
		Bounds result (glm::vec3(transform * glm::vec4(corners[0], 1.0f)));
		
		for (size_t j = 1; j < corners.size(); j++) {
			result.extend(glm::vec3(transform * glm::vec4(corners[j], 1.0f)));
		}
		
		return result;
	}
	
	Bounds transformBounds(const glm::mat4& transform, const Bounds& bounds, bool& negative_w) {
		const auto corners = bounds.getCorners();
		negative_w = true;
		
		Bounds result (transformPoint(transform, corners[0], negative_w));
		
		for (size_t j = 1; j < corners.size(); j++) {
			result.extend(transformPoint(transform, corners[j], negative_w));
		}
		
		return result;
	}
	
	bool checkFrustum(const glm::mat4& transform, const Bounds& bounds) {
		static Bounds frustum (
				glm::vec3(-1.0f, -1.0f, -0.0f),
				glm::vec3(+1.0f, +1.0f, +1.0f)
		);
		
		bool negative_w;
		auto box = transformBounds(transform, bounds, negative_w);
		
		if (negative_w) {
			return false;
		} else {
			return box.intersects(frustum);
		}
	}

}
