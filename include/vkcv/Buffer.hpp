#pragma once
/**
 * @authors Lars Hoerttrich
 * @file include/vkcv/Buffer.hpp
 * @brief template buffer class, template for type security, implemented here because template classes can't be written in .cpp
 */
#include "vkcv/Handles.hpp"
#include <vulkan/vulkan.hpp>
#include "vkcv/Context.hpp"



namespace vkcv {
	//Enum of buffertypes
	enum BufferType { VERTEX, UNIFORM, STORAGE };

	//(temporary?) struct example for T
	//TODO
	struct vertex_t {
		//glm::vec3 pos;
		//glm::vec3 color;
		float x, y, z;
	};

	template<typename T>
	class Buffer {
	public:
		//future bufferHandle struct
		struct Handle {
			uint64_t id;

		};

		// explicit destruction of default constructor
		Buffer<T>() = delete;
		// is never called directly
		~Buffer<T>() {
			m_Device.freeMemory(m_BufferMemory);
			m_Device.destroyBuffer(m_Buffer);
		} //noexcept; //gibt Fehlermeldung

		Buffer<T>(const Buffer<T>& other) = delete; // copy-ctor
		Buffer<T>(Buffer<T>&& other) {
			other.m_Buffer = nullptr;
			other.m_BufferMemory = nullptr;
			other.m_Device = nullptr;
			other.m_MemoryRequirement = nullptr;
			other.m_Type = vkcv::VERTEX;
			other.m_Size = 0;
			other.m_DataP = nullptr;

			return *this;
		} //noexcept; // move-ctor

		Buffer<T>& operator=(const Buffer<T>& other) = delete; // copy assignment
		Buffer<T>& operator=(Buffer<T>&& other) {
			m_Buffer = other.m_Buffer;
			m_BufferMemory = other.m_BufferMemory;
			m_Device = other.m_Device;
			m_MemoryRequirement = other.m_MemoryRequirement;
			m_Type = other.m_Type;
			m_Size = other.m_Size;
			m_DataP = other.m_DataP;

			other.m_Buffer = nullptr;
			other.m_BufferMemory = nullptr;
			other.m_Device = nullptr;
			other.m_MemoryRequirement = nullptr;
			other.m_Type = vkcv::VERTEX;
			other.m_Size = 0;
			other.m_DataP = nullptr;

		}//noexcept; // move assignment

		BufferType getType() { return m_Type; };
		size_t getSize() { return m_Size; };

		//i'm not sure what the input argument has to be, WORK IN PROGRESS
		//TODO
		void fill(void* data) {
			m_DataP = static_cast<uint8_t*>(m_Device.mapMemory(m_BufferMemory, 0, m_MemoryRequirement.size));
			memcpy(m_DataP, data, sizeof(data));
			m_Device.unmapMemory(m_BufferMemory);
		};
		void map() {
			m_DataP = static_cast<uint8_t*>(m_Device.mapMemory(m_BufferMemory, 0, m_MemoryRequirement.size));
		};
		void unmap() {
			m_Device.unmapMemory(m_BufferMemory);
		};

	private:
		vk::Buffer m_Buffer;
		vk::DeviceMemory m_BufferMemory;
		vk::Device m_Device;
		vk::MemoryRequirements m_MemoryRequirement;
		BufferType m_Type;
		size_t m_Size=0;
		static uint8_t* m_DataP;

		/**
		 * @brief searches memory type index for buffer allocation, inspired by vulkan tutorial and "https://github.com/KhronosGroup/Vulkan-Hpp/blob/master/samples/utils/utils.hpp"
		 * @param memoryProperties 
		 * @param typeBits 
		 * @param requirementsMask 
		 * @return memory type for Buffer
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
		};

		/**
		 * * Constructor of #Buffer requires a @p device, a @p physicalDevice, a @p buffer type and a @p size.
		 *
		 * @param device Vulkan-Device
		 * @param physicalDevice Vulkan-PhysicalDevice
		 * @param type Enum type of possible vkcv::BufferType
		 * @param Size size of data
		 */
		Buffer<T>(vk::Device device, vk::PhysicalDevice physicalDevice, BufferType type, size_t size) {
			m_Type = type;
			m_Size = size;
			m_Device = device;


			switch (m_Type) {
				case VERTEX: {
					//create vertex buffer
					m_Buffer = m_Device.createBuffer(vk::BufferCreateInfo(vk::BufferCreateFlags(), sizeof(T) * m_Size, vk::BufferUsageFlagBits::eVertexBuffer));
				}
			}
			//get requirements for allocation
			m_MemoryRequirement = m_Device.getBufferMemoryRequirements(m_Buffer);
			//find Memory Type 
			uint32_t memoryType = searchMemoryType(physicalDevice.getMemoryProperties(), m_MemoryRequirement.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			//allocate memory for buffer
			m_BufferMemory = m_Device.allocateMemory(vk::MemoryAllocateInfo(m_MemoryRequirement.size, memoryType));

			device.bindBufferMemory(m_Buffer, m_BufferMemory, 0);
		}
	
	};
}
