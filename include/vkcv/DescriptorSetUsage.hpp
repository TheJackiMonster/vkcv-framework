#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/DescriptorUsage.hpp
 * @brief Structures to handle descriptor usages.
 */

#include <vector>

#include "Handles.hpp"

namespace vkcv {
	
	/**
	 * @brief Structure to configure a descriptor set usage.
	 */
	struct DescriptorSetUsage {
		uint32_t location;
		DescriptorSetHandle descriptorSet;
		std::vector<uint32_t> dynamicOffsets;
	};
	
	DescriptorSetUsage useDescriptorSet(uint32_t location,
										const DescriptorSetHandle &descriptorSet,
										const std::vector<uint32_t> &dynamicOffsets = {});
	
}
