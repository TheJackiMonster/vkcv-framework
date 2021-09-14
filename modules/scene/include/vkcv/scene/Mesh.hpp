#pragma once

#include <glm/mat4x4.hpp>

#include <vkcv/camera/Camera.hpp>

#include "MeshPart.hpp"

namespace vkcv::scene {

    /**
     * @addtogroup vkcv_scene
     * @{
     */
	
	typedef typename event_function<const glm::mat4&, const glm::mat4&, PushConstants&, vkcv::DrawcallInfo&>::type RecordMeshDrawcallFunction;
	
	class Node;
	
	class Mesh {
		friend class Node;
		
	private:
		Scene& m_scene;
		std::vector<MeshPart> m_parts;
		std::vector<DrawcallInfo> m_drawcalls;
		glm::mat4 m_transform;
		Bounds m_bounds;
		
		explicit Mesh(Scene& scene);
		
		void load(const asset::Scene& scene,
				  const asset::Mesh& mesh);
		
		void recordDrawcalls(const glm::mat4& viewProjection,
							 PushConstants& pushConstants,
							 std::vector<DrawcallInfo>& drawcalls,
							 const RecordMeshDrawcallFunction& record);
		
		[[nodiscard]]
		size_t getDrawcallCount() const;
	
	public:
		~Mesh();
		
		Mesh(const Mesh& other) = default;
		Mesh(Mesh&& other) = default;
		
		Mesh& operator=(const Mesh& other);
		Mesh& operator=(Mesh&& other) noexcept;
		
		[[nodiscard]]
		const Bounds& getBounds() const;
	
	};

    /** @} */
	
}
