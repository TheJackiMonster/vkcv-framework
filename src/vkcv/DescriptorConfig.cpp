#include "vkcv/DescriptorConfig.hpp"

namespace vkcv {

    DescriptorSetUsage::DescriptorSetUsage(uint32_t setLocation, DescriptorSetHandle handle) noexcept 
    : setLocation(setLocation), handle(handle) {}

	DescriptorBinding::DescriptorBinding(
		DescriptorType descriptorType,
		uint32_t descriptorCount,
		ShaderStage shaderStage) noexcept
		:
		descriptorType(descriptorType),
		descriptorCount(descriptorCount),
		shaderStage(shaderStage) {}
	
}
