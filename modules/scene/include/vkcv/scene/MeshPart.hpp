#pragma once

#include <vector>

#include <vkcv/Buffer.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/material/Material.hpp>

#include "Bounds.hpp"

namespace vkcv::scene {
	
	class Scene;
	class Mesh;
	
	class MeshPart {
		friend class Mesh;
	
	private:
		Scene& m_scene;
		BufferHandle m_vertices;
		std::vector<VertexBufferBinding> m_vertexBindings;
		BufferHandle m_indices;
		size_t m_indexCount;
		Bounds m_bounds;
		size_t m_materialIndex;
		
		explicit MeshPart(Scene& scene);
		
		void load(const asset::Scene& scene,
				  const asset::VertexGroup& vertexGroup,
				  std::vector<DrawcallInfo>& drawcalls);
	
	public:
		~MeshPart();
		
		MeshPart(const MeshPart& other);
		MeshPart(MeshPart&& other);
		
		MeshPart& operator=(const MeshPart& other);
		MeshPart& operator=(MeshPart&& other) noexcept;
		
		[[nodiscard]]
		const material::Material& getMaterial() const;
		
		[[nodiscard]]
		const Bounds& getBounds() const;
		
		explicit operator bool() const;
		bool operator!() const;
		
	};

}
