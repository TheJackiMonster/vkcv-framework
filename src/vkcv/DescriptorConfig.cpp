#include "vkcv/DescriptorConfig.hpp"

namespace vkcv
{
	DescriptorBinding::DescriptorBinding(
		uint32_t bindingID,
		DescriptorType descriptorType,
		uint32_t descriptorCount,
		ShaderStage shaderStages,
		bool variableCount) noexcept:
		bindingID(bindingID),
		descriptorType(descriptorType),
		descriptorCount(descriptorCount),
		shaderStages(shaderStages),
		variableCount(variableCount){}

    bool DescriptorBinding::operator==(const DescriptorBinding &other) const
    {
	    return (this->bindingID == other.bindingID) &&
	           (this->descriptorType == other.descriptorType) &&
	           (this->descriptorCount == other.descriptorCount) &&
	           (this->shaderStages == other.shaderStages) &&
	           (this->variableCount == other.variableCount);
    }
}
