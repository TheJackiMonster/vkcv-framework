
#include "vkcv/geometry/Sphere.hpp"

#include <numbers>

namespace vkcv::geometry {
	
	Sphere::Sphere(const glm::vec3& position, float radius)
	: Volume(position), Circular(radius) {}
	
	float Sphere::distanceTo(const glm::vec3 &point) {
		return glm::distance(getPosition(), point) - getRadius();
	}
	
	VertexData Sphere::generateVertexData(vkcv::Core &core) const {
		const auto& position = getPosition();
		const auto radius = getRadius();
		const auto resolution = getResolution();
		
		const size_t vertexCount = resolution * (resolution + 1);
		
		std::vector<glm::vec3> sphereVertices;
		std::vector<glm::vec3> sphereNormals;
		std::vector<glm::vec2> sphereUVCoords;
		
		sphereVertices.reserve(vertexCount);
		sphereNormals.reserve(vertexCount);
		sphereUVCoords.reserve(vertexCount);
		
		std::vector<uint32_t> sphereIndices;
		sphereIndices.reserve((resolution - 1) * resolution * 6);
		
		size_t i, j;
		float u, v;
		float phi, theta;
		float sinTheta;
		float x, y, z;
		
		for (i = 0; i < resolution; i++) {
			v = static_cast<float>(i) / static_cast<float>(resolution - 1);
			theta = std::numbers::pi_v<float> * v;
			sinTheta = std::sin(theta);
			
			y = position.y + radius * std::cos(theta);
			
			for (j = 0; j < resolution; j++) {
				u = static_cast<float>(j) / static_cast<float>(resolution);
				phi = 2.0f * std::numbers::pi_v<float> * u;
				
				x = position.x + radius * sinTheta * std::sin(phi);
				z = position.z + radius * sinTheta * std::cos(phi);
				
				sphereVertices.push_back(glm::vec3(x, y, z));
				sphereNormals.push_back(glm::vec3(x, y, z) / radius);
				sphereUVCoords.push_back(glm::vec2(u, 1.0f - v));
			}
			
			x = position.x;
			z = position.z + radius * sinTheta;
			
			sphereVertices.push_back(glm::vec3(x, y, z));
			sphereNormals.push_back(glm::vec3(x, y, z) / radius);
			sphereUVCoords.push_back(glm::vec2(1.0f, 1.0f - v));
		}
		
		size_t offset = 0;
		
		for (i = 1; i < resolution; i++) {
			for (j = 0; j < resolution; j++) {
				sphereIndices.push_back((offset + j) % vertexCount);
				sphereIndices.push_back((offset + resolution + j + 1) % vertexCount);
				sphereIndices.push_back((offset + resolution + j + 2) % vertexCount);
				
				sphereIndices.push_back((offset + resolution + j + 2) % vertexCount);
				sphereIndices.push_back((offset + j + 1) % vertexCount);
				sphereIndices.push_back((offset + j) % vertexCount);
			}
			
			offset += (resolution + 1);
		}
		
		auto positionBuffer = buffer<glm::vec3>(core, BufferType::VERTEX, sphereVertices.size());
		positionBuffer.fill(sphereVertices);
		
		auto normalBuffer = buffer<glm::vec3>(core, BufferType::VERTEX, sphereNormals.size());
		normalBuffer.fill(sphereNormals);
		
		auto uvBuffer = buffer<glm::vec2>(core, BufferType::VERTEX, sphereUVCoords.size());
		uvBuffer.fill(sphereUVCoords);
		
		auto indexBuffer = buffer<uint32_t>(core, BufferType::INDEX, sphereIndices.size());
		indexBuffer.fill(sphereIndices);
		
		VertexData data ({
			vkcv::vertexBufferBinding(positionBuffer.getHandle()),
			vkcv::vertexBufferBinding(normalBuffer.getHandle()),
			vkcv::vertexBufferBinding(uvBuffer.getHandle())
		});
		
		data.setIndexBuffer(indexBuffer.getHandle(), IndexBitCount::Bit32);
		data.setCount(indexBuffer.getCount());
		
		return data;
	}

}
