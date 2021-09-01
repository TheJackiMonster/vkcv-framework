#include "ComputePipelineManager.hpp"

namespace vkcv
{

    ComputePipelineManager::ComputePipelineManager(vk::Device device) noexcept :
            m_Device{device},
            m_Pipelines{}
    {}

    ComputePipelineManager::~ComputePipelineManager() noexcept
    {
        for (uint64_t id = 0; id < m_Pipelines.size(); id++) {
            destroyPipelineById(id);
        }
    }

    vk::Pipeline ComputePipelineManager::getVkPipeline(const ComputePipelineHandle &handle) const
    {
        const uint64_t id = handle.getId();

        if (id >= m_Pipelines.size()) {
            return nullptr;
        }

        auto& pipeline = m_Pipelines[id];

        return pipeline.m_handle;
    }

    vk::PipelineLayout ComputePipelineManager::getVkPipelineLayout(const ComputePipelineHandle &handle) const
    {
        const uint64_t id = handle.getId();

        if (id >= m_Pipelines.size()) {
            return nullptr;
        }

        auto& pipeline = m_Pipelines[id];

        return pipeline.m_layout;
    }

    ComputePipelineHandle ComputePipelineManager::createComputePipeline(
            const ShaderProgram &shaderProgram,
            const std::vector <vk::DescriptorSetLayout> &descriptorSetLayouts) {

        // Temporally handing over the Shader Program instead of a pipeline config
        vk::ShaderModule computeModule{};
        if (createShaderModule(computeModule, shaderProgram, ShaderStage::COMPUTE) != vk::Result::eSuccess)
            return ComputePipelineHandle();

        vk::PipelineShaderStageCreateInfo pipelineComputeShaderStageInfo(
                {},
                vk::ShaderStageFlagBits::eCompute,
                computeModule,
                "main",
                nullptr
        );

        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo({}, descriptorSetLayouts);

        const size_t pushConstantSize = shaderProgram.getPushConstantSize();
        vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eCompute, 0, pushConstantSize);
        if (pushConstantSize > 0) {
            pipelineLayoutCreateInfo.setPushConstantRangeCount(1);
            pipelineLayoutCreateInfo.setPPushConstantRanges(&pushConstantRange);
        }

        vk::PipelineLayout vkPipelineLayout{};
        if (m_Device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &vkPipelineLayout) !=
            vk::Result::eSuccess) {
            m_Device.destroy(computeModule);
            return ComputePipelineHandle();
        }

        vk::ComputePipelineCreateInfo computePipelineCreateInfo{};
        computePipelineCreateInfo.stage = pipelineComputeShaderStageInfo;
        computePipelineCreateInfo.layout = vkPipelineLayout;

        vk::Pipeline vkPipeline;
        if (m_Device.createComputePipelines(nullptr, 1, &computePipelineCreateInfo, nullptr, &vkPipeline) !=
            vk::Result::eSuccess) {
            m_Device.destroy(computeModule);
            return ComputePipelineHandle();
        }

        m_Device.destroy(computeModule);

        const uint64_t id = m_Pipelines.size();
        m_Pipelines.push_back({vkPipeline, vkPipelineLayout});

        return ComputePipelineHandle(id, [&](uint64_t id) { destroyPipelineById(id); });
    }

    void vkcv::ComputePipelineManager::destroyPipelineById(uint64_t id) {
        if (id >= m_Pipelines.size()) {
            return;
        }

        auto& pipeline = m_Pipelines[id];

        if (pipeline.m_handle) {
            m_Device.destroy(pipeline.m_handle);
            pipeline.m_handle = nullptr;
        }

        if (pipeline.m_layout) {
            m_Device.destroy(pipeline.m_layout);
            pipeline.m_layout = nullptr;
        }
    }

    vk::Result ComputePipelineManager::createShaderModule(vk::ShaderModule &module, const ShaderProgram &shaderProgram, const ShaderStage stage)
    {
        std::vector<char> code = shaderProgram.getShader(stage).shaderCode;
        vk::ShaderModuleCreateInfo moduleInfo({}, code.size(), reinterpret_cast<uint32_t*>(code.data()));
        return m_Device.createShaderModule(&moduleInfo, nullptr, &module);
    }
}