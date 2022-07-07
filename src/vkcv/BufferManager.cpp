/**
 * @author Tobias Frisch
 * @file vkcv/BufferManager.cpp
 */

#include "BufferManager.hpp"
#include "vkcv/Core.hpp"
#include <vkcv/Logger.hpp>

namespace vkcv {
	
	BufferManager::BufferManager() noexcept :
		m_core(nullptr), m_buffers(), m_stagingBuffer(BufferHandle())
	{
	}
	
	void BufferManager::init() {
		if (!m_core) {
			return;
		}
		
		m_stagingBuffer = createBuffer(
				TypeGuard(1),
				BufferType::STAGING,
				BufferMemoryType::HOST_VISIBLE,
				1024 * 1024,
				false
		);
	}
	
	BufferManager::~BufferManager() noexcept {
		for (uint64_t id = 0; id < m_buffers.size(); id++) {
			destroyBufferById(id);
		}
	}
	
	BufferHandle BufferManager::createBuffer(const TypeGuard &typeGuard,
											 BufferType type,
											 BufferMemoryType memoryType,
											 size_t size,
											 bool readable) {
		vk::BufferCreateFlags createFlags;
		vk::BufferUsageFlags usageFlags;
		
		switch (type) {
			case BufferType::VERTEX:
				usageFlags = vk::BufferUsageFlagBits::eVertexBuffer;
				break;
			case BufferType::UNIFORM:
				usageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
				break;
			case BufferType::STORAGE:
				usageFlags = vk::BufferUsageFlagBits::eStorageBuffer;
				break;
			case BufferType::STAGING:
				usageFlags = vk::BufferUsageFlagBits::eTransferSrc |
							 vk::BufferUsageFlagBits::eTransferDst;
				break;
			case BufferType::INDEX:
				usageFlags = vk::BufferUsageFlagBits::eIndexBuffer;
				break;
            case BufferType::INDIRECT:
                usageFlags = vk::BufferUsageFlagBits::eStorageBuffer |
							 vk::BufferUsageFlagBits::eIndirectBuffer ;
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
		
		const vma::Allocator& allocator = m_core->getContext().getAllocator();
		
		vk::MemoryPropertyFlags memoryTypeFlags;
		vma::MemoryUsage memoryUsage;
		bool mappable = false;
		
		switch (memoryType) {
			case BufferMemoryType::DEVICE_LOCAL:
				memoryTypeFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
				memoryUsage = vma::MemoryUsage::eGpuOnly;
				mappable = false;
				break;
			case BufferMemoryType::HOST_VISIBLE:
				memoryTypeFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
				memoryUsage = vma::MemoryUsage::eCpuOnly;
				mappable = true;
				break;
			default:
				vkcv_log(LogLevel::WARNING, "Unknown buffer memory type");
				memoryUsage = vma::MemoryUsage::eUnknown;
				mappable = false;
				break;
		}
		
		if (type == BufferType::STAGING) {
			memoryUsage = vma::MemoryUsage::eCpuToGpu;
		}

		auto bufferAllocation = allocator.createBuffer(
				vk::BufferCreateInfo(createFlags, size, usageFlags),
				vma::AllocationCreateInfo(
						vma::AllocationCreateFlags(),
						memoryUsage,
						memoryTypeFlags,
						memoryTypeFlags,
						0,
						vma::Pool(),
						nullptr
				)
		);
		
		vk::Buffer buffer = bufferAllocation.first;
		vma::Allocation allocation = bufferAllocation.second;
		
		const uint64_t id = m_buffers.size();
		m_buffers.push_back({
			typeGuard,
			type,
			memoryType,
			size,
			buffer,
			allocation,
			mappable
		});
		
		return BufferHandle(id, [&](uint64_t id) { destroyBufferById(id); });
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
	static void fillFromStagingBuffer(Core* core, StagingWriteInfo& info) {
		const size_t remaining = info.size - info.stagingPosition;
		const size_t mapped_size = std::min(remaining, info.stagingLimit);
		
		const vma::Allocator& allocator = core->getContext().getAllocator();
		
		void* mapped = allocator.mapMemory(info.stagingAllocation);
		memcpy(mapped, reinterpret_cast<const char*>(info.data) + info.stagingPosition, mapped_size);
		allocator.unmapMemory(info.stagingAllocation);
		
		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Transfer;
		
		core->recordAndSubmitCommandsImmediate(
				submitInfo,
				[&info, &mapped_size](const vk::CommandBuffer& commandBuffer) {
					const vk::BufferCopy region (
							0,
							info.offset + info.stagingPosition,
							mapped_size
					);
					
					commandBuffer.copyBuffer(info.stagingBuffer, info.buffer, 1, &region);
				},
				[&core, &info, &mapped_size, &remaining]() {
					if (mapped_size < remaining) {
						info.stagingPosition += mapped_size;
						
						fillFromStagingBuffer(
								core,
								info
						);
					}
				}
		);
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
	static void readToStagingBuffer(Core* core, StagingReadInfo& info) {
		const size_t remaining = info.size - info.stagingPosition;
		const size_t mapped_size = std::min(remaining, info.stagingLimit);
		
		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Transfer;
		
		core->recordAndSubmitCommandsImmediate(
				submitInfo,
				[&info, &mapped_size](const vk::CommandBuffer& commandBuffer) {
					const vk::BufferCopy region (
							info.offset + info.stagingPosition,
							0,
							mapped_size
					);
					
					commandBuffer.copyBuffer(info.buffer, info.stagingBuffer, 1, &region);
				},
				[&core, &info, &mapped_size, &remaining]() {
					const vma::Allocator& allocator = core->getContext().getAllocator();
					
					const void* mapped = allocator.mapMemory(info.stagingAllocation);
					memcpy(reinterpret_cast<char*>(info.data) + info.stagingPosition, mapped, mapped_size);
					allocator.unmapMemory(info.stagingAllocation);
					
					if (mapped_size < remaining) {
						info.stagingPosition += mapped_size;
						
						readToStagingBuffer(
								core,
								info
						);
					}
				}
		);
	}
	
	vk::Buffer BufferManager::getBuffer(const BufferHandle& handle) const {
		const uint64_t id = handle.getId();
		
		if (id >= m_buffers.size()) {
			return nullptr;
		}
		
		auto& buffer = m_buffers[id];
		
		return buffer.m_handle;
	}
	
	TypeGuard BufferManager::getTypeGuard(const BufferHandle &handle) const {
		const uint64_t id = handle.getId();
		
		if (id >= m_buffers.size()) {
			return TypeGuard(0);
		}
		
		auto& buffer = m_buffers[id];
		
		return buffer.m_typeGuard;
	}
	
	BufferType BufferManager::getBufferType(const BufferHandle &handle) const {
		const uint64_t id = handle.getId();
		
		if (id >= m_buffers.size()) {
			return BufferType::UNKNOWN;
		}
		
		auto& buffer = m_buffers[id];
		
		return buffer.m_type;
	}
	
	BufferMemoryType BufferManager::getBufferMemoryType(const BufferHandle &handle) const {
		const uint64_t id = handle.getId();
		
		if (id >= m_buffers.size()) {
			return BufferMemoryType::UNKNOWN;
		}
		
		auto& buffer = m_buffers[id];
		
		return buffer.m_memoryType;
	}
	
	size_t BufferManager::getBufferSize(const BufferHandle &handle) const {
		const uint64_t id = handle.getId();
		
		if (id >= m_buffers.size()) {
			return 0;
		}
		
		auto& buffer = m_buffers[id];
		
		return buffer.m_size;
	}
	
	vk::DeviceMemory BufferManager::getDeviceMemory(const BufferHandle& handle) const {
		const uint64_t id = handle.getId();
		
		if (id >= m_buffers.size()) {
			return nullptr;
		}
		
		auto& buffer = m_buffers[id];
		
		const vma::Allocator& allocator = m_core->getContext().getAllocator();
		
		auto info = allocator.getAllocationInfo(
				buffer.m_allocation
		);
		
		return info.deviceMemory;
	}
	
	void BufferManager::fillBuffer(const BufferHandle& handle,
								   const void *data,
								   size_t size,
								   size_t offset) {
		const uint64_t id = handle.getId();
		
		if (size == 0) {
			size = SIZE_MAX;
		}
		
		if (id >= m_buffers.size()) {
			return;
		}
		
		auto& buffer = m_buffers[id];
		
		const vma::Allocator& allocator = m_core->getContext().getAllocator();
		
		if (offset > buffer.m_size) {
			return;
		}
		
		const size_t max_size = std::min(size, buffer.m_size - offset);
		
		if (buffer.m_mappable) {
			void* mapped = allocator.mapMemory(buffer.m_allocation);
			memcpy(reinterpret_cast<char*>(mapped) + offset, data, max_size);
			allocator.unmapMemory(buffer.m_allocation);
		} else {
			auto& stagingBuffer = m_buffers[ m_stagingBuffer.getId() ];
			
			StagingWriteInfo info;
			info.data = data;
			info.size = max_size;
			info.offset = offset;
			
			info.buffer = buffer.m_handle;
			info.stagingBuffer = stagingBuffer.m_handle;
			info.stagingAllocation = stagingBuffer.m_allocation;
			
			info.stagingLimit = stagingBuffer.m_size;
			info.stagingPosition = 0;
			
			fillFromStagingBuffer(m_core, info);
		}
	}
	
	void BufferManager::readBuffer(const BufferHandle &handle,
								   void *data,
								   size_t size,
								   size_t offset) {
		const uint64_t id = handle.getId();
		
		if (size == 0) {
			size = SIZE_MAX;
		}
		
		if (id >= m_buffers.size()) {
			return;
		}
		
		auto& buffer = m_buffers[id];
		
		const vma::Allocator& allocator = m_core->getContext().getAllocator();
		
		if (offset > buffer.m_size) {
			return;
		}
		
		const size_t max_size = std::min(size, buffer.m_size - offset);
		
		if (buffer.m_mappable) {
			const void* mapped = allocator.mapMemory(buffer.m_allocation);
			memcpy(data, reinterpret_cast<const char*>(mapped) + offset, max_size);
			allocator.unmapMemory(buffer.m_allocation);
		} else {
			auto& stagingBuffer = m_buffers[ m_stagingBuffer.getId() ];
			
			StagingReadInfo info;
			info.data = data;
			info.size = max_size;
			info.offset = offset;
			
			info.buffer = buffer.m_handle;
			info.stagingBuffer = stagingBuffer.m_handle;
			info.stagingAllocation = stagingBuffer.m_allocation;
			
			info.stagingLimit = stagingBuffer.m_size;
			info.stagingPosition = 0;
			
			readToStagingBuffer(m_core, info);
		}
	}
	
	void* BufferManager::mapBuffer(const BufferHandle& handle, size_t offset, size_t size) {
		const uint64_t id = handle.getId();
		
		if (size == 0) {
			size = SIZE_MAX;
		}
		
		if (id >= m_buffers.size()) {
			return nullptr;
		}
		
		auto& buffer = m_buffers[id];
		
		const vma::Allocator& allocator = m_core->getContext().getAllocator();
		
		if (offset > buffer.m_size) {
			return nullptr;
		}
		
		return reinterpret_cast<char*>(allocator.mapMemory(buffer.m_allocation)) + offset;
	}
	
	void BufferManager::unmapBuffer(const BufferHandle& handle) {
		const uint64_t id = handle.getId();
		
		if (id >= m_buffers.size()) {
			return;
		}
		
		auto& buffer = m_buffers[id];
		
		const vma::Allocator& allocator = m_core->getContext().getAllocator();
		
		allocator.unmapMemory(buffer.m_allocation);
	}
	
	void BufferManager::destroyBufferById(uint64_t id) {
		if (id >= m_buffers.size()) {
			return;
		}
		
		auto& buffer = m_buffers[id];
		
		const vma::Allocator& allocator = m_core->getContext().getAllocator();
		
		if (buffer.m_handle) {
			allocator.destroyBuffer(buffer.m_handle, buffer.m_allocation);
			
			buffer.m_handle = nullptr;
			buffer.m_allocation = nullptr;
		}
	}

	void BufferManager ::recordBufferMemoryBarrier(const BufferHandle& handle, vk::CommandBuffer cmdBuffer) {

		const uint64_t id = handle.getId();

		if (id >= m_buffers.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid buffer handle");
			return;
		}

		auto& buffer = m_buffers[id];
		
		vk::BufferMemoryBarrier memoryBarrier(
			vk::AccessFlagBits::eMemoryWrite, 
			vk::AccessFlagBits::eMemoryRead,
			0,
			0,
			buffer.m_handle,
			0,
			buffer.m_size);

		cmdBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eAllCommands,
			{},
			nullptr,
			memoryBarrier,
			nullptr);
	}

}
