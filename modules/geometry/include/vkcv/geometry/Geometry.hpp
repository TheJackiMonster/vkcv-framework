#pragma once

#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <vkcv/VertexData.hpp>

#include <glm/glm.hpp>

namespace vkcv::geometry {
	
	/**
     * @defgroup vkcv_geometry Geometry Module
     * A module to use basic geometry for rendering.
     * @{
     */
	
	class Geometry {
	private:
		glm::vec3 m_position;
		
	public:
		explicit Geometry(const glm::vec3& position);
		
		Geometry(const Geometry& other) = default;
		Geometry(Geometry&& other) = default;
		
		~Geometry() = default;
		
		Geometry& operator=(const Geometry& other) = default;
		Geometry& operator=(Geometry&& other) = default;
		
		[[nodiscard]]
		const glm::vec3& getPosition() const;
		
		void setPosition(const glm::vec3& position);
		
		[[nodiscard]]
		virtual VertexData generateVertexData(Core& core) const = 0;
		
	};
	
	/** @} */

}
