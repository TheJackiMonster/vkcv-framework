#pragma once

#include <memory>
#include <vector>

#include <vkcv/Buffer.hpp>
#include <vkcv/material/Material.hpp>
#include <vkcv/asset/asset_loader.hpp>

#include "Bounds.hpp"

namespace vkcv::scene {
	
	class Scene;
	
	class MeshPart {
		friend class Mesh;
		
	private:
		Scene* m_scene;
		BufferHandle m_vertices;
		BufferHandle m_indices;
		Bounds m_bounds;
		size_t m_materialIndex;
		
		explicit MeshPart(Scene* scene);
		
		void load(const asset::Scene& scene,
				  const asset::VertexGroup& vertexGroup);
		
	public:
		~MeshPart();
		
		MeshPart(const MeshPart& other) = default;
		MeshPart(MeshPart&& other) = default;
		
		MeshPart& operator=(const MeshPart& other) = default;
		MeshPart& operator=(MeshPart&& other) = default;
		
		[[nodiscard]]
		const material::Material& getMaterial() const;
		
	};
	
	class Node;
	
	class Mesh {
		friend class Node;
		
	private:
		Scene* m_scene;
		std::vector<MeshPart> m_parts;
		
		explicit Mesh(Scene* scene);
		
		void load(const asset::Scene& scene,
				  const asset::Mesh& mesh);
	
	public:
		~Mesh() = default;
		
		Mesh(const Mesh& other) = default;
		Mesh(Mesh&& other) = default;
		
		Mesh& operator=(const Mesh& other) = default;
		Mesh& operator=(Mesh&& other) = default;
	
	};
	
}
