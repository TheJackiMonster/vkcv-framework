
#include "vkcv/geometry/Volume.hpp"

namespace vkcv::geometry {
	
	Volume::Volume(const glm::vec3 &position)
	: Geometry(position) {}
	
	bool Volume::contains(const glm::vec3 &point) {
		return (this->distanceTo(point) <= 0.0f);
	}

}
