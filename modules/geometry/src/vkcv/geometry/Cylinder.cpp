
#include "vkcv/geometry/Cylinder.hpp"

namespace vkcv::geometry {
	
	Cylinder::Cylinder(const glm::vec3 &position, float height, float radius)
	: Volume(position), Circular(radius), m_height(height) {}
	
	float Cylinder::getHeight() const {
		return m_height;
	}
	
	void Cylinder::setHeight(float height) {
		m_height = height;
	}
	
	float Cylinder::distanceTo(const glm::vec3 &point) {
		const auto& position = getPosition();
		
		const auto verticalDistance = glm::abs(position.y - point.y) - getHeight();
		const auto circularDistance = glm::distance(
				glm::vec2(position.x, position.z),
				glm::vec2(point.x, point.z)
		) - getRadius();
		
		if (circularDistance <= 0.0f) {
			return glm::max(verticalDistance, circularDistance);
		} else
		if (verticalDistance <= 0.0f) {
			return circularDistance;
		} else {
			return glm::length(glm::vec2(verticalDistance, circularDistance));
		}
	}
	
	VertexData Cylinder::generateVertexData(vkcv::Core &core) const {
		const auto& position = getPosition();
		const auto radius = getRadius();
		const auto height = getHeight();
		const auto resolution = getResolution();
		
		const size_t vertexCount = (resolution * 4 + 2);
		
		std::vector<glm::vec3> cylinderVertices;
		std::vector<glm::vec3> cylinderNormals;
		std::vector<glm::vec2> cylinderUVCoords;
		
		cylinderVertices.reserve(vertexCount);
		cylinderNormals.reserve(vertexCount);
		cylinderUVCoords.reserve(vertexCount);
		
		std::vector<uint32_t> cylinderIndices;
		cylinderIndices.reserve(resolution * 12);
		
		size_t i, j;
		float u, v;
		float phi;
		float sinPhi, cosPhi;
		float x, y, z;
		
		for (j = 0; j < 2; j++) {
			v = static_cast<float>(j);
			
			x = position.x;
			y = position.y + height * (v - 0.5f);
			z = position.z;
			
			cylinderVertices.push_back(glm::vec3(x, y, z));
			cylinderNormals.push_back(glm::vec3(0.0f, v * 2.0f - 1.0f, 0.0f));
			cylinderUVCoords.push_back(glm::vec2(0.5f, 0.5f));
		}
		
		size_t offset = 0;
		
		for (i = 0; i < resolution; i++) {
			u = static_cast<float>(i) / static_cast<float>(resolution);
			phi = 2.0f * std::numbers::pi_v<float> * u;
			
			sinPhi = std::sin(phi);
			cosPhi = std::cos(phi);
			
			x = position.x + radius * sinPhi;
			z = position.z + radius * cosPhi;
			
			for (j = 0; j < 2; j++) {
				v = static_cast<float>(j);
				
				y = position.y + height * (v - 0.5f);
				
				cylinderVertices.push_back(glm::vec3(x, y, z));
				cylinderNormals.push_back(glm::vec3(0.0f, v * 2.0f - 1.0f, 0.0f));
				cylinderUVCoords.push_back(glm::vec2((sinPhi + 1.0f) * 0.5f, (cosPhi + 1.0f) * 0.5f));
				
				cylinderVertices.push_back(glm::vec3(x, y, z));
				cylinderNormals.push_back(glm::vec3(sinPhi, 0.0f, cosPhi));
				cylinderUVCoords.push_back(glm::vec2(u, v));
				
				cylinderIndices.push_back(2 + (offset + j * 2) % (vertexCount - 2));
				cylinderIndices.push_back(2 + (offset + j * 2 + 4) % (vertexCount - 2));
				cylinderIndices.push_back(j);
				
				cylinderIndices.push_back(2 + (offset + j * 2 + 1) % (vertexCount - 2));
				cylinderIndices.push_back(2 + (offset + j * 2 + 5) % (vertexCount - 2));
				cylinderIndices.push_back(2 + (offset + j * 2 + 3) % (vertexCount - 2));
			}
			
			offset += 4;
		}
		
		std::vector<glm::vec3> cylinderTangents;
		cylinderTangents.resize(cylinderVertices.size(), glm::vec3(0.0f));
		
		std::vector<size_t> cylinderTangentWeights;
		cylinderTangentWeights.resize(cylinderTangents.size(), 0);
		
		for (i = 0; i < cylinderIndices.size(); i += 3) {
			const auto index0 = cylinderIndices[i + 0];
			const auto index1 = cylinderIndices[i + 1];
			const auto index2 = cylinderIndices[i + 2];
			
			const std::array<glm::vec3, 3> positions = {
					cylinderVertices[index0],
					cylinderVertices[index1],
					cylinderVertices[index2]
			};
			
			const std::array<glm::vec2, 3> uvs = {
					cylinderUVCoords[index0],
					cylinderUVCoords[index1],
					cylinderUVCoords[index2]
			};
			
			const glm::vec3 tangent = generateTangent(positions, uvs);
			
			cylinderTangents[index0] += tangent;
			cylinderTangents[index1] += tangent;
			cylinderTangents[index2] += tangent;
			
			cylinderTangentWeights[index0]++;
			cylinderTangentWeights[index1]++;
			cylinderTangentWeights[index2]++;
		}
		
		for (i = 0; i < cylinderTangents.size(); i++) {
			if (cylinderTangentWeights[i] <= 0) {
				continue;
			}
			
			cylinderTangents[i] /= cylinderTangentWeights[i];
		}
		
		auto positionBuffer = buffer<glm::vec3>(core, BufferType::VERTEX, cylinderVertices.size());
		positionBuffer.fill(cylinderVertices);
		
		auto normalBuffer = buffer<glm::vec3>(core, BufferType::VERTEX, cylinderNormals.size());
		normalBuffer.fill(cylinderNormals);
		
		auto uvBuffer = buffer<glm::vec2>(core, BufferType::VERTEX, cylinderUVCoords.size());
		uvBuffer.fill(cylinderUVCoords);
		
		auto tangentBuffer = buffer<glm::vec3>(core, BufferType::VERTEX, cylinderTangents.size());
		tangentBuffer.fill(cylinderTangents);
		
		auto indexBuffer = buffer<uint32_t>(core, BufferType::INDEX, cylinderIndices.size());
		indexBuffer.fill(cylinderIndices);
		
		VertexData data ({
			vkcv::vertexBufferBinding(positionBuffer.getHandle()),
			vkcv::vertexBufferBinding(normalBuffer.getHandle()),
			vkcv::vertexBufferBinding(uvBuffer.getHandle()),
			vkcv::vertexBufferBinding(tangentBuffer.getHandle())
		});
		
		data.setIndexBuffer(indexBuffer.getHandle(), IndexBitCount::Bit32);
		data.setCount(indexBuffer.getCount());
		
		return data;
	}
	
}
