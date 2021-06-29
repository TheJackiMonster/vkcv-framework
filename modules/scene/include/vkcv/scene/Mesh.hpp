#pragma once

#include <memory>
#include <vector>

#include <vkcv/Buffer.hpp>
#include <vkcv/material/Material.hpp>
#include <vkcv/asset/asset_loader.hpp>

#include "Bounds.hpp"

namespace vkcv::scene {
	
	class MeshPart {
		friend class Mesh;
		
	private:
		material::Material* m_material;
		Buffer<uint8_t> m_vertices;
		Buffer<uint8_t> m_indices;
		Bounds m_bounds;
		
		MeshPart();
		
		void load(const asset::Scene& scene,
				  const asset::VertexGroup& vertexGroup);
		
	public:
		~MeshPart();
		
	};
	
	class Mesh {
	private:
		std::vector<MeshPart> m_parts;
	
	public:
	
	};
	
}
