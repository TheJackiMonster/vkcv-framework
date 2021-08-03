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

bool loadSphereMesh(vkcv::Core& core, MeshResources* outMesh);
bool loadMeshGraphicPass(vkcv::Core& core, GraphicPassHandles* outPassHandles);

RenderTargets createRenderTargets(vkcv::Core& core, const uint32_t width, const uint32_t height);