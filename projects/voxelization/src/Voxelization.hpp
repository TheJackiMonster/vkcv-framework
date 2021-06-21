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
	Voxelization(
		vkcv::Core*         corePtr, 
		const Dependencies& dependencies, 
		vkcv::BufferHandle  lightInfoBuffer,
		vkcv::ImageHandle   shadowMap,
		vkcv::SamplerHandle shadowSampler);

	void voxelizeMeshes(
		vkcv::CommandStreamHandle                       cmdStream,
		const glm::vec3&                                cameraPosition,
		const glm::vec3&                                cameraLookDirection,
		const std::vector<vkcv::Mesh>&                  meshes,
		const std::vector<glm::mat4>&                   modelMatrices,
		const std::vector<vkcv::DescriptorSetHandle>&   perMeshDescriptorSets);

	void renderVoxelVisualisation(
		vkcv::CommandStreamHandle               cmdStream,
		const glm::mat4&                        viewProjectin,
		const std::vector<vkcv::ImageHandle>&   renderTargets,
		uint32_t                                mipLevel);

	void setVoxelExtent(float extent);

	vkcv::ImageHandle getVoxelImageHandle() const;
	vkcv::BufferHandle getVoxelInfoBufferHandle() const;

private:
	vkcv::Core* m_corePtr;

	struct VoxelBufferContent{
		uint32_t isFilled;
	};

	vkcv::Image                         m_voxelImage;
    vkcv::Buffer<VoxelBufferContent>    m_voxelBuffer;

	vkcv::Image                 m_dummyRenderTarget;
	vkcv::PassHandle            m_voxelizationPass;
	vkcv::PipelineHandle        m_voxelizationPipe;
	vkcv::DescriptorSetHandle   m_voxelizationDescriptorSet;

	vkcv::PipelineHandle        m_voxelResetPipe;
	vkcv::DescriptorSetHandle   m_voxelResetDescriptorSet;

	vkcv::PipelineHandle        m_bufferToImagePipe;
	vkcv::DescriptorSetHandle   m_bufferToImageDescriptorSet;

	vkcv::PassHandle            m_visualisationPass;
	vkcv::PipelineHandle        m_visualisationPipe;

	vkcv::DescriptorSetHandle   m_visualisationDescriptorSet;

	struct VoxelizationInfo {
		glm::vec3 offset;
		float extent;
	};
	vkcv::Buffer<VoxelizationInfo> m_voxelInfoBuffer;

	float m_voxelExtent = 30.f;
};