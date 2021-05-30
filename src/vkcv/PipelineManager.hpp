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
    private:
        vk::Device m_Device;
        std::vector<vk::Pipeline> m_Pipelines;
        std::vector<vk::PipelineLayout> m_PipelineLayouts;
        uint64_t m_NextPipelineId;
    public:
        PipelineManager() = delete; // no default ctor
        explicit PipelineManager(vk::Device device) noexcept; // ctor
        ~PipelineManager() noexcept; // dtor

        PipelineManager(const PipelineManager &other) = delete; // copy-ctor
        PipelineManager(PipelineManager &&other) = delete; // move-ctor;

        PipelineManager & operator=(const PipelineManager &other) = delete; // copy-assign op
        PipelineManager & operator=(PipelineManager &&other) = delete; // move-assign op

        PipelineHandle createPipeline(const PipelineConfig &config, PassManager& passManager);

        [[nodiscard]]
        vk::Pipeline getVkPipeline(const PipelineHandle &handle) const;
        [[nodiscard]]
        vk::PipelineLayout getVkPipelineLayout(const PipelineHandle &handle) const;
    };
}
