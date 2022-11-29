
#include "vkcv/geometry/Geometry.hpp"

namespace vkcv::geometry {
	
	Geometry::Geometry(const glm::vec3 &position)
	: m_position(position) {}
	
	const glm::vec3 &Geometry::getPosition() const {
		return m_position;
	}
	
	void Geometry::setPosition(const glm::vec3 &position) {
		m_position = position;
	}
	
	glm::vec3 Geometry::generateTangent(const std::array<glm::vec3, 3>& positions,
										const  std::array<glm::vec2, 3>& uvs) const {
		auto delta1 = positions[1] - positions[0];
		auto delta2 = positions[2] - positions[0];
		
		auto deltaUV1 = uvs[1] - uvs[0];
		auto deltaUV2 = uvs[2] - uvs[0];
		
		return glm::normalize(glm::vec3(
				delta1 * deltaUV2.y -
				delta2 * deltaUV1.y
		));
	}
	
	GeometryData Geometry::extractGeometryData(Core& core,
											   const vkcv::VertexData &vertexData) const {
		const VertexBufferBinding positionBufferBinding = vertexData.getVertexBufferBindings()[0];
		const size_t bufferSize = core.getBufferSize(positionBufferBinding.m_buffer);
		
		if (positionBufferBinding.m_stride < sizeof(float) * 3) {
			return {};
		}
		
		const size_t vertexCount = (bufferSize / positionBufferBinding.m_stride);
		
		if (vertexCount < 3) {
			return {};
		}
		
		GeometryData data (
				positionBufferBinding,
				vertexCount - 1,
				GeometryVertexType::POSITION_FLOAT3
		);
		
		data.setIndexBuffer(vertexData.getIndexBuffer(), vertexData.getIndexBitCount());
		data.setCount(vertexData.getCount());
		
		return data;
	}
	
}
