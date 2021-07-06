#include <vkcv/DrawcallRecording.hpp>
#include <vkcv/Logger.hpp>

namespace vkcv {

    vk::IndexType getIndexType(IndexBitCount indexByteCount){
        switch (indexByteCount) {
            case IndexBitCount::Bit16: return vk::IndexType::eUint16;
            case IndexBitCount::Bit32: return vk::IndexType::eUint32;
            default:
                vkcv_log(LogLevel::ERROR, "unknown Enum");
                return vk::IndexType::eUint16;
        }
    }

    void recordDrawcall(
        const DrawcallInfo      &drawcall,
        vk::CommandBuffer       cmdBuffer,
        vk::PipelineLayout      pipelineLayout,
        const PushConstantData  &pushConstantData,
        const size_t            drawcallIndex) {

        for (uint32_t i = 0; i < drawcall.mesh.vertexBufferBindings.size(); i++) {
            const auto& vertexBinding = drawcall.mesh.vertexBufferBindings[i];
            cmdBuffer.bindVertexBuffers(i, vertexBinding.buffer, vertexBinding.offset);
        }

        for (const auto& descriptorUsage : drawcall.descriptorSets) {
            cmdBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                pipelineLayout,
                descriptorUsage.setLocation,
                descriptorUsage.vulkanHandle,
                nullptr);
        }

        const size_t drawcallPushConstantOffset = drawcallIndex * pushConstantData.sizePerDrawcall;
        // char* cast because void* does not support pointer arithmetic
        const void* drawcallPushConstantData = drawcallPushConstantOffset + (char*)pushConstantData.data;

        cmdBuffer.pushConstants(
            pipelineLayout,
            vk::ShaderStageFlagBits::eAll,
            0,
            pushConstantData.sizePerDrawcall,
            drawcallPushConstantData);

        if (drawcall.mesh.indexBuffer) {
            cmdBuffer.bindIndexBuffer(drawcall.mesh.indexBuffer, 0, getIndexType(drawcall.mesh.indexBitCount));
            cmdBuffer.drawIndexed(drawcall.mesh.indexCount, 1, 0, 0, {});
        }
        else {
            cmdBuffer.draw(drawcall.mesh.indexCount, 1, 0, 0, {});
        }
    }



    struct MeshShaderFunctions
    {
        PFN_vkCmdDrawMeshTasksNV cmdDrawMeshTasks                           = nullptr;
        PFN_vkCmdDrawMeshTasksIndirectNV cmdDrawMeshTasksIndirect           = nullptr;
        PFN_vkCmdDrawMeshTasksIndirectCountNV cmdDrawMeshTasksIndirectCount = nullptr;
    } MeshShaderFunctions;

    void InitMeshShaderDrawFunctions(vk::Device device)
    {
        MeshShaderFunctions.cmdDrawMeshTasks = PFN_vkCmdDrawMeshTasksNV(device.getProcAddr("vkCmdDrawMeshTasksNV"));
        MeshShaderFunctions.cmdDrawMeshTasksIndirect = PFN_vkCmdDrawMeshTasksIndirectNV(device.getProcAddr("vkCmdDrawMeshTasksIndirectNV"));
        MeshShaderFunctions.cmdDrawMeshTasksIndirectCount = PFN_vkCmdDrawMeshTasksIndirectCountNV (device.getProcAddr( "vkCmdDrawMeshTasksIndirectCountNV"));
    }

    void recordMeshShaderDrawcall(
        vk::CommandBuffer                       cmdBuffer,
        vk::PipelineLayout                      pipelineLayout,
        const PushConstantData&                 pushConstantData,
        const uint32_t                          pushConstantOffset,
        const MeshShaderDrawcall&               drawcall,
        const uint32_t                          firstTask) {

        for (const auto& descriptorUsage : drawcall.descriptorSets) {
            cmdBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                pipelineLayout,
                descriptorUsage.setLocation,
                descriptorUsage.vulkanHandle,
                nullptr);
        }

        // char* cast because void* does not support pointer arithmetic
        const void* drawcallPushConstantData = pushConstantOffset + (char*)pushConstantData.data;

        if (pushConstantData.data) {
            cmdBuffer.pushConstants(
                pipelineLayout,
                vk::ShaderStageFlagBits::eAll,
                0,
                pushConstantData.sizePerDrawcall,
                drawcallPushConstantData);
        }

        MeshShaderFunctions.cmdDrawMeshTasks(VkCommandBuffer(cmdBuffer), drawcall.taskCount, firstTask);
    }
}