#pragma once

/**
 * @authors Mark Mints
 * @file src/vkcv/PipelineManager.hpp
 * @brief Creation and handling of Graphic Pipelines
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

    };
}
