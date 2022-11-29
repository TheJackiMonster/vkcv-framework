
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
				+0.5f, -0.5f, -0.5f,
				-0.5f, -0.5f, +0.5f,
				+0.5f, -0.5f, +0.5f,
				-0.5f, +0.5f, -0.5f,
				+0.5f, +0.5f, -0.5f,
				-0.5f, +0.5f, +0.5f,
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
		
		std::vector<glm::vec3> cuboidTangents;
		cuboidTangents.resize(24, glm::vec3(0.0f));
		
		std::vector<size_t> cuboidTangentWeights;
		cuboidTangentWeights.resize(cuboidTangents.size(), 0);
		
		for (size_t i = 0; i < cuboidIndices.size(); i += 3) {
			const auto index0 = cuboidIndices[i + 0];
			const auto index1 = cuboidIndices[i + 1];
			const auto index2 = cuboidIndices[i + 2];
			
			const std::array<glm::vec3, 3> positions = {
					glm::vec3(
							cuboidPositions[index0 * 3 + 0],
							cuboidPositions[index0 * 3 + 1],
							cuboidPositions[index0 * 3 + 2]
					),
					glm::vec3(
							cuboidPositions[index1 * 3 + 0],
							cuboidPositions[index1 * 3 + 1],
							cuboidPositions[index1 * 3 + 2]
					),
					glm::vec3(
							cuboidPositions[index2 * 3 + 0],
							cuboidPositions[index2 * 3 + 1],
							cuboidPositions[index2 * 3 + 2]
					)
			};
			
			const std::array<glm::vec2, 3> uvs = {
					glm::vec2(
							cuboidUVCoords[index0 * 3 + 0],
							cuboidUVCoords[index0 * 3 + 1]
					),
					glm::vec2(
							cuboidUVCoords[index1 * 3 + 0],
							cuboidUVCoords[index1 * 3 + 1]
					),
					glm::vec2(
							cuboidUVCoords[index2 * 3 + 0],
							cuboidUVCoords[index2 * 3 + 1]
					)
			};
			
			const glm::vec3 tangent = generateTangent(positions, uvs);
			
			cuboidTangents[index0] += tangent;
			cuboidTangents[index1] += tangent;
			cuboidTangents[index2] += tangent;
			
			cuboidTangentWeights[index0]++;
			cuboidTangentWeights[index1]++;
			cuboidTangentWeights[index2]++;
		}
		
		for (size_t i = 0; i < cuboidTangents.size(); i++) {
			if (cuboidTangentWeights[i] <= 0) {
				continue;
			}
			
			cuboidTangents[i] /= cuboidTangentWeights[i];
		}
		
		const auto& position = getPosition();
		const auto& size = getSize();
		
		for (size_t i = 0; i < 24; i++) {
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
		
		auto tangentBuffer = buffer<glm::vec3>(core, BufferType::VERTEX, cuboidTangents.size());
		tangentBuffer.fill(cuboidTangents);
		
		VertexData data ({
			vkcv::vertexBufferBinding(positionBuffer.getHandle(), sizeof(float) * 3),
			vkcv::vertexBufferBinding(normalBuffer.getHandle(), sizeof(float) * 3),
			vkcv::vertexBufferBinding(uvBuffer.getHandle(), sizeof(float) * 2),
			vkcv::vertexBufferBinding(tangentBuffer)
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
			std::vector<uint16_t> cuboidIndices16;
			cuboidIndices16.resize(cuboidIndices.size());
			
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
