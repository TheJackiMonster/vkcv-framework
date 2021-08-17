#pragma once
#include <vkcv/Core.hpp>

struct AppRenderTargets {
	vkcv::ImageHandle depthBuffer;
	vkcv::ImageHandle colorBuffer;
	vkcv::ImageHandle motionBuffer;
};

struct GraphicPassHandles {
	vkcv::PipelineHandle    pipeline;
	vkcv::PassHandle        renderPass;
};

struct ComputePassHandles {
	vkcv::PipelineHandle        pipeline;
	vkcv::DescriptorSetHandle   descriptorSet;
};

struct MeshResources {
	vkcv::Mesh          mesh;
	vkcv::BufferHandle  vertexBuffer;
	vkcv::BufferHandle  indexBuffer;
};

// loads position and normal of the first mesh in a scene
bool loadMesh(vkcv::Core& core, const std::filesystem::path& path, MeshResources* outMesh);

bool loadGraphicPass(
	vkcv::Core& core,
	const std::filesystem::path vertexPath,
	const std::filesystem::path fragmentPath,
	const vkcv::PassConfig&     passConfig,
	const vkcv::DepthTest       depthTest,
	GraphicPassHandles*         outPassHandles);

bool loadMeshPass  (vkcv::Core& core, GraphicPassHandles* outHandles);
bool loadSkyPass   (vkcv::Core& core, GraphicPassHandles* outHandles);
bool loadPrePass   (vkcv::Core& core, GraphicPassHandles* outHandles);
bool loadSkyPrePass(vkcv::Core& core, GraphicPassHandles* outHandles);

bool loadComputePass(vkcv::Core& core, const std::filesystem::path& path, ComputePassHandles* outComputePass);

AppRenderTargets createRenderTargets(vkcv::Core& core, const uint32_t width, const uint32_t height);