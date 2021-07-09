#include <vkcv/DrawcallRecording.hpp>

namespace vkcv {

    void recordDrawcall(
        const DrawcallInfo      &drawcall,
        vk::CommandBuffer       cmdBuffer,
        vk::PipelineLayout      pipelineLayout,
        const PushConstants     &pushConstants,
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

        if (pushConstants.getSizePerDrawcall() > 0) {
            cmdBuffer.pushConstants(
                pipelineLayout,
                vk::ShaderStageFlagBits::eAll,
                0,
				pushConstants.getSizePerDrawcall(),
                pushConstants.getDrawcallData(drawcallIndex));
        }

        if (drawcall.mesh.indexBuffer) {
            cmdBuffer.bindIndexBuffer(drawcall.mesh.indexBuffer, 0, vk::IndexType::eUint16);	//FIXME: choose proper size
            cmdBuffer.drawIndexed(drawcall.mesh.indexCount, drawcall.instanceCount, 0, 0, {});
        }
        else {
            cmdBuffer.draw(drawcall.mesh.indexCount, 1, 0, 0, {});
        }
    }
}
