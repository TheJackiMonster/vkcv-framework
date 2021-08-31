/**
 * @authors Artur Wasmut, Susanne D�tsch, Simeon Hermann
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

	    DescriptorSetLayoutHandle createDescriptorSetLayout(const std::unordered_map<uint32_t, DescriptorBinding> &setBindingsMap);
        DescriptorSetHandle createDescriptorSet(const DescriptorSetLayoutHandle &setLayoutHandle);

		void writeDescriptorSet(
			const DescriptorSetHandle	&handle,
			const DescriptorWrites  &writes,
			const ImageManager      &imageManager,
			const BufferManager     &bufferManager,
			const SamplerManager    &samplerManager);

		[[nodiscard]]
		DescriptorSetLayout getDescriptorSetLayout(const DescriptorSetLayoutHandle handle) const;
		[[nodiscard]]
		DescriptorSet getDescriptorSet(const DescriptorSetHandle handle) const;

	private:
		vk::Device m_Device;
		std::vector<vk::DescriptorPool>	m_Pools;
		std::vector<vk::DescriptorPoolSize> m_PoolSizes;
		vk::DescriptorPoolCreateInfo m_PoolInfo;

		/**
        * Contains all the descriptor set layout descriptions
        * that were requested by the user in calls of createDescriptorSetLayout.
        */
        std::vector<DescriptorSetLayout> m_DescriptorSetLayouts;

        /**
		* Contains all the descriptor sets that were created by the user in calls of createDescriptorSet.
		*/
        std::vector<DescriptorSet> m_DescriptorSets;

		/**
		* Destroys a specific descriptor set
		* @param[in] the DescriptorSetHandle
		*/
		void destroyDescriptorSetById(uint64_t id);

		/**
        * Destroys a specific descriptor set LAYOUT (not the set)
        * @param[in] the DescriptorSetLayoutHandle
        */
		void destroyDescriptorSetLayoutById(uint64_t id);

		/**
		* creates a descriptor pool based on the poolSizes and poolInfo defined in the constructor
		* is called initially in the constructor and then every time the pool runs out memory
		* @return a DescriptorPool object
		*/
		vk::DescriptorPool allocateDescriptorPool();
		
	};
}