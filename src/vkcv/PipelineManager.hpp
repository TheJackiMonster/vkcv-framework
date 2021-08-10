#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include "vkcv/Handles.hpp"
#include "vkcv/PipelineConfig.hpp"
#include "PassManager.hpp"

namespace vkcv
{
    class PipelineManager
    {
    public:
        PipelineManager() = delete; // no default ctor
        explicit PipelineManager(vk::Device device) noexcept; // ctor
        ~PipelineManager() noexcept; // dtor

        PipelineManager(const PipelineManager &other) = delete; // copy-ctor
        PipelineManager(PipelineManager &&other) = delete; // move-ctor;

        PipelineManager & operator=(const PipelineManager &other) = delete; // copy-assign op
        PipelineManager & operator=(PipelineManager &&other) = delete; // move-assign op

        PipelineHandle createPipeline(const PipelineConfig &config, PassManager& passManager);

        PipelineHandle createComputePipeline(
                const ShaderProgram& shaderProgram,
                const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);

        [[nodiscard]]
        vk::Pipeline getVkPipeline(const PipelineHandle &handle) const;

        [[nodiscard]]
        vk::PipelineLayout getVkPipelineLayout(const PipelineHandle &handle) const;

        [[nodiscard]]
        const PipelineConfig &getPipelineConfig(const PipelineHandle &handle) const;

    private:
        struct Pipeline {
            vk::Pipeline m_handle;
            vk::PipelineLayout m_layout;
            PipelineConfig m_config;
        };

        vk::Device m_Device;
        std::vector<Pipeline> m_Pipelines;

        void destroyPipelineById(uint64_t id);

        vk::Result createShaderModule(vk::ShaderModule &module, const ShaderProgram &shaderProgram, ShaderStage stage);

        /**
         * Fills Vertex Attribute and Binding Description with the corresponding objects form the Vertex Layout.
         * @param vertexAttributeDescriptions
         * @param vertexBindingDescriptions
         * @param existsVertexShader
         * @param config
         */
        void fillVertexInputDescription(
                std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions,
                std::vector<vk::VertexInputBindingDescription>   &vertexBindingDescriptions,
                const bool existsVertexShader,
                const PipelineConfig &config);

        /**
         * Create a Pipeline Vertex Input State Create Info Struct and fills it with Attribute and Binding data.
         * @param vertexAttributeDescriptions
         * @param vertexBindingDescriptions
         * @return Pipeline Vertex Input State Create Info Struct
         */
        vk::PipelineVertexInputStateCreateInfo createPipelineVertexInputStateCreateInfo(
                std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions,
                std::vector<vk::VertexInputBindingDescription>   &vertexBindingDescriptions
        );
    };
}
