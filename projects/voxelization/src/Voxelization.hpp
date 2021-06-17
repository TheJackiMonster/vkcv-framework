#pragma once
#include <vkcv/Core.hpp>
#include <glm/glm.hpp>

class Voxelization{
public:
	struct Dependencies {
		vkcv::VertexLayout  vertexLayout;
		vk::Format          colorBufferFormat;
		vk::Format          depthBufferFormat;
	};
	Voxelization(vkcv::Core* corePtr, const Dependencies& dependencies);

	void voxelizeMeshes(
		vkcv::CommandStreamHandle       cmdStream, 
		const glm::vec3&                cameraPosition, 
		const std::vector<vkcv::Mesh>&  meshes,
		const std::vector<glm::mat4>&   modelMatrices);

	void renderVoxelVisualisation(
		vkcv::CommandStreamHandle               cmdStream,
		const glm::mat4&                        viewProjectin,
		const std::vector<vkcv::ImageHandle>&   renderTargets);

private:
	vkcv::Core* m_corePtr;

	vkcv::Image                 m_voxelImage;
	vkcv::Image                 m_dummyRenderTarget;
	vkcv::PassHandle            m_voxelizationPass;
	vkcv::PipelineHandle        m_voxelizationPipe;
	vkcv::DescriptorSetHandle   m_voxelizationDescriptorSet;

	vkcv::PipelineHandle        m_voxelResetPipe;
	vkcv::DescriptorSetHandle   m_voxelResetDescriptorSet;

	vkcv::PassHandle            m_visualisationPass;
	vkcv::PipelineHandle        m_visualisationPipe;
	vkcv::Buffer<uint16_t>      m_visualisationIndexBuffer;

	vkcv::DescriptorSetHandle   m_visualisationDescriptorSet;

	struct VoxelizationInfo {
		glm::vec3 offset;
		float extent;
	};
	vkcv::Buffer<VoxelizationInfo> m_voxelInfoBuffer;

	const float m_voxelExtent = 20.f;
};