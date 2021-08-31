#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include "vkcv/Handles.hpp"
#include "PassManager.hpp"

/**
 *  * @authors Mark Mints
 * @file src/vkcv/ComputePipelineManager.hpp
 * @brief Creation and handling of Compute Pipelines
 */

namespace vkcv {

    class ComputePipelineManager
    {
    public:
        ComputePipelineManager() = delete; // no default ctor
        explicit ComputePipelineManager(vk::Device device) noexcept; // ctor
        ~ComputePipelineManager() noexcept; // dtor

        ComputePipelineManager(const ComputePipelineManager &other) = delete; // copy-ctor
        ComputePipelineManager(ComputePipelineManager &&other) = delete; // move-ctor;

        ComputePipelineManager & operator=(const ComputePipelineManager &other) = delete; // copy-assign op
        ComputePipelineManager & operator=(ComputePipelineManager &&other) = delete; // move-assign op

        ComputePipelineHandle createComputePipeline(
                const ShaderProgram& shaderProgram,
                const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);

    private:
        struct ComputePipeline {
            vk::Pipeline m_handle;
            vk::PipelineLayout m_layout;
            PipelineConfig m_config;
        };

        vk::Device m_Device;
        std::vector<ComputePipeline> m_Pipelines;

        void destroyPipelineById(uint64_t id);

        // TODO: Move to ComputePipelineManager
        vk::Result createShaderModule(vk::ShaderModule &module, const ShaderProgram &shaderProgram, ShaderStage stage);

    };

}
