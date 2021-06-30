#include "vkcv/DescriptorConfig.hpp"

namespace vkcv {
	DescriptorBinding::DescriptorBinding(
		uint32_t bindingID,
		DescriptorType descriptorType,
		uint32_t descriptorCount,
		vk::ShaderStageFlags shaderStages) noexcept
		:
		bindingID(bindingID),
		descriptorType(descriptorType),
		descriptorCount(descriptorCount),
		shaderStages(shaderStages) {}
	
}
