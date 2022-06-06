
#include "vkcv/DrawcallRecording.hpp"
#include "vkcv/Logger.hpp"
#include "vkcv/Core.hpp"

namespace vkcv {

    void recordMeshShaderDrawcall(const Core& core,
                                  vk::CommandBuffer cmdBuffer,
                                  vk::PipelineLayout pipelineLayout,
                                  const PushConstants& pushConstantData,
                                  uint32_t pushConstantOffset,
                                  const MeshShaderDrawcall& drawcall,
                                  uint32_t firstTask) {
        static PFN_vkCmdDrawMeshTasksNV cmdDrawMeshTasks = reinterpret_cast<PFN_vkCmdDrawMeshTasksNV>(
                core.getContext().getDevice().getProcAddr("vkCmdDrawMeshTasksNV")
        );
	
		if (!cmdDrawMeshTasks) {
			vkcv_log(LogLevel::ERROR, "Mesh shader drawcalls are not supported");
			return;
		}

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
    
        cmdDrawMeshTasks(VkCommandBuffer(cmdBuffer), drawcall.taskCount, firstTask);
    }
}
