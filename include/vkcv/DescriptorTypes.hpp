#pragma once
/**
 * @authors Artur Wasmut, Tobias Frisch, Simeon Hermann, Alexander Gauggel, Vanessa Karolek
 * @file vkcv/DescriptorConfig.hpp
 * @brief Enum classes to handle descriptor types.
 */
 
#include <vulkan/vulkan.hpp>
 
namespace vkcv {
	
	/**
	 * @brief Enum class to specify the type of a descriptor set binding.
	 */
	enum class DescriptorType {
		UNIFORM_BUFFER,
		STORAGE_BUFFER,
		SAMPLER,
		IMAGE_SAMPLED,
		IMAGE_STORAGE,
		UNIFORM_BUFFER_DYNAMIC,
		STORAGE_BUFFER_DYNAMIC,
		ACCELERATION_STRUCTURE_KHR
	};
	
	/**
     * @brief Converts the descriptor type from the frameworks enumeration
     * to the Vulkan type specifier.
     *
     * @param[in] type Descriptor type
     * @return Vulkan descriptor type
     */
	constexpr vk::DescriptorType getVkDescriptorType(DescriptorType type) noexcept {
		switch (type)
		{
			case DescriptorType::UNIFORM_BUFFER:
				return vk::DescriptorType::eUniformBuffer;
			case DescriptorType::UNIFORM_BUFFER_DYNAMIC:
				return vk::DescriptorType::eUniformBufferDynamic;
			case DescriptorType::STORAGE_BUFFER:
				return vk::DescriptorType::eStorageBuffer;
			case DescriptorType::STORAGE_BUFFER_DYNAMIC:
				return vk::DescriptorType::eStorageBufferDynamic;
			case DescriptorType::SAMPLER:
				return vk::DescriptorType::eSampler;
			case DescriptorType::IMAGE_SAMPLED:
				return vk::DescriptorType::eSampledImage;
			case DescriptorType::IMAGE_STORAGE:
				return vk::DescriptorType::eStorageImage;
			case DescriptorType::ACCELERATION_STRUCTURE_KHR:
				return vk::DescriptorType::eAccelerationStructureKHR;
			default:
				return vk::DescriptorType::eMutableVALVE;
		}
	}

}
