#include <vkcv/DrawcallRecording.hpp>

namespace vkcv {

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

        cmdBuffer.bindIndexBuffer(drawcall.mesh.indexBuffer, 0, vk::IndexType::eUint16);	//FIXME: choose proper size

        const size_t drawcallPushConstantOffset = drawcallIndex * pushConstantData.sizePerDrawcall;
        // char* cast because void* does not support pointer arithmetic
        const void* drawcallPushConstantData = drawcallPushConstantOffset + (char*)pushConstantData.data;

        cmdBuffer.pushConstants(
            pipelineLayout,
            vk::ShaderStageFlagBits::eAll,
            0,
            pushConstantData.sizePerDrawcall,
            drawcallPushConstantData);

        cmdBuffer.drawIndexed(drawcall.mesh.indexCount, 1, 0, 0, {});
    }
}