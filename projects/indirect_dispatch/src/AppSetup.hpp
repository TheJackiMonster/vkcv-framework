#pragma once
#include <vkcv/Core.hpp>

struct RenderTargets {
	vkcv::ImageHandle depthBuffer;
};

struct GraphicPassHandles {
	vkcv::PipelineHandle    pipeline;
	vkcv::PassHandle        renderPass;
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
	GraphicPassHandles*         outPassHandles);

bool loadMeshPass(vkcv::Core& core, GraphicPassHandles* outHandles);
bool loadSkyPass (vkcv::Core& core, GraphicPassHandles* outHandles);

RenderTargets createRenderTargets(vkcv::Core& core, const uint32_t width, const uint32_t height);