
#include "vkcv/DrawcallRecording.hpp"
#include "vkcv/Logger.hpp"
#include "vkcv/Core.hpp"

namespace vkcv {

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

    void recordMeshShaderDrawcall(const Core& core,
                                  vk::CommandBuffer cmdBuffer,
                                  vk::PipelineLayout pipelineLayout,
                                  const PushConstants& pushConstantData,
                                  uint32_t pushConstantOffset,
                                  const MeshShaderDrawcall& drawcall,
                                  uint32_t firstTask) {

        for (const auto& descriptorUsage : drawcall.descriptorSets) {
            cmdBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                pipelineLayout,
                descriptorUsage.setLocation,
				core.getDescriptorSet(descriptorUsage.descriptorSet).vulkanHandle,
                nullptr);
        }

        // char* cast because void* does not support pointer arithmetic
        const void* drawcallPushConstantData = pushConstantOffset + (char*)pushConstantData.getData();

        if (pushConstantData.getData()) {
            cmdBuffer.pushConstants(
                pipelineLayout,
                vk::ShaderStageFlagBits::eAll,
                0,
                pushConstantData.getSizePerDrawcall(),
                drawcallPushConstantData);
        }

        MeshShaderFunctions.cmdDrawMeshTasks(VkCommandBuffer(cmdBuffer), drawcall.taskCount, firstTask);
    }
}
