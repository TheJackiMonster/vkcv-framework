#pragma once

/**
 * @authors Mark Mints
 * @file src/vkcv/ComputePipelineManager.hpp
 * @brief Creation and handling of Compute Pipelines
 */

#include <vulkan/vulkan.hpp>
#include <vector>

#include "vkcv/Handles.hpp"
#include "vkcv/ShaderProgram.hpp"
#include "vkcv/ComputePipelineConfig.hpp"

namespace vkcv
{

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

        /**
        * Returns a vk::Pipeline object by handle.
        * @param handle Directing to the requested pipeline.
        * @return vk::Pipeline.
        */
        [[nodiscard]]
        vk::Pipeline getVkPipeline(const ComputePipelineHandle &handle) const;

        /**
         * Returns a vk::PipelineLayout object by handle.
         * @param handle Directing to the requested pipeline.
         * @return vk::PipelineLayout.
         */
        [[nodiscard]]
        vk::PipelineLayout getVkPipelineLayout(const ComputePipelineHandle &handle) const;

        /**
         * Creates a Compute Pipeline based on the set shader stages in the Config Struct.
         * This function is wrapped in /src/vkcv/Core.cpp by Core::createComputePipeline(const ComputePipelineConfig &config).
         * On application level it is necessary first to fill a ComputePipelineConfig Struct.
         * @param config Hands over all needed information for pipeline creation.
         * @return A Handler to the created Compute Pipeline Object.
         */
        ComputePipelineHandle createComputePipeline(const ComputePipelineConfig& config);

    private:
        struct ComputePipeline {
            vk::Pipeline m_handle;
            vk::PipelineLayout m_layout;
        };

        vk::Device m_Device;
        std::vector<ComputePipeline> m_Pipelines;

        void destroyPipelineById(uint64_t id);

        vk::Result createShaderModule(vk::ShaderModule &module, const ShaderProgram &shaderProgram, ShaderStage stage);

    };

}
