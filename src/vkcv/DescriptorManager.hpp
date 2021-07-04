/**
 * @authors Artur Wasmut, Susanne Dötsch, Simeon Hermann
 * @file src/vkcv/DescriptorManager.cpp
 * @brief Creation and handling of descriptor sets and the respective descriptor pools
 */
#include <vulkan/vulkan.hpp>

#include "vkcv/Handles.hpp"
#include "vkcv/DescriptorConfig.hpp"
#include "vkcv/DescriptorWrites.hpp"

#include "ImageManager.hpp"
#include "vkcv/BufferManager.hpp"
#include "SamplerManager.hpp"

namespace vkcv
{
	class DescriptorManager
	{
	public:
	    explicit DescriptorManager(vk::Device device) noexcept;
	    ~DescriptorManager() noexcept;

        DescriptorSetHandle createDescriptorSet(const std::unordered_map<uint32_t, DescriptorBinding> &descriptorBindings);

		void writeDescriptorSet(
			const DescriptorSetHandle	&handle,
			const DescriptorWrites  &writes,
			const ImageManager      &imageManager,
			const BufferManager     &bufferManager,
			const SamplerManager    &samplerManager);

		[[nodiscard]]
		DescriptorSet getDescriptorSet(const DescriptorSetHandle handle) const;

	private:
		vk::Device m_Device;
		std::vector<vk::DescriptorPool>	m_Pools;
		std::vector<vk::DescriptorPoolSize> m_PoolSizes;
		vk::DescriptorPoolCreateInfo m_PoolInfo;


		/**
		* Contains all the resource descriptions that were requested by the user in calls of createResourceDescription.
		*/
        std::vector<DescriptorSet> m_DescriptorSets;
		
		/**
		* Converts the flags of the descriptor types from VulkanCV (vkcv) to Vulkan (vk).
		* @param[in] vkcv flag of the DescriptorType (see DescriptorConfig.hpp)
		* @return vk flag of the DescriptorType
		*/
		static vk::DescriptorType convertDescriptorTypeFlag(DescriptorType type);
		
		/**
		* Destroys a specific resource description
		* @param[in] the handle id of the respective resource description
		*/
		void destroyDescriptorSetById(uint64_t id);

		/**
		* creates a descriptor pool based on the poolSizes and poolInfo defined in the constructor
		* is called initially in the constructor and then every time the pool runs out memory
		* @return a DescriptorPool object
		*/
		vk::DescriptorPool allocateDescriptorPool();
		
	};
}