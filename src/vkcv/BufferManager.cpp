/**
 * @author Tobias Frisch
 * @file vkcv/BufferManager.cpp
 */

#include "vkcv/BufferManager.hpp"
#include "vkcv/Core.hpp"

namespace vkcv {
	
	BufferManager::BufferManager() noexcept :
		m_core(nullptr), m_buffers(), m_stagingBuffer(BufferHandle())
	{
	}
	
	void BufferManager::init() {
		if (!m_core) {
			return;
		}
		
		m_stagingBuffer = createBuffer(BufferType::STAGING, 1024 * 1024, BufferMemoryType::HOST_VISIBLE);
	}
	
	BufferManager::~BufferManager() noexcept {
		for (uint64_t id = 0; id < m_buffers.size(); id++) {
			destroyBuffer(BufferHandle(id));
		}
	}
	
	/**
	 * @brief searches memory type index for buffer allocation, combines requirements of buffer and application
	 * @param physicalMemoryProperties Memory Properties of physical device
	 * @param typeBits Bit field for suitable memory types
	 * @param requirements Property flags that are required
	 * @return memory type index for Buffer
	 */
	uint32_t searchBufferMemoryType(const vk::PhysicalDeviceMemoryProperties& physicalMemoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirements) {
		const uint32_t memoryCount = physicalMemoryProperties.memoryTypeCount;
		for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex) {
			const uint32_t memoryTypeBits = (1 << memoryIndex);
			const bool isRequiredMemoryType = typeBits & memoryTypeBits;

			const vk::MemoryPropertyFlags properties =
				physicalMemoryProperties.memoryTypes[memoryIndex].propertyFlags;
			const bool hasRequiredProperties =
				(properties & requirements) == requirements;

			if (isRequiredMemoryType && hasRequiredProperties)
				return static_cast<int32_t>(memoryIndex);
		}

		// failed to find memory type
		return -1;
	}
	
	BufferHandle BufferManager::createBuffer(BufferType type, size_t size, BufferMemoryType memoryType) {
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
				usageFlags = vk::BufferUsageFlagBits::eTransferSrc;
				break;
			case BufferType::INDEX:
				usageFlags = vk::BufferUsageFlagBits::eIndexBuffer;
				break;
			default:
				// TODO: maybe an issue
				break;
		}
		
		if (memoryType == BufferMemoryType::DEVICE_LOCAL) {
			usageFlags |= vk::BufferUsageFlagBits::eTransferDst;
		}
		
		const vk::Device& device = m_core->getContext().getDevice();
		
		vk::Buffer buffer = device.createBuffer(
				vk::BufferCreateInfo(createFlags, size, usageFlags)
		);
		
		const vk::MemoryRequirements requirements = device.getBufferMemoryRequirements(buffer);
		const vk::PhysicalDevice& physicalDevice = m_core->getContext().getPhysicalDevice();
		
		vk::MemoryPropertyFlags memoryTypeFlags;
		bool mappable = false;
		
		switch (memoryType) {
			case BufferMemoryType::DEVICE_LOCAL:
				memoryTypeFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
				break;
			case BufferMemoryType::HOST_VISIBLE:
				memoryTypeFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
				mappable = true;
				break;
			default:
				// TODO: maybe an issue
				break;
		}
		
		const uint32_t memoryTypeIndex = searchBufferMemoryType(
				physicalDevice.getMemoryProperties(),
				requirements.memoryTypeBits,
				memoryTypeFlags
		);
		
		vk::DeviceMemory memory = device.allocateMemory(vk::MemoryAllocateInfo(requirements.size, memoryTypeIndex));
		
		device.bindBufferMemory(buffer, memory, 0);
		
		const uint64_t id = m_buffers.size();
		m_buffers.push_back({ buffer, memory, size, nullptr, mappable });
		return BufferHandle{ id };
	}
	
	struct StagingStepInfo {
		const void* data;
		size_t size;
		size_t offset;
		
		vk::Buffer buffer;
		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingMemory;
		
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
	void copyFromStagingBuffer(Core* core, StagingStepInfo& info) {
		const size_t remaining = info.size - info.stagingPosition;
		const size_t mapped_size = std::min(remaining, info.stagingLimit);
		
		const vk::Device& device = core->getContext().getDevice();
		
		void* mapped = device.mapMemory(info.stagingMemory, 0, mapped_size);
		memcpy(mapped, reinterpret_cast<const char*>(info.data) + info.stagingPosition, mapped_size);
		device.unmapMemory(info.stagingMemory);
		
		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Transfer;
		
		core->submitCommands(
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
						
						copyFromStagingBuffer(
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
		
		return buffer.m_memory;
	}
	
	void BufferManager::fillBuffer(const BufferHandle& handle, const void *data, size_t size, size_t offset) {
		const uint64_t id = handle.getId();
		
		if (size == 0) {
			size = SIZE_MAX;
		}
		
		if (id >= m_buffers.size()) {
			return;
		}
		
		auto& buffer = m_buffers[id];
		
		if (buffer.m_mapped) {
			return;
		}
		
		const vk::Device& device = m_core->getContext().getDevice();
		
		if (offset > buffer.m_size) {
			return;
		}
		
		const size_t max_size = std::min(size, buffer.m_size - offset);
		
		if (buffer.m_mappable) {
			void* mapped = device.mapMemory(buffer.m_memory, offset, max_size);
			memcpy(mapped, data, max_size);
			device.unmapMemory(buffer.m_memory);
		} else {
			auto& stagingBuffer = m_buffers[ m_stagingBuffer.getId() ];
			
			StagingStepInfo info;
			info.data = data;
			info.size = std::min(size, max_size - offset);
			info.offset = offset;
			
			info.buffer = buffer.m_handle;
			info.stagingBuffer = stagingBuffer.m_handle;
			info.stagingMemory = stagingBuffer.m_memory;
			
			const vk::MemoryRequirements stagingRequirements = device.getBufferMemoryRequirements(stagingBuffer.m_handle);
			
			info.stagingLimit = stagingRequirements.size;
			info.stagingPosition = 0;
			
			copyFromStagingBuffer(m_core, info);
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
		
		if (buffer.m_mapped) {
			return nullptr;
		}
		
		const vk::Device& device = m_core->getContext().getDevice();
		
		if (offset > buffer.m_size) {
			return nullptr;
		}
		
		const size_t max_size = std::min(size, buffer.m_size - offset);
		buffer.m_mapped = device.mapMemory(buffer.m_memory, offset, max_size);
		return buffer.m_mapped;
	}
	
	void BufferManager::unmapBuffer(const BufferHandle& handle) {
		const uint64_t id = handle.getId();
		
		if (id >= m_buffers.size()) {
			return;
		}
		
		auto& buffer = m_buffers[id];
		
		if (buffer.m_mapped == nullptr) {
			return;
		}
		
		const vk::Device& device = m_core->getContext().getDevice();
		
		device.unmapMemory(buffer.m_memory);
		buffer.m_mapped = nullptr;
	}
	
	void BufferManager::destroyBuffer(const BufferHandle& handle) {
		const uint64_t id = handle.getId();
		
		if (id >= m_buffers.size()) {
			return;
		}
		
		auto& buffer = m_buffers[id];
		
		const vk::Device& device = m_core->getContext().getDevice();
		
		if (buffer.m_memory) {
			device.freeMemory(buffer.m_memory);
			buffer.m_memory = nullptr;
		}
		
		if (buffer.m_handle) {
			device.destroyBuffer(buffer.m_handle);
			buffer.m_handle = nullptr;
		}
	}

}
