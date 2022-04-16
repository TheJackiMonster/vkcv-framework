#pragma once
/**
 * @authors Sebastian Gaida, Alexander Gauggel, Artur Wasmut, Tobias Frisch
 * @file vkcv/DrawcallRecording.hpp
 * @brief Structures and functions to record drawcalls.
 */

#include <vulkan/vulkan.hpp>

#include "Handles.hpp"
#include "DescriptorConfig.hpp"
#include "PushConstants.hpp"

#include "Buffer.hpp"

namespace vkcv {
	
    struct VertexBufferBinding {
        inline VertexBufferBinding(vk::DeviceSize offset, vk::Buffer buffer) noexcept
            : offset(offset), buffer(buffer) {}

        vk::DeviceSize  offset;
        vk::Buffer      buffer;
    };

    enum class IndexBitCount{
        Bit16,
        Bit32
    };

    struct DescriptorSetUsage {
        inline DescriptorSetUsage(uint32_t setLocation, DescriptorSetHandle descriptorSet,
								  const std::vector<uint32_t>& dynamicOffsets = {}) noexcept
            : setLocation(setLocation), descriptorSet(descriptorSet), dynamicOffsets(dynamicOffsets) {}

        const uint32_t          	setLocation;
        const DescriptorSetHandle 	descriptorSet;
        const std::vector<uint32_t> dynamicOffsets;
    };

    struct Mesh {

        inline Mesh(){}

        inline Mesh(
            std::vector<VertexBufferBinding>    vertexBufferBindings,
            vk::Buffer                          indexBuffer,
            size_t                              indexCount,
            IndexBitCount                       indexBitCount = IndexBitCount::Bit16) noexcept
            :
            vertexBufferBindings(vertexBufferBindings),
            indexBuffer(indexBuffer),
            indexCount(indexCount),
            indexBitCount(indexBitCount) {}

        std::vector<VertexBufferBinding>    vertexBufferBindings;
        vk::Buffer                          indexBuffer;
        size_t                              indexCount;
        IndexBitCount                       indexBitCount;

    };

    struct DrawcallInfo {
        inline DrawcallInfo(const Mesh& mesh, const std::vector<DescriptorSetUsage>& descriptorSets, const uint32_t instanceCount = 1)
            : mesh(mesh), descriptorSets(descriptorSets), instanceCount(instanceCount){}

        Mesh                            mesh;
        std::vector<DescriptorSetUsage> descriptorSets;
        uint32_t                        instanceCount;
    };

    void InitMeshShaderDrawFunctions(vk::Device device);

    struct MeshShaderDrawcall {
        inline MeshShaderDrawcall(const std::vector<DescriptorSetUsage> descriptorSets, uint32_t taskCount)
            : descriptorSets(descriptorSets), taskCount(taskCount) {}

        std::vector<DescriptorSetUsage> descriptorSets;
        uint32_t                        taskCount;
    };

    void recordMeshShaderDrawcall(
		const Core&								core,
        vk::CommandBuffer                       cmdBuffer,
        vk::PipelineLayout                      pipelineLayout,
        const PushConstants&                 	pushConstantData,
        const uint32_t                          pushConstantOffset,
        const MeshShaderDrawcall&               drawcall,
        const uint32_t                          firstTask);
	
}
