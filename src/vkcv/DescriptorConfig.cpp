#include "vkcv/DescriptorConfig.hpp"

namespace vkcv {
	DescriptorBinding::DescriptorBinding(
		uint32_t bindingID,
		DescriptorType descriptorType,
		uint32_t descriptorCount,
		ShaderStage shaderStage,
		bool variableCount) noexcept
		:
		bindingID(bindingID),
		descriptorType(descriptorType),
		descriptorCount(descriptorCount),
		shaderStage(shaderStage),
		variableCount(variableCount){}
	
}
