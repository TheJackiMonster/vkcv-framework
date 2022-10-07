#pragma once

#include "Volume.hpp"

namespace vkcv::geometry {
	
	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	class Cuboid : public Volume {
	private:
		glm::vec3 m_size;
		
	public:
		Cuboid(const glm::vec3& position, const glm::vec3& size);
		
		Cuboid(const glm::vec3& position, float size);
		
		Cuboid(const Cuboid& other) = default;
		Cuboid(Cuboid&& other) = default;
		
		~Cuboid() = default;
		
		Cuboid& operator=(const Cuboid& other) = default;
		Cuboid& operator=(Cuboid&& other) = default;
		
		[[nodiscard]]
		const glm::vec3& getSize() const;
		
		void setSize(const glm::vec3& size);
		
		[[nodiscard]]
		float distanceTo(const glm::vec3& point) override;
		
		[[nodiscard]]
		VertexData generateVertexData(Core& core) const override;
		
	};
	
	/** @} */
	
}
