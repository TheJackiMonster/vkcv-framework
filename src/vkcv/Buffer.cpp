/**
 * @authors Lars Hoerttrich
 * @file src/vkcv/Buffer.cpp
 * @brief Implementation of template buffer class, template for type security
 */
#include"vkcv/Buffer.hpp"

namespace vkcv {

	/**
	 * @brief searches memory type index for buffer allocation, inspired by vulkan tutorial and "https://github.com/KhronosGroup/Vulkan-Hpp/blob/master/samples/utils/utils.hpp"
	 * @param physicalMemoryProperties Memory Properties of physical device
	 * @param typeBits
	 * @param requirements Property flags that are required
	 * @return memory type index for Buffer
	 */
	uint32_t searchMemoryType(vk::PhysicalDeviceMemoryProperties const& physicalMemoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirements) {
		uint32_t memoryTypeIndex = uint32_t(0);
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

	void outsourcedDestructor(vk::Device device, vk::DeviceMemory bufferMemory, vk::Buffer buffer)
	{
		if (device) {
			device.freeMemory(bufferMemory);
			device.destroyBuffer(buffer);
		}
	}

	vk::Buffer outsourcedCreateBuffer(vk::Device device, BufferType type, size_t size)
	{
		switch (type) {
		case VERTEX: {
			//create vertex buffer
			return device.createBuffer(vk::BufferCreateInfo(vk::BufferCreateFlags(), size, vk::BufferUsageFlagBits::eVertexBuffer));
		}
		default: {
			// TODO: maybe an issue
		}
		}

		return vk::Buffer(); //should never be reached
	}

	vk::DeviceMemory outsourcedAllocateMemory(vk::Device device, vk::PhysicalDevice physicalDevice, vk::MemoryRequirements memoryRequirements)
	{
		//find Memory Type
		uint32_t memoryType = searchMemoryType(physicalDevice.getMemoryProperties(), memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		return device.allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size, memoryType));
	}
}