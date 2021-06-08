#include "vkcv/DescriptorConfig.hpp"

namespace vkcv {
	DescriptorBinding::DescriptorBinding(
		DescriptorType descriptorType,
		uint32_t descriptorCount,
		ShaderStage shaderStage) noexcept
		:
		descriptorType(descriptorType),
		descriptorCount(descriptorCount),
		shaderStage(shaderStage) {}
	
}
