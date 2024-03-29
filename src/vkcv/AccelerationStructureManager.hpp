#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/AccelerationStructureManager.hpp
 * @brief Manager to handle acceleration structure operations.
 */

#include <memory>

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "BufferManager.hpp"

#include "vkcv/Container.hpp"
#include "vkcv/Handles.hpp"
#include "vkcv/GeometryData.hpp"

namespace vkcv {
	
	struct AccelerationStructureEntry {
		vk::AccelerationStructureTypeKHR m_type;
		vk::DeviceSize m_size;
		vk::AccelerationStructureKHR m_accelerationStructure;
		Vector<AccelerationStructureHandle> m_children;
		BufferHandle m_storageBuffer;
	};
	
	/**
	 * @brief Class to manage the creation, destruction, allocation
	 * and filling of buffers.
	 */
	class AccelerationStructureManager :
			public HandleManager<AccelerationStructureEntry, AccelerationStructureHandle> {
		friend class Core;
	
	private:
		BufferManager* m_bufferManager;
		
		bool init(Core &core) override;
		bool init(Core &core, BufferManager &bufferManager);
		
		[[nodiscard]] uint64_t getIdFrom(const AccelerationStructureHandle &handle) const override;
		
		[[nodiscard]] AccelerationStructureHandle createById(
				uint64_t id,
				const HandleDestroyFunction &destroy) override;
		
		/**
		 * Destroys and deallocates image represented by a given
		 * image handle id.
		 *
		 * @param id Image handle id
		 */
		void destroyById(uint64_t id) override;
		
		[[nodiscard]] const BufferManager &getBufferManager() const;
		
		[[nodiscard]] BufferManager &getBufferManager();
		
	public:
		AccelerationStructureManager() noexcept;
		
		~AccelerationStructureManager() noexcept override;
		
		[[nodiscard]] vk::AccelerationStructureKHR getVulkanAccelerationStructure(
				const AccelerationStructureHandle &handle) const;
		
		[[nodiscard]] vk::Buffer getVulkanBuffer(const AccelerationStructureHandle &handle) const;
		
		/**
		 * @brief Returns the Vulkan device address of an acceleration
		 * structure represented by a given acceleration structure
		 * handle id.
		 *
		 * @param[in] handle Acceleration structure handle
		 * @return Vulkan device address
		 */
		[[nodiscard]] vk::DeviceAddress getAccelerationStructureDeviceAddress(
				const AccelerationStructureHandle &handle) const;
		
		[[nodiscard]] AccelerationStructureHandle createAccelerationStructure(
				const Vector<GeometryData> &geometryData,
				const BufferHandle &transformBuffer,
				bool compaction);
		
		[[nodiscard]] AccelerationStructureHandle createAccelerationStructure(
				const Vector<AccelerationStructureHandle> &accelerationStructures);
		
	};
	
}