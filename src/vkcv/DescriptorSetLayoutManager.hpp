#pragma once
/**
 * @authors Artur Wasmut, Susanne D�tsch, Simeon Hermann, Tobias Frisch
 * @file src/vkcv/DescriptorManager.cpp
 * @brief Creation and handling of descriptor set layouts.
 */
#include <vulkan/vulkan.hpp>

#include "vkcv/DescriptorBinding.hpp"

#include "HandleManager.hpp"

namespace vkcv {
	
	/**
	 * @brief Structure to store details about a descriptor set layout.
	 */
	struct DescriptorSetLayoutEntry {
		vk::DescriptorSetLayout vulkanHandle;
		DescriptorBindings descriptorBindings;
		size_t layoutUsageCount;
	};
	
	/**
	 * @brief Class to manage descriptor set layouts.
	 */
	class DescriptorSetLayoutManager : public HandleManager<DescriptorSetLayoutEntry, DescriptorSetLayoutHandle> {
		friend class Core;
	private:
		[[nodiscard]]
		uint64_t getIdFrom(const DescriptorSetLayoutHandle& handle) const override;
		
		[[nodiscard]]
		DescriptorSetLayoutHandle createById(uint64_t id, const HandleDestroyFunction& destroy) override;
		
		/**
		 * Destroys and deallocates descriptor set layout represented by a given
		 * descriptor set layout handle id.
		 *
		 * @param id Descriptor set layout handle id
		 */
		void destroyById(uint64_t id) override;
	
	public:
		/**
		 * @brief Constructor of the descriptor set layout manager
		 */
		DescriptorSetLayoutManager() noexcept;
		
		/**
		 * @brief Destructor of the descriptor set layout manager
		 */
		~DescriptorSetLayoutManager() noexcept override;
		
		/**
		 * @brief Creates a descriptor set layout with given descriptor bindings
		 * or returns a matching handle.
		 *
		 * @param[in] bindings Descriptor bindings
		 * @return Handle of descriptor set layout
		 */
		[[nodiscard]]
		DescriptorSetLayoutHandle createDescriptorSetLayout(const DescriptorBindings &bindings);
		
		[[nodiscard]]
		const DescriptorSetLayoutEntry& getDescriptorSetLayout(const DescriptorSetLayoutHandle& handle) const;
		
	};
	
}
