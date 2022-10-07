
#include "vkcv/geometry/Cuboid.hpp"

namespace vkcv::geometry {
	
	Cuboid::Cuboid(const glm::vec3 &position, const glm::vec3 &size)
	: Volume(position), m_size(size) {}
	
	Cuboid::Cuboid(const glm::vec3 &position, float size)
	: Cuboid(position, glm::vec3(size)) {}
	
	const glm::vec3 &Cuboid::getSize() const {
		return m_size;
	}
	
	void Cuboid::setSize(const glm::vec3 &size) {
		m_size = size;
	}
	
	float Cuboid::distanceTo(const glm::vec3 &point) {
		const auto offset = (point - getPosition());
		const auto distance = (glm::abs(offset) - getSize() * 0.5f);
		const auto inside = glm::lessThanEqual(distance, glm::vec3(0.0f));
		
		if (glm::all(inside)) {
			return glm::max(glm::max(distance.x, distance.y), distance.z);
		} else {
			return glm::length(glm::vec3(glm::not_(inside)) * distance);
		}
	}
	
	VertexData Cuboid::generateVertexData(vkcv::Core &core) const {
		std::array<float, 72> cuboidPositions = {
				-0.5f, -0.5f, -0.5f,
				-0.5f, -0.5f, +0.5f,
				-0.5f, +0.5f, -0.5f,
				-0.5f, +0.5f, +0.5f,
				+0.5f, -0.5f, -0.5f,
				+0.5f, -0.5f, +0.5f,
				+0.5f, +0.5f, -0.5f,
				+0.5f, +0.5f, +0.5f,
				
				-0.5f, -0.5f, -0.5f,
				-0.5f, -0.5f, +0.5f,
				+0.5f, -0.5f, -0.5f,
				+0.5f, -0.5f, +0.5f,
				-0.5f, +0.5f, -0.5f,
				-0.5f, +0.5f, +0.5f,
				+0.5f, +0.5f, -0.5f,
				+0.5f, +0.5f, +0.5f,
				
				-0.5f, -0.5f, -0.5f,
				-0.5f, +0.5f, -0.5f,
				+0.5f, -0.5f, -0.5f,
				+0.5f, +0.5f, -0.5f,
				-0.5f, -0.5f, +0.5f,
				-0.5f, +0.5f, +0.5f,
				+0.5f, -0.5f, +0.5f,
				+0.5f, +0.5f, +0.5f
		};
		
		const std::array<float, 72> cuboidNormals = {
				-1.0f, 0.0f, 0.0f,
				-1.0f, 0.0f, 0.0f,
				-1.0f, 0.0f, 0.0f,
				-1.0f, 0.0f, 0.0f,
				+1.0f, 0.0f, 0.0f,
				+1.0f, 0.0f, 0.0f,
				+1.0f, 0.0f, 0.0f,
				+1.0f, 0.0f, 0.0f,
				
				0.0f, -1.0f, 0.0f,
				0.0f, -1.0f, 0.0f,
				0.0f, -1.0f, 0.0f,
				0.0f, -1.0f, 0.0f,
				0.0f, +1.0f, 0.0f,
				0.0f, +1.0f, 0.0f,
				0.0f, +1.0f, 0.0f,
				0.0f, +1.0f, 0.0f,
				
				0.0f, 0.0f, -1.0f,
				0.0f, 0.0f, -1.0f,
				0.0f, 0.0f, -1.0f,
				0.0f, 0.0f, -1.0f,
				0.0f, 0.0f, +1.0f,
				0.0f, 0.0f, +1.0f,
				0.0f, 0.0f, +1.0f,
				0.0f, 0.0f, +1.0f
		};
		
		const std::array<float, 48> cuboidUVCoords = {
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
		};
		
		const std::array<uint8_t, 36> cuboidIndices = {
				0, 1, 3,
				0, 3, 2,
				4, 6, 7,
				4, 7, 5,
				
				8, 9, 11,
				8, 11, 10,
				12, 14, 15,
				12, 15, 13,
				
				16, 17, 19,
				16, 19, 18,
				20, 22, 23,
				20, 23, 21
		};
		
		const auto& position = getPosition();
		const auto& size = getSize();
		
		for (size_t i = 0; i < 8; i++) {
			cuboidPositions[i * 3 + 0] = cuboidPositions[i * 3 + 0] * size.x + position.x;
			cuboidPositions[i * 3 + 1] = cuboidPositions[i * 3 + 1] * size.y + position.y;
			cuboidPositions[i * 3 + 2] = cuboidPositions[i * 3 + 2] * size.z + position.z;
		}
		
		auto positionBuffer = buffer<float>(core, BufferType::VERTEX, cuboidPositions.size());
		positionBuffer.fill(cuboidPositions);
		
		auto normalBuffer = buffer<float>(core, BufferType::VERTEX, cuboidNormals.size());
		normalBuffer.fill(cuboidNormals);
		
		auto uvBuffer = buffer<float>(core, BufferType::VERTEX, cuboidUVCoords.size());
		uvBuffer.fill(cuboidUVCoords);
		
		VertexData data ({
			vkcv::vertexBufferBinding(positionBuffer.getHandle()),
			vkcv::vertexBufferBinding(normalBuffer.getHandle()),
			vkcv::vertexBufferBinding(uvBuffer.getHandle())
		});
		
		const auto& featureManager = core.getContext().getFeatureManager();
		
		const bool index8Bit = featureManager.checkFeatures<vk::PhysicalDeviceIndexTypeUint8FeaturesEXT>(
				vk::StructureType::ePhysicalDeviceIndexTypeUint8FeaturesEXT,
				[](const vk::PhysicalDeviceIndexTypeUint8FeaturesEXT& features) {
					return features.indexTypeUint8;
				}
		);
		
		if (index8Bit) {
			auto indexBuffer = buffer<uint8_t>(core, BufferType::INDEX, cuboidIndices.size());
			indexBuffer.fill(cuboidIndices);
			
			data.setIndexBuffer(indexBuffer.getHandle(), IndexBitCount::Bit8);
			data.setCount(indexBuffer.getCount());
		} else {
			std::array<uint16_t, cuboidIndices.size()> cuboidIndices16;
			
			for (size_t i = 0; i < cuboidIndices16.size(); i++) {
				cuboidIndices16[i] = static_cast<uint16_t>(cuboidIndices[i]);
			}
			
			auto indexBuffer = buffer<uint16_t>(core, BufferType::INDEX, cuboidIndices16.size());
			indexBuffer.fill(cuboidIndices16);
			
			data.setIndexBuffer(indexBuffer.getHandle());
			data.setCount(indexBuffer.getCount());
		}
		
		return data;
	}
	
}
