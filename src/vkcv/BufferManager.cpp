/**
 * @author Tobias Frisch
 * @file vkcv/BufferManager.cpp
 */

#include "vkcv/BufferManager.hpp"

namespace vkcv {
	
	BufferManager::BufferManager(vk::Device device, vk::PhysicalDevice physicalDevice) noexcept :
		m_device(device), m_physicalDevice(physicalDevice)
	{}
	
	BufferManager::~BufferManager() noexcept {
		for (size_t id = 0; id < m_buffers.size(); id++) {
			destroyBuffer(id);
		}
	}
	
	/**
	 * @brief searches memory type index for buffer allocation, inspired by vulkan tutorial and "https://github.com/KhronosGroup/Vulkan-Hpp/blob/master/samples/utils/utils.hpp"
	 * @param physicalMemoryProperties Memory Properties of physical device
	 * @param typeBits
	 * @param requirements Property flags that are required
	 * @return memory type index for Buffer
	 */
	uint32_t searchMemoryType(const vk::PhysicalDeviceMemoryProperties& physicalMemoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirements) {
		uint32_t memoryTypeIndex = 0;
		
		for (uint32_t i = 0; i < physicalMemoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) &&
				((physicalMemoryProperties.memoryTypes[i].propertyFlags & requirements) == requirements))
			{
				memoryTypeIndex = i;
				break;
			}
			
			typeBits >>= 1;
		}
		
		return memoryTypeIndex;
	}
	
	uint64_t BufferManager::createBuffer(BufferType type, size_t size) {
		vk::BufferCreateFlags createFlags;
		vk::BufferUsageFlags usageFlags;
		
		switch (type) {
			case VERTEX:
				usageFlags = vk::BufferUsageFlagBits::eVertexBuffer;
				break;
			case UNIFORM:
				usageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
				break;
			case STORAGE:
				usageFlags = vk::BufferUsageFlagBits::eStorageBuffer;
				break;
			default:
				// TODO: maybe an issue
				break;
		}
		
		vk::Buffer buffer = m_device.createBuffer(
				vk::BufferCreateInfo(createFlags, size, usageFlags)
		);
		
		const vk::MemoryRequirements requirements = m_device.getBufferMemoryRequirements(buffer);
		
		const uint32_t memoryType = searchMemoryType(
				m_physicalDevice.getMemoryProperties(),
				requirements.memoryTypeBits,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		
		vk::DeviceMemory memory = m_device.allocateMemory(vk::MemoryAllocateInfo(requirements.size, memoryType));
		
		const uint64_t id = m_buffers.size();
		m_buffers.push_back({ buffer, memory, nullptr });
		return id;
	}
	
	void BufferManager::fillBuffer(uint64_t id, void *data, size_t size, size_t offset) {
		if (id >= m_buffers.size()) {
			return;
		}
		
		auto& buffer = m_buffers[id];
		
		if (buffer.m_mapped) {
			return;
		}
		
		const vk::MemoryRequirements requirements = m_device.getBufferMemoryRequirements(buffer.m_handle);
		
		if (offset > requirements.size) {
			return;
		}
		
		const size_t mapped_size = std::min(size, requirements.size - offset);
		void* mapped = m_device.mapMemory(buffer.m_memory, offset, mapped_size);
		memcpy(mapped, data, mapped_size);
		m_device.unmapMemory(buffer.m_memory);
	}
	
	void* BufferManager::mapBuffer(uint64_t id, size_t offset, size_t size) {
		if (id >= m_buffers.size()) {
			return nullptr;
		}
		
		auto& buffer = m_buffers[id];
		
		if (buffer.m_mapped) {
			return nullptr;
		}
		
		const vk::MemoryRequirements requirements = m_device.getBufferMemoryRequirements(buffer.m_handle);
		
		if (offset > requirements.size) {
			return nullptr;
		}
		
		const size_t mapped_size = std::min(size, requirements.size - offset);
		buffer.m_mapped = m_device.mapMemory(buffer.m_memory, offset, mapped_size);
		return buffer.m_mapped;
	}
	
	void BufferManager::unmapBuffer(uint64_t id) {
		if (id >= m_buffers.size()) {
			return;
		}
		
		auto& buffer = m_buffers[id];
		
		if (buffer.m_mapped == nullptr) {
			return;
		}
		
		m_device.unmapMemory(buffer.m_memory);
		buffer.m_mapped = nullptr;
	}
	
	void BufferManager::destroyBuffer(uint64_t id) {
		if (id >= m_buffers.size()) {
			return;
		}
		
		auto& buffer = m_buffers[id];
		
		if (buffer.m_memory) {
			m_device.freeMemory(buffer.m_memory);
		}
		
		if (buffer.m_handle) {
			m_device.destroyBuffer(buffer.m_handle);
		}
	}

}
