#pragma once

#include <vector>
#include "vulkan/vulkan.hpp"
#include "vkcv/Core.hpp"
#include "ASManager.hpp"

namespace vkcv::rtx {

    class RTXModule {
    private:

        Core* m_core;
        ASManager* m_asManager;
        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_pipelineLayout;
        RTXBuffer m_shaderBindingtableBuffer;
        vk::DeviceSize m_shaderGroupBaseAlignment;

    public:

        /**
         * TODO
         * @brief Initializes the RTXModule with scene data.
         * @param core The reference to the #Core.
         * @param vertices The scene vertex data of type uint8_t.
         * @param indices The scene index data of type uint8_t.
         * @param descriptorSetHandles The descriptorSetHandles for RTX
         */
        RTXModule(Core* core, ASManager* asManager, std::vector<float>& vertices,
            std::vector<uint32_t>& indices, std::vector<vkcv::DescriptorSetHandle>& descriptorSetHandles);

        /**
         * @brief Default #RTXModule destructor.
         */
        ~RTXModule();

        /**
         * @brief TODO
         * @return
         */
        vk::Pipeline getPipeline();

        /** TODO
        */
        vk::Buffer getShaderBindingBuffer();

        /** TODO
        */
        vk::DeviceSize getShaderGroupBaseAlignment();

        /**
         * @brief TODO
         * @return
         */
        vk::PipelineLayout getPipelineLayout();

        /** TODO
        */
        void createShaderBindingTable(uint32_t shaderCount);

        /**
         * @brief Creates Descriptor-Writes for RTX
         * @param descriptorSetHandles The descriptorSetHandles for RTX
         */
        void RTXDescriptors(std::vector<vkcv::DescriptorSetHandle>& descriptorSetHandles);

        /**
         * TODO
         * @brief Returns the Vulkan handle of the RTX pipeline.
         * @param descriptorSetLayouts The descriptorSetLayouts used for creating a @p vk::PipelineLayoutCreateInfo.
         * @param rayGenShader The ray generation shader.
         * @param rayMissShader The ray miss shader.
         * @param rayClostestHitShader The ray closest hit shader.
         * @return The Vulkan handle of the RTX pipeline.
         */
        void createRTXPipeline(uint32_t pushConstantSize, std::vector<DescriptorSetLayoutHandle> descriptorSetLayouts, ShaderProgram &rayGenShader, ShaderProgram &rayMissShader, ShaderProgram &rayClosestHitShader);
    };

}
