#pragma once
#include <vkcv/Core.hpp>
#include <vkcv/upscaling/FSR2Upscaling.hpp>

struct AppRenderTargets {
	vkcv::ImageHandle depthBuffer;
	vkcv::ImageHandle colorBuffer;
	vkcv::ImageHandle motionBuffer;
	vkcv::ImageHandle finalBuffer;
};

struct GraphicPassHandles {
	vkcv::GraphicsPipelineHandle        pipeline;
	vkcv::PassHandle                    renderPass;
	vkcv::DescriptorSetLayoutHandle     descriptorSetLayout;
	vkcv::DescriptorSetHandle           descriptorSet;
};

struct ComputePassHandles {
	vkcv::ComputePipelineHandle         pipeline;
	vkcv::DescriptorSetLayoutHandle     descriptorSetLayout;
	vkcv::DescriptorSetHandle           descriptorSet;
};

struct MeshResources {
	vkcv::VertexData    mesh;
	vkcv::BufferHandle  vertexBuffer;
	vkcv::BufferHandle  indexBuffer;
};

// loads position, uv and normal of the first mesh in a scene
bool loadMesh(vkcv::Core& core, const std::filesystem::path& path, MeshResources* outMesh);

bool loadImage(vkcv::Core& core, const std::filesystem::path& path, vkcv::ImageHandle* outImage);

bool loadGraphicPass(
	vkcv::Core& core,
	std::filesystem::path vertexPath,
	std::filesystem::path fragmentPath,
	const vkcv::PassConfig&     passConfig,
	vkcv::DepthTest       		depthTest,
	GraphicPassHandles*         outPassHandles);

bool loadMeshPass  (vkcv::Core& core, GraphicPassHandles* outHandles);
bool loadSkyPass   (vkcv::Core& core, GraphicPassHandles* outHandles);
bool loadPrePass   (vkcv::Core& core, GraphicPassHandles* outHandles);
bool loadSkyPrePass(vkcv::Core& core, GraphicPassHandles* outHandles);

bool loadComputePass(vkcv::Core& core, const std::filesystem::path& path, ComputePassHandles* outComputePass);

AppRenderTargets createRenderTargets(vkcv::Core& core,
									 uint32_t width,
									 uint32_t height,
									 vkcv::upscaling::FSR2QualityMode mode);