/**
 * @author Tobias Frisch
 * @file vkcv/BufferManager.cpp
 */

#include "BufferManager.hpp"
#include "vkcv/Core.hpp"
#include <vkcv/Logger.hpp>

#include <limits>
#include <numeric>

namespace vkcv {

	bool BufferManager::init(Core &core) {
		if (!HandleManager<BufferEntry, BufferHandle>::init(core)) {
			return false;
		}
		
		const vma::Allocator &allocator = getCore().getContext().getAllocator();
		const auto& memoryProperties = allocator.getMemoryProperties();
		const auto& heaps = memoryProperties->memoryHeaps;
		
		std::vector<vk::MemoryPropertyFlags> heapMemoryFlags;
		heapMemoryFlags.resize(heaps.size());
		
		for (const auto& type : memoryProperties->memoryTypes) {
			if (type.heapIndex >= heaps.size()) {
				continue;
			}
			
			heapMemoryFlags[type.heapIndex] |= type.propertyFlags;
		}
		
		vk::DeviceSize maxDeviceHeapSize = 0;
		uint32_t deviceHeapIndex = 0;
		
		for (uint32_t i = 0; i < heaps.size(); i++) {
			if (!(heaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal)) {
				continue;
			}
			
			if (!(heapMemoryFlags[i] & vk::MemoryPropertyFlagBits::eDeviceLocal)) {
				continue;
			}
			
			if (heaps[i].size < maxDeviceHeapSize) {
				continue;
			}
			
			maxDeviceHeapSize = heaps[i].size;
			deviceHeapIndex = i;
		}
		
		if (heapMemoryFlags[deviceHeapIndex] & vk::MemoryPropertyFlagBits::eHostVisible) {
			m_resizableBar = true;
		} else {
			m_resizableBar = false;
		}
		
		m_shaderDeviceAddress = getCore().getContext().getFeatureManager().checkFeatures<
		        vk::PhysicalDeviceBufferDeviceAddressFeatures
		>(
				vk::StructureType::ePhysicalDeviceBufferDeviceAddressFeatures,
				[](const vk::PhysicalDeviceBufferDeviceAddressFeatures &features) {
					return features.bufferDeviceAddress;
				}
		);

		m_stagingBuffer = createBuffer(
				TypeGuard(1),
				BufferType::STAGING,
				BufferMemoryType::HOST_VISIBLE,
				1024 * 1024,
				false
		);

		return true;
	}

	uint64_t BufferManager::getIdFrom(const BufferHandle &handle) const {
		return handle.getId();
	}

	BufferHandle BufferManager::createById(uint64_t id, const HandleDestroyFunction &destroy) {
		return BufferHandle(id, destroy);
	}

	void BufferManager::destroyById(uint64_t id) {
		auto &buffer = getById(id);

		const vma::Allocator &allocator = getCore().getContext().getAllocator();

		if (buffer.m_handle) {
			if (buffer.m_mapping) {
				allocator.unmapMemory(buffer.m_allocation);
			}
			
			allocator.destroyBuffer(buffer.m_handle, buffer.m_allocation);

			buffer.m_handle = nullptr;
			buffer.m_allocation = nullptr;
		}
	}

	BufferManager::BufferManager() noexcept :
		HandleManager<BufferEntry, BufferHandle>(),
		m_resizableBar(false),
		m_shaderDeviceAddress(false),
		m_stagingBuffer(BufferHandle()) {}

	BufferManager::~BufferManager() noexcept {
		clear();
	}

	BufferHandle BufferManager::createBuffer(const TypeGuard &typeGuard, BufferType type,
											 BufferMemoryType memoryType, size_t size,
											 bool readable, size_t alignment) {
		vk::BufferCreateFlags createFlags;
		vk::BufferUsageFlags usageFlags;

		switch (type) {
		case BufferType::VERTEX:
			usageFlags = vk::BufferUsageFlagBits::eVertexBuffer
						| vk::BufferUsageFlagBits::eStorageBuffer
						| vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
			break;
		case BufferType::UNIFORM:
			usageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
			break;
		case BufferType::STORAGE:
			usageFlags = vk::BufferUsageFlagBits::eStorageBuffer;
			break;
		case BufferType::STAGING:
			usageFlags = vk::BufferUsageFlagBits::eTransferSrc
						| vk::BufferUsageFlagBits::eTransferDst;
			break;
		case BufferType::INDEX:
			usageFlags = vk::BufferUsageFlagBits::eIndexBuffer
						| vk::BufferUsageFlagBits::eStorageBuffer
						| vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
			break;
		case BufferType::INDIRECT:
			usageFlags = vk::BufferUsageFlagBits::eStorageBuffer
						| vk::BufferUsageFlagBits::eIndirectBuffer;
			break;
		case BufferType::SHADER_BINDING:
			usageFlags = vk::BufferUsageFlagBits::eShaderBindingTableKHR;
			break;
		case BufferType::ACCELERATION_STRUCTURE_INPUT:
			usageFlags = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
			break;
		case BufferType::ACCELERATION_STRUCTURE_STORAGE:
			usageFlags = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
						 | vk::BufferUsageFlagBits::eStorageBuffer;
			break;
		default:
			vkcv_log(LogLevel::WARNING, "Unknown buffer type");
			break;
		}

		if (memoryType == BufferMemoryType::DEVICE_LOCAL) {
			usageFlags |= vk::BufferUsageFlagBits::eTransferDst;
		}

		if (readable) {
			usageFlags |= vk::BufferUsageFlagBits::eTransferSrc;
		}
		
		if (m_shaderDeviceAddress) {
			usageFlags |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
		}
		
		if (usageFlags & vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR) {
			alignment = (alignment > 0? std::lcm(alignment, 16) : 16);
		}

		const vma::Allocator &allocator = getCore().getContext().getAllocator();

		vma::MemoryUsage memoryUsage;
		bool mappable;

		switch (memoryType) {
		case BufferMemoryType::DEVICE_LOCAL:
			memoryUsage = vma::MemoryUsage::eAutoPreferDevice;
			mappable = false;
			break;
		case BufferMemoryType::HOST_VISIBLE:
			memoryUsage = vma::MemoryUsage::eAutoPreferHost;
			mappable = true;
			break;
		default:
			vkcv_log(LogLevel::WARNING, "Unknown buffer memory type");
			memoryUsage = vma::MemoryUsage::eUnknown;
			mappable = false;
			break;
		}
		
		vma::AllocationCreateFlags allocationCreateFlags;
		
		if (mappable) {
			if (type == vkcv::BufferType::STAGING) {
				allocationCreateFlags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;
			} else {
				allocationCreateFlags = vma::AllocationCreateFlagBits::eHostAccessRandom;
			}
		} else
		if ((m_resizableBar) && (memoryType == BufferMemoryType::DEVICE_LOCAL)) {
			allocationCreateFlags = vma::AllocationCreateFlagBits::eHostAccessAllowTransferInstead
									| vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;
		}

		const auto bufferAllocation = allocator.createBufferWithAlignment(
			vk::BufferCreateInfo(createFlags, size, usageFlags),
			vma::AllocationCreateInfo(
					allocationCreateFlags,
					memoryUsage,
					vk::MemoryPropertyFlags(),
					vk::MemoryPropertyFlags(),
					0,
					vma::Pool(),
					nullptr
			),
			static_cast<vk::DeviceSize>(alignment)
		);

		const vk::Buffer buffer = bufferAllocation.first;
		const vma::Allocation allocation = bufferAllocation.second;
		
		const vk::MemoryPropertyFlags finalMemoryFlags = allocator.getAllocationMemoryProperties(
				allocation
		);
		
		if (vk::MemoryPropertyFlagBits::eHostVisible & finalMemoryFlags) {
			mappable = true;
		}

		return add({
			typeGuard,
			type,
			memoryType,
			size,
			buffer,
			allocation,
			readable,
			mappable,
			nullptr,
			0
		});
	}

	/**
	 * @brief Structure to store details required for a write staging process.
	 */
	struct StagingWriteInfo {
		const void* data;
		size_t size;
		size_t offset;

		vk::Buffer buffer;
		vk::Buffer stagingBuffer;
		vma::Allocation stagingAllocation;

		size_t stagingLimit;
		size_t stagingPosition;
	};

	/**
	 * Copies data from CPU to a staging buffer and submits the commands to copy
	 * each part one after another into the actual target buffer.
	 *
	 * The function can be used fully asynchronously!
	 * Just be careful to not use the staging buffer in parallel!
	 *
	 * @param core Core instance
	 * @param info Staging-info structure
	 */
	static void fillFromStagingBuffer(Core &core, StagingWriteInfo &info) {
		const size_t remaining = info.size - info.stagingPosition;
		const size_t mapped_size = std::min(remaining, info.stagingLimit);

		const vma::Allocator &allocator = core.getContext().getAllocator();

		void* mapped = allocator.mapMemory(info.stagingAllocation);
		memcpy(mapped, reinterpret_cast<const char*>(info.data) + info.stagingPosition,
			   mapped_size);
		allocator.unmapMemory(info.stagingAllocation);

		auto stream = core.createCommandStream(QueueType::Transfer);

		core.recordCommandsToStream(
			stream,
			[&info, &mapped_size](const vk::CommandBuffer &commandBuffer) {
				const vk::BufferCopy region(0, info.offset + info.stagingPosition, mapped_size);

				commandBuffer.copyBuffer(info.stagingBuffer, info.buffer, 1, &region);
			},
			[&core, &info, &mapped_size, &remaining]() {
				if (mapped_size < remaining) {
					info.stagingPosition += mapped_size;

					fillFromStagingBuffer(core, info);
				}
			});

		core.submitCommandStream(stream, false);
	}

	/**
	 * @brief Structure to store details required for a read staging process.
	 */
	struct StagingReadInfo {
		void* data;
		size_t size;
		size_t offset;

		vk::Buffer buffer;
		vk::Buffer stagingBuffer;
		vma::Allocation stagingAllocation;

		size_t stagingLimit;
		size_t stagingPosition;
	};

	/**
	 * Copies data from a staging buffer to CPU and submits the commands to copy
	 * each part one after another into the actual target data pointer.
	 *
	 * The function can be used fully asynchronously!
	 * Just be careful to not use the staging buffer in parallel!
	 *
	 * @param core Core instance
	 * @param info Staging-info structure
	 */
	static void readToStagingBuffer(Core &core, StagingReadInfo &info) {
		const size_t remaining = info.size - info.stagingPosition;
		const size_t mapped_size = std::min(remaining, info.stagingLimit);

		auto stream = core.createCommandStream(QueueType::Transfer);

		core.recordCommandsToStream(
			stream,
			[&info, &mapped_size](const vk::CommandBuffer &commandBuffer) {
				const vk::BufferCopy region(info.offset + info.stagingPosition, 0, mapped_size);

				commandBuffer.copyBuffer(info.buffer, info.stagingBuffer, 1, &region);
			},
			[&core, &info, &mapped_size, &remaining]() {
				const vma::Allocator &allocator = core.getContext().getAllocator();

				const void* mapped = allocator.mapMemory(info.stagingAllocation);
				memcpy(reinterpret_cast<char*>(info.data) + info.stagingPosition, mapped,
					   mapped_size);
				allocator.unmapMemory(info.stagingAllocation);

				if (mapped_size < remaining) {
					info.stagingPosition += mapped_size;

					readToStagingBuffer(core, info);
				}
			});

		core.submitCommandStream(stream, false);
	}

	vk::Buffer BufferManager::getBuffer(const BufferHandle &handle) const {
		auto &buffer = (*this) [handle];

		return buffer.m_handle;
	}

	TypeGuard BufferManager::getTypeGuard(const BufferHandle &handle) const {
		auto &buffer = (*this) [handle];

		return buffer.m_typeGuard;
	}

	BufferType BufferManager::getBufferType(const BufferHandle &handle) const {
		auto &buffer = (*this) [handle];

		return buffer.m_type;
	}

	BufferMemoryType BufferManager::getBufferMemoryType(const BufferHandle &handle) const {
		auto &buffer = (*this) [handle];

		return buffer.m_memoryType;
	}

	size_t BufferManager::getBufferSize(const BufferHandle &handle) const {
		auto &buffer = (*this) [handle];

		return buffer.m_size;
	}

	vk::DeviceMemory BufferManager::getDeviceMemory(const BufferHandle &handle) const {
		auto &buffer = (*this) [handle];

		const vma::Allocator &allocator = getCore().getContext().getAllocator();

		auto info = allocator.getAllocationInfo(buffer.m_allocation);

		return info.deviceMemory;
	}
	
	vk::DeviceAddress BufferManager::getBufferDeviceAddress(
			const vkcv::BufferHandle &handle) const {
		auto &buffer = (*this) [handle];
		
		return getCore().getContext().getDevice().getBufferAddress(
				vk::BufferDeviceAddressInfo(buffer.m_handle)
		);
	}

	void BufferManager::fillBuffer(const BufferHandle &handle,
								   const void* data,
								   size_t size,
								   size_t offset,
								   bool forceStaging) {
		auto &buffer = (*this) [handle];

		if (size == 0) {
			size = std::numeric_limits<size_t>::max();
		}

		const vma::Allocator &allocator = getCore().getContext().getAllocator();

		if (offset > buffer.m_size) {
			return;
		}

		const size_t max_size = std::min(size, buffer.m_size - offset);

		if ((buffer.m_mappable) && (!forceStaging)) {
			void* mapped = allocator.mapMemory(buffer.m_allocation);
			memcpy(reinterpret_cast<char*>(mapped) + offset, data, max_size);
			allocator.unmapMemory(buffer.m_allocation);
		} else {
			auto &stagingBuffer = (*this) [m_stagingBuffer];

			StagingWriteInfo info;
			info.data = data;
			info.size = max_size;
			info.offset = offset;

			info.buffer = buffer.m_handle;
			info.stagingBuffer = stagingBuffer.m_handle;
			info.stagingAllocation = stagingBuffer.m_allocation;

			info.stagingLimit = stagingBuffer.m_size;
			info.stagingPosition = 0;

			fillFromStagingBuffer(getCore(), info);
		}
	}

	void BufferManager::readBuffer(const BufferHandle &handle,
								   void* data,
								   size_t size,
								   size_t offset) {
		auto &buffer = (*this) [handle];

		if (size == 0) {
			size = std::numeric_limits<size_t>::max();
		}

		const vma::Allocator &allocator = getCore().getContext().getAllocator();

		if (offset > buffer.m_size) {
			return;
		}

		const size_t max_size = std::min(size, buffer.m_size - offset);

		if (buffer.m_mappable) {
			const void* mapped = allocator.mapMemory(buffer.m_allocation);
			memcpy(data, reinterpret_cast<const char*>(mapped) + offset, max_size);
			allocator.unmapMemory(buffer.m_allocation);
		} else {
			auto &stagingBuffer = (*this) [m_stagingBuffer];

			StagingReadInfo info;
			info.data = data;
			info.size = max_size;
			info.offset = offset;

			info.buffer = buffer.m_handle;
			info.stagingBuffer = stagingBuffer.m_handle;
			info.stagingAllocation = stagingBuffer.m_allocation;

			info.stagingLimit = stagingBuffer.m_size;
			info.stagingPosition = 0;

			readToStagingBuffer(getCore(), info);
		}
	}

	void* BufferManager::mapBuffer(const BufferHandle &handle, size_t offset, size_t size) {
		auto &buffer = (*this) [handle];

		if (size == 0) {
			size = std::numeric_limits<size_t>::max();
		}

		if (offset > buffer.m_size) {
			return nullptr;
		}
		
		if (buffer.m_mapping) {
			++buffer.m_mapCounter;
			
			vkcv_log(LogLevel::WARNING,
					 "Mapping a buffer multiple times (%lu) is not recommended",
					 buffer.m_mapCounter);
			
			return buffer.m_mapping + offset;
		}
		
		if (buffer.m_mappable) {
			const vma::Allocator &allocator = getCore().getContext().getAllocator();
			
			buffer.m_mapping = reinterpret_cast<char*>(allocator.mapMemory(buffer.m_allocation));
		} else {
			buffer.m_mapping = m_allocator.allocate(buffer.m_size);
			
			if (buffer.m_readable) {
				readBuffer(handle, buffer.m_mapping, buffer.m_size, 0);
			}
		}
		
		buffer.m_mapCounter = 1;
		return buffer.m_mapping + offset;
	}

	void BufferManager::unmapBuffer(const BufferHandle &handle) {
		auto &buffer = (*this) [handle];
		
		if (buffer.m_mapCounter > 1) {
			--buffer.m_mapCounter;
			return;
		}
		
		if (buffer.m_mapCounter == 0) {
			vkcv_log(LogLevel::WARNING,
					 "It seems like the buffer is not mapped to memory");
		}
		
		if (!buffer.m_mapping) {
			vkcv_log(LogLevel::ERROR,
					 "Buffer is not mapped to memory");
		}
		
		if (buffer.m_mappable) {
			const vma::Allocator &allocator = getCore().getContext().getAllocator();
			
			allocator.unmapMemory(buffer.m_allocation);
		} else {
			fillBuffer(handle, buffer.m_mapping, buffer.m_size, 0);
			m_allocator.deallocate(buffer.m_mapping, buffer.m_size);
		}
		
		buffer.m_mapping = nullptr;
		buffer.m_mapCounter = 0;
	}

	void BufferManager ::recordBufferMemoryBarrier(const BufferHandle &handle,
												   vk::CommandBuffer cmdBuffer) {
		auto &buffer = (*this) [handle];

		vk::BufferMemoryBarrier memoryBarrier(vk::AccessFlagBits::eMemoryWrite,
											  vk::AccessFlagBits::eMemoryRead, 0, 0,
											  buffer.m_handle, 0, buffer.m_size);

		cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
								  vk::PipelineStageFlagBits::eAllCommands, {}, nullptr,
								  memoryBarrier, nullptr);
	}

} // namespace vkcv
