#pragma once
/**
 * @authors Artur Wasmut, Susanne D�tsch, Simeon Hermann, Tobias Frisch
 * @file src/vkcv/DescriptorManager.cpp
 * @brief Creation and handling of descriptor sets and the respective descriptor pools.
 */
#include <vulkan/vulkan.hpp>

#include "vkcv/DescriptorBinding.hpp"
#include "vkcv/DescriptorWrites.hpp"

#include "BufferManager.hpp"
#include "DescriptorSetLayoutManager.hpp"
#include "HandleManager.hpp"
#include "ImageManager.hpp"
#include "SamplerManager.hpp"

namespace vkcv {

	/**
	 * @brief Structure to store details about a descriptor set.
	 */
	struct DescriptorSetEntry {
		vk::DescriptorSet vulkanHandle;
		DescriptorSetLayoutHandle setLayoutHandle;
		size_t poolIndex;
	};

	/**
	 * @brief Class to manage descriptor sets.
	 */
	class DescriptorSetManager : public HandleManager<DescriptorSetEntry, DescriptorSetHandle> {
		friend class Core;

	private:
		DescriptorSetLayoutManager* m_DescriptorSetLayoutManager;

		std::vector<vk::DescriptorPool> m_Pools;
		std::vector<vk::DescriptorPoolSize> m_PoolSizes;
		vk::DescriptorPoolCreateInfo m_PoolInfo;

		bool init(Core &core, DescriptorSetLayoutManager &descriptorSetLayoutManager);

		[[nodiscard]] uint64_t getIdFrom(const DescriptorSetHandle &handle) const override;

		[[nodiscard]] DescriptorSetHandle createById(uint64_t id,
													 const HandleDestroyFunction &destroy) override;

		/**
		 * Destroys and deallocates descriptor set represented by a given
		 * descriptor set handle id.
		 *
		 * @param id Descriptor set handle id
		 */
		void destroyById(uint64_t id) override;

		/**
		 * @brief Creates a descriptor pool based on the poolSizes and poolInfo defined in the
		 * constructor is called initially in the constructor and then every time the pool runs
		 * out memory.
		 *
		 * @return whether a DescriptorPool object could be created
		 */
		bool allocateDescriptorPool();

	public:
		/**
		 * @brief Constructor of the descriptor set manager
		 */
		DescriptorSetManager() noexcept;

		/**
		 * @brief Destructor of the descriptor set manager
		 */
		~DescriptorSetManager() noexcept override;

		/**
		 * @brief Creates a descriptor set using a given descriptor set layout.
		 *
		 * @param[in] layout Handle of descriptor set layout
		 * @return Handle of descriptor set
		 */
		[[nodiscard]] DescriptorSetHandle
		createDescriptorSet(const DescriptorSetLayoutHandle &layout);

		/**
		 * @brief Writes to a descriptor set using writes and all required managers.
		 *
		 * @param[in] handle Handle of descriptor set
		 * @param[in] writes Descriptor set writes
		 * @param[in] imageManager Image manager
		 * @param[in] bufferManager Buffer manager
		 * @param[in] samplerManager Sampler manager
		 */
		void writeDescriptorSet(const DescriptorSetHandle &handle, const DescriptorWrites &writes,
								const ImageManager &imageManager,
								const BufferManager &bufferManager,
								const SamplerManager &samplerManager);

		[[nodiscard]] const DescriptorSetEntry &
		getDescriptorSet(const DescriptorSetHandle &handle) const;
	};

} // namespace vkcv
