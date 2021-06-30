#pragma once

#include <glm/mat4x4.hpp>

#include <vkcv/camera/Camera.hpp>

#include "MeshPart.hpp"

namespace vkcv::scene {
	
	class Node;
	
	class Mesh {
		friend class Node;
		
	private:
		Scene* m_scene;
		std::vector<MeshPart> m_parts;
		std::vector<DrawcallInfo> m_drawcalls;
		glm::mat4 m_transform;
		
		explicit Mesh(Scene* scene);
		
		void load(const asset::Scene& scene,
				  const asset::Mesh& mesh);
		
		void recordDrawcalls(std::vector<glm::mat4>& matrices, std::vector<DrawcallInfo>& drawcalls);
	
	public:
		~Mesh();
		
		Mesh(const Mesh& other);
		Mesh(Mesh&& other) noexcept;
		
		Mesh& operator=(const Mesh& other);
		Mesh& operator=(Mesh&& other) noexcept;
	
	};
	
}
