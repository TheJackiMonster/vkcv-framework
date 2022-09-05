
#include "vkcv/DescriptorSetUsage.hpp"

namespace vkcv {
	
	DescriptorSetUsage useDescriptorSet(uint32_t location, const DescriptorSetHandle &descriptorSet,
										const std::vector<uint32_t> &dynamicOffsets) {
		DescriptorSetUsage usage (location, descriptorSet, dynamicOffsets);
		return usage;
	}
	
}
