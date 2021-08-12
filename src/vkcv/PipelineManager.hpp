#pragma once

/**
 *  * @authors Mark Mints
 * @file src/vkcv/PipelineManager.hpp
 * @brief Creation and handling of pipelines
 */
// TODO: Edit @brief: add graphics pipeline, but only then when the compute part is in an own class.
// TODO: More authors? Do we need authors (general question)?

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

        /**
         * Creates a Graphics Pipeline based on the set shader stages in the Config Struct.
         * This function is wrapped in /src/vkcv/Core.cpp by Core::createGraphicsPipeline(const PipelineConfig &config).
         * Therefore the passManager is filled already by the overall context of an application.
         * On application level it is necessary first to fill a PipelineConfig Struct.
         * @param config Hands over all needed information for pipeline creation.
         * @param passManager Hands over the corresponding render pass.
         * @return A Handler to the created Pipeline Object.
         */
        PipelineHandle createPipeline(const PipelineConfig &config, PassManager& passManager);

        // TODO: Move to ComputePipelineManager
        PipelineHandle createComputePipeline(
                const ShaderProgram& shaderProgram,
                const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);

        /**
         * Returns a vk::Pipeline object by handle.
         * @param handle Directing to the requested pipeline.
         * @return vk::Pipeline.
         */
        [[nodiscard]]
        vk::Pipeline getVkPipeline(const PipelineHandle &handle) const;

        /**
         * Returns a vk::PipelineLayout object by handle.
         * @param handle Directing to the requested pipeline.
         * @return vk::PipelineLayout.
         */
        [[nodiscard]]
        vk::PipelineLayout getVkPipelineLayout(const PipelineHandle &handle) const;

        /**
         * Returns the corresponding Pipeline Config Struct of a pipeline object directed by the given Handler.
         * @param handle Directing to the requested pipeline.
         * @return Pipeline Config Struct
         */
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

        // TODO: Move to ComputePipelineManager
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
         * Creates a Pipeline Vertex Input State Create Info Struct and fills it with Attribute and Binding data.
         * @param vertexAttributeDescriptions
         * @param vertexBindingDescriptions
         * @return Pipeline Vertex Input State Create Info Struct
         */
        vk::PipelineVertexInputStateCreateInfo createPipelineVertexInputStateCreateInfo(
                std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions,
                std::vector<vk::VertexInputBindingDescription>   &vertexBindingDescriptions
        );

        /**
         * Creates a Pipeline Input Assembly State Create Info Struct with 'Primitive Restart' disabled.
         * @param config provides data for primitive topology.
         * @return Pipeline Input Assembly State Create Info Struct
         */
        vk::PipelineInputAssemblyStateCreateInfo createPipelineInputAssemblyStateCreateInfo(const PipelineConfig &config);

        /**
         * Creates a Pipeline Viewport State Create Info Struct with default set viewport and scissor settings.
         * @param config provides with and height of the output window
         * @return Pipeline Viewport State Create Info Struct
         */
        vk::PipelineViewportStateCreateInfo createPipelineViewportStateCreateInfo(const PipelineConfig &config);

        /**
         * Creates a Pipeline Rasterization State Create Info Struct with default values set to:
         * Rasterizer Discard: Disabled
         * Polygon Mode: Fill
         * Front Face: Counter Clockwise
         * Depth Bias: Disabled
         * Line Width: 1.0
         * Depth Clamping and Culling Mode ist set by the Pipeline Config
         * @param config sets Depth Clamping and Culling Mode
         * @return Pipeline Rasterization State Create Info Struct
         */
        vk::PipelineRasterizationStateCreateInfo createPipelineRasterizationStateCreateInfo(const PipelineConfig &config);

        /**
         * Creates a Pipeline Multisample State Create Info Struct.
         * @param config set MSAA Sample Count Flag
         * @return Pipeline Multisample State Create Info Struct
         */
        vk::PipelineMultisampleStateCreateInfo createPipelineMultisampleStateCreateInfo(const PipelineConfig &config);

        /**
         * Creates a Pipeline Color Blend State Create Info Struct.
         * Currently only one blend mode is supported! There for, blending is set to additive.
         * @param config sets blend mode
         * @return
         */
        vk::PipelineColorBlendStateCreateInfo createPipelineColorBlendStateCreateInfo(const PipelineConfig &config);

        /**
         * Creates a Pipeline Layout Create Info Struct.
         * @param config sets Push Constant Size and Descriptor Layouts.
         * @return Pipeline Layout Create Info Struct
         */
        vk::PipelineLayoutCreateInfo createPipelineLayoutCreateInfo(const PipelineConfig &config);

        /**
         * Creates a Pipeline Depth Stencil State Create Info Struct.
         * @param config sets if depth test in enabled or not.
         * @return Pipeline Layout Create Info Struct
         */
        vk::PipelineDepthStencilStateCreateInfo createPipelineDepthStencilStateCreateInfo(const PipelineConfig &config);

        /**
         * Creates a Pipeline Dynamic State Create Info Struct.
         * @param config sets whenever a dynamic viewport is used or not.
         * @return Pipeline Dynamic State Create Info Struct
         */
        vk::PipelineDynamicStateCreateInfo createPipelineDynamicStateCreateInfo(const PipelineConfig &config);
    };
}
