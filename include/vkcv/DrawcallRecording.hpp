#pragma once
#include <vulkan/vulkan.hpp>
#include <vkcv/Handles.hpp>
#include <vkcv/DescriptorConfig.hpp>
#include <vkcv/PushConstants.hpp>

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
        inline DescriptorSetUsage(uint32_t setLocation, vk::DescriptorSet vulkanHandle,
								  const std::vector<uint32_t>& dynamicOffsets = {}) noexcept
            : setLocation(setLocation), vulkanHandle(vulkanHandle), dynamicOffsets(dynamicOffsets) {}

        const uint32_t          	setLocation;
        const vk::DescriptorSet 	vulkanHandle;
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

    void recordDrawcall(
        const DrawcallInfo      &drawcall,
        vk::CommandBuffer       cmdBuffer,
        vk::PipelineLayout      pipelineLayout,
        const PushConstants     &pushConstants,
        const size_t            drawcallIndex);

    void InitMeshShaderDrawFunctions(vk::Device device);

    struct MeshShaderDrawcall {
        inline MeshShaderDrawcall(const std::vector<DescriptorSetUsage> descriptorSets, uint32_t taskCount)
            : descriptorSets(descriptorSets), taskCount(taskCount) {}

        std::vector<DescriptorSetUsage> descriptorSets;
        uint32_t                        taskCount;
    };

    void recordMeshShaderDrawcall(
        vk::CommandBuffer                       cmdBuffer,
        vk::PipelineLayout                      pipelineLayout,
        const PushConstants&                 pushConstantData,
        const uint32_t                          pushConstantOffset,
        const MeshShaderDrawcall&               drawcall,
        const uint32_t                          firstTask);
}
