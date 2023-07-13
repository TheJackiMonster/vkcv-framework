#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/DescriptorUsage.hpp
 * @brief Structures to handle descriptor usages.
 */

#include "Container.hpp"
#include "Handles.hpp"

namespace vkcv {

	/**
	 * @brief Structure to configure a descriptor set usage.
	 */
	struct DescriptorSetUsage {
		uint32_t location;
		DescriptorSetHandle descriptorSet;
		Vector<uint32_t> dynamicOffsets;
	};

	DescriptorSetUsage useDescriptorSet(uint32_t location, const DescriptorSetHandle &descriptorSet,
										const Vector<uint32_t> &dynamicOffsets = {});

} // namespace vkcv
