#include "vkcv/DescriptorBinding.hpp"

namespace vkcv {
	
    bool DescriptorBinding::operator==(const DescriptorBinding &other) const
    {
	    return (this->bindingID == other.bindingID) &&
	           (this->descriptorType == other.descriptorType) &&
	           (this->descriptorCount == other.descriptorCount) &&
	           (this->shaderStages == other.shaderStages) &&
	           (this->variableCount == other.variableCount);
    }
	
}
