#pragma once

#include "Geometry.hpp"

namespace vkcv::geometry {

	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	class Teapot : public Geometry {
	private:
		float m_scale;
		
	public:
		explicit Teapot(const glm::vec3& position, float scale);
		
		Teapot(const Teapot& other) = default;
		Teapot(Teapot&& other) = default;
		
		~Teapot() = default;
		
		Teapot& operator=(const Teapot& other) = default;
		Teapot& operator=(Teapot&& other) = default;
		
		[[nodiscard]]
		float getScale() const;
		
		void setScale(float scale);
		
		[[nodiscard]]
		VertexData generateVertexData(Core& core) const override;
		
	};
	
	/** @} */

}
