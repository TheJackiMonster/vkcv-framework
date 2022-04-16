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
#include "vkcv/GraphicsPipelineConfig.hpp"
#include "PassManager.hpp"
#include "DescriptorManager.hpp"

namespace vkcv
{
    class GraphicsPipelineManager
    {
    public:
		GraphicsPipelineManager() = delete; // no default ctor
        explicit GraphicsPipelineManager(vk::Device device, vk::PhysicalDevice physicalDevice) noexcept; // ctor
        ~GraphicsPipelineManager() noexcept; // dtor
	
		GraphicsPipelineManager(const GraphicsPipelineManager &other) = delete; // copy-ctor
		GraphicsPipelineManager(GraphicsPipelineManager &&other) = delete; // move-ctor;
	
		GraphicsPipelineManager & operator=(const GraphicsPipelineManager &other) = delete; // copy-assign op
		GraphicsPipelineManager & operator=(GraphicsPipelineManager &&other) = delete; // move-assign op

        /**
         * Creates a Graphics Pipeline based on the set shader stages in the Config Struct.
         * This function is wrapped in /src/vkcv/Core.cpp by Core::createGraphicsPipeline(const PipelineConfig &config).
         * Therefore the passManager is filled already by the overall context of an application.
         * On application level it is necessary first to fill a PipelineConfig Struct.
         * @param config Hands over all needed information for pipeline creation.
         * @param passManager Hands over the corresponding render pass.
         * @param descriptorManager Hands over the corresponding descriptor set layouts
         * @return A Handler to the created Graphics Pipeline Object.
         */
		GraphicsPipelineHandle createPipeline(const GraphicsPipelineConfig &config,
											  const PassManager& passManager,
											  const DescriptorManager& descriptorManager);

        /**
         * Returns a vk::Pipeline object by handle.
         * @param handle Directing to the requested pipeline.
         * @return vk::Pipeline.
         */
        [[nodiscard]]
        vk::Pipeline getVkPipeline(const GraphicsPipelineHandle &handle) const;

        /**
         * Returns a vk::PipelineLayout object by handle.
         * @param handle Directing to the requested pipeline.
         * @return vk::PipelineLayout.
         */
        [[nodiscard]]
        vk::PipelineLayout getVkPipelineLayout(const GraphicsPipelineHandle &handle) const;

        /**
         * Returns the corresponding Pipeline Config Struct of a pipeline object directed by the given Handler.
         * @param handle Directing to the requested pipeline.
         * @return Pipeline Config Struct
         */
        [[nodiscard]]
        const GraphicsPipelineConfig &getPipelineConfig(const GraphicsPipelineHandle &handle) const;

    private:
        struct GraphicsPipeline {
            vk::Pipeline m_handle;
            vk::PipelineLayout m_layout;
			GraphicsPipelineConfig m_config;
        };

        vk::Device                      m_Device;
        vk::PhysicalDevice              m_physicalDevice; // needed to get infos to configure conservative rasterization
        std::vector<GraphicsPipeline>   m_Pipelines;

        void destroyPipelineById(uint64_t id);

    };
}
