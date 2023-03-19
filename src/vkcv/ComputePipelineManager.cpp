#include "ComputePipelineManager.hpp"

#include "vkcv/Core.hpp"

namespace vkcv {

	uint64_t ComputePipelineManager::getIdFrom(const ComputePipelineHandle &handle) const {
		return handle.getId();
	}

	ComputePipelineHandle ComputePipelineManager::createById(uint64_t id,
															 const HandleDestroyFunction &destroy) {
		return ComputePipelineHandle(id, destroy);
	}

	void ComputePipelineManager::destroyById(uint64_t id) {
		auto &pipeline = getById(id);

		if (pipeline.m_handle) {
			getCore().getContext().getDevice().destroy(pipeline.m_handle);
			pipeline.m_handle = nullptr;
		}

		if (pipeline.m_layout) {
			getCore().getContext().getDevice().destroy(pipeline.m_layout);
			pipeline.m_layout = nullptr;
		}
	}

	ComputePipelineManager::ComputePipelineManager() noexcept :
		HandleManager<ComputePipelineEntry, ComputePipelineHandle>() {}

	vk::Result ComputePipelineManager::createShaderModule(vk::ShaderModule &module,
														  const ShaderProgram &shaderProgram,
														  const ShaderStage stage) {
		Vector<uint32_t> code = shaderProgram.getShaderBinary(stage);
		vk::ShaderModuleCreateInfo moduleInfo({}, code.size() * sizeof(uint32_t), code.data());
		return getCore().getContext().getDevice().createShaderModule(&moduleInfo, nullptr, &module);
	}

	ComputePipelineManager::~ComputePipelineManager() noexcept {
		clear();
	}

	vk::Pipeline ComputePipelineManager::getVkPipeline(const ComputePipelineHandle &handle) const {
		auto &pipeline = (*this) [handle];
		return pipeline.m_handle;
	}

	vk::PipelineLayout
	ComputePipelineManager::getVkPipelineLayout(const ComputePipelineHandle &handle) const {
		auto &pipeline = (*this) [handle];
		return pipeline.m_layout;
	}

	ComputePipelineHandle ComputePipelineManager::createComputePipeline(
		const ShaderProgram &shaderProgram,
		const Vector<vk::DescriptorSetLayout> &descriptorSetLayouts) {
		// Temporally handing over the Shader Program instead of a pipeline config
		vk::ShaderModule computeModule {};
		if (createShaderModule(computeModule, shaderProgram, ShaderStage::COMPUTE)
			!= vk::Result::eSuccess)
			return {};

		vk::PipelineShaderStageCreateInfo pipelineComputeShaderStageInfo(
			{}, vk::ShaderStageFlagBits::eCompute, computeModule, "main", nullptr);

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo({}, descriptorSetLayouts);

		const size_t pushConstantsSize = shaderProgram.getPushConstantsSize();
		vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eCompute, 0,
												pushConstantsSize);
		if (pushConstantsSize > 0) {
			pipelineLayoutCreateInfo.setPushConstantRangeCount(1);
			pipelineLayoutCreateInfo.setPPushConstantRanges(&pushConstantRange);
		}

		vk::PipelineLayout vkPipelineLayout {};
		if (getCore().getContext().getDevice().createPipelineLayout(&pipelineLayoutCreateInfo,
																	nullptr, &vkPipelineLayout)
			!= vk::Result::eSuccess) {
			getCore().getContext().getDevice().destroy(computeModule);
			return {};
		}

		vk::ComputePipelineCreateInfo computePipelineCreateInfo {};
		computePipelineCreateInfo.stage = pipelineComputeShaderStageInfo;
		computePipelineCreateInfo.layout = vkPipelineLayout;

		vk::Pipeline vkPipeline;
		if (getCore().getContext().getDevice().createComputePipelines(
				nullptr, 1, &computePipelineCreateInfo, nullptr, &vkPipeline)
			!= vk::Result::eSuccess) {
			getCore().getContext().getDevice().destroy(computeModule);
			return ComputePipelineHandle();
		}

		getCore().getContext().getDevice().destroy(computeModule);
		return add({ vkPipeline, vkPipelineLayout });
	}

} // namespace vkcv