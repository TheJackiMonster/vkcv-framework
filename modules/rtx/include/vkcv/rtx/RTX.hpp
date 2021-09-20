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
        RTXBuffer m_shaderBindingTableBuffer;
        vk::DeviceSize m_shaderGroupBaseAlignment;

    public:

        /**
         * @brief Initializes the @#RTXModule with scene data.
         * @param core The reference to the @#Core.
         * @param asManager The reference to the @#ASManager.
         * @param vertices The vertex data of the scene.
         * @param indices The index data of the scene.
         * @param descriptorSetHandles The descriptor set handles for RTX.
         */
        RTXModule(Core* core, ASManager* asManager, std::vector<float>& vertices,
            std::vector<uint32_t>& indices, std::vector<vkcv::DescriptorSetHandle>& descriptorSetHandles);

        /**
         * @brief Default #RTXModule destructor.
         */
        ~RTXModule();

        /**
         * @brief Returns the RTX pipeline.
         * @return The RTX pipeline.
         */
        vk::Pipeline getPipeline();

        /**
         * @brief Returns the shader binding table buffer.
         * @return The shader binding table buffer.
         */
        vk::Buffer getShaderBindingTableBuffer();

        /**
         * @brief Returns the shader group base alignment for partitioning the shader binding table buffer.
         * @return The shader group base alignment.
         */
        vk::DeviceSize getShaderGroupBaseAlignment();

        /**
         * @brief Returns the RTX pipeline layout.
         * @return The RTX pipeline layout.
         */
        vk::PipelineLayout getPipelineLayout();

        /**
         * @brief Sets the shader group base alignment and creates the shader binding table by allocating a shader
         * binding table buffer. The allocation depends on @p shaderCount and the shader group base alignment.
         * @param shaderCount The amount of shaders to be used for RTX.
         */
        void createShaderBindingTable(uint32_t shaderCount);

        /**
         * @brief Creates Descriptor-Writes for RTX
         * @param descriptorSetHandles The descriptorSetHandles for RTX.
         */
        void RTXDescriptors(std::vector<vkcv::DescriptorSetHandle>& descriptorSetHandles);

        /**
         * @brief Creates the RTX pipeline and the RTX pipeline layout. Currently, only RayGen, RayClosestHit and
         * RayMiss are supported.
         * @param pushConstantSize The size of the push constant used in the RTX shaders.
         * @param descriptorSetLayouts The descriptor set layout handles.
         * @param rtxShader The RTX shader program.
         */
        void createRTXPipelineAndLayout(uint32_t pushConstantSize, std::vector<DescriptorSetLayoutHandle> descriptorSetLayouts, ShaderProgram &rtxShader);
    };

}
