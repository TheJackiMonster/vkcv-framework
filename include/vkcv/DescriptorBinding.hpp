#pragma once
/**
 * @authors Artur Wasmut, Tobias Frisch, Simeon Hermann, Alexander Gauggel, Vanessa Karolek
 * @file vkcv/DescriptorConfig.hpp
 * @brief Structures to handle descriptor bindings.
 */

#include "Container.hpp"
#include "DescriptorTypes.hpp"
#include "ShaderStage.hpp"

namespace vkcv {

	/**
	 * @brief Structure to store details from a descriptor binding.
	 */
	struct DescriptorBinding {
		uint32_t bindingID;
		DescriptorType descriptorType;
		uint32_t descriptorCount;
		ShaderStages shaderStages;
		bool variableCount;
		bool partialBinding;

		bool operator==(const DescriptorBinding &other) const;
	};

	typedef Dictionary<uint32_t, DescriptorBinding> DescriptorBindings;

} // namespace vkcv
