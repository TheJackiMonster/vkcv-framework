#pragma once

#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <vkcv/Image.hpp>
#include <glm/glm.hpp>
#include <vkcv/camera/Camera.hpp>
#include <vkcv/Downsampler.hpp>

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
		vkcv::SamplerHandle shadowSampler,
		vkcv::SamplerHandle voxelSampler,
		vkcv::Multisampling msaa);

	void voxelizeMeshes(
		vkcv::CommandStreamHandle                       cmdStream,
		const std::vector<vkcv::VertexData>&            meshes,
		const std::vector<glm::mat4>&                   modelMatrices,
		const std::vector<vkcv::DescriptorSetHandle>&   perMeshDescriptorSets,
		const vkcv::WindowHandle&                       windowHandle,
		vkcv::Downsampler&								downsampler);

	void renderVoxelVisualisation(
		vkcv::CommandStreamHandle               cmdStream,
		const glm::mat4&                        viewProjectin,
		const std::vector<vkcv::ImageHandle>&   renderTargets,
		uint32_t                                mipLevel,
		const vkcv::WindowHandle&               windowHandle);

	void updateVoxelOffset(const vkcv::camera::Camera& camera);
	void setVoxelExtent(float extent);

	vkcv::ImageHandle   getVoxelImageHandle() const;
	vkcv::BufferHandle  getVoxelInfoBufferHandle() const;

	glm::vec3   getVoxelOffset() const;
	float       getVoxelExtent() const;

private:
	vkcv::Core* m_corePtr;

	struct VoxelBufferContent{
		uint32_t lightEncoded;
		uint32_t normalEncoded;
		uint32_t albedoEncoded;
	};

	vkcv::Image                         m_voxelImageIntermediate;
	vkcv::Image                         m_voxelImage;
	vkcv::Buffer<VoxelBufferContent>    m_voxelBuffer;

	vkcv::Image                         m_dummyRenderTarget;
	vkcv::PassHandle                    m_voxelizationPass;
	vkcv::GraphicsPipelineHandle        m_voxelizationPipe;
	vkcv::DescriptorSetLayoutHandle     m_voxelizationDescriptorSetLayout;
	vkcv::DescriptorSetHandle           m_voxelizationDescriptorSet;

	vkcv::ComputePipelineHandle         m_voxelResetPipe;
	vkcv::DescriptorSetLayoutHandle     m_voxelResetDescriptorSetLayout;
	vkcv::DescriptorSetHandle           m_voxelResetDescriptorSet;

	vkcv::ComputePipelineHandle         m_bufferToImagePipe;
	vkcv::DescriptorSetLayoutHandle     m_bufferToImageDescriptorSetLayout;
	vkcv::DescriptorSetHandle           m_bufferToImageDescriptorSet;

	vkcv::PassHandle                    m_visualisationPass;
	vkcv::GraphicsPipelineHandle        m_visualisationPipe;

	vkcv::ComputePipelineHandle         m_secondaryBouncePipe;
	vkcv::DescriptorSetLayoutHandle     m_secondaryBounceDescriptorSetLayout;
	vkcv::DescriptorSetHandle           m_secondaryBounceDescriptorSet;

	vkcv::DescriptorSetLayoutHandle     m_visualisationDescriptorSetLayout;
	vkcv::DescriptorSetHandle           m_visualisationDescriptorSet;

	struct VoxelizationInfo {
		glm::vec3 offset;
		float extent;
	};
	vkcv::Buffer<VoxelizationInfo> m_voxelInfoBuffer;

	VoxelizationInfo m_voxelInfo;
};