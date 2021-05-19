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

	//Functions outsourced to Buffer.cpp file:
	void outsourcedDestructor(vk::Device device, vk::DeviceMemory bufferMemory, vk::Buffer buffer);
	vk::Buffer outsourcedCreateBuffer(vk::Device device, BufferType type, size_t size);
	vk::DeviceMemory outsourcedAllocateMemory(vk::Device device, vk::PhysicalDevice physicalDevice, vk::MemoryRequirements memoryRequirements);

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
		~Buffer<T>() noexcept {
			outsourcedDestructor(m_Device, m_BufferMemory, m_Buffer);
		}

		Buffer<T>(const Buffer<T>& other) = delete; // copy-ctor
		Buffer<T>(Buffer<T>&& other) noexcept :
		        m_Buffer(other.m_Buffer),
				m_BufferMemory(other.m_BufferMemory),
				m_Device(other.m_Device),
		        m_Type(other.m_Type),
		        m_Size(other.m_Size),
		        m_DataP(other.m_DataP)
		{
			other.m_Buffer = nullptr;
			other.m_BufferMemory = nullptr;
			other.m_Device = nullptr;
			other.m_Type = vkcv::VERTEX; //set to 0
			other.m_Size = 0;
			other.m_DataP = nullptr;
		} // move-ctor

		Buffer<T>& operator=(const Buffer<T>& other) = delete; // copy assignment
		Buffer<T>& operator=(Buffer<T>&& other) noexcept {
			m_Buffer = other.m_Buffer;
			m_BufferMemory = other.m_BufferMemory;
			m_Device = other.m_Device;
			m_Type = other.m_Type;
			m_Size = other.m_Size;
			m_DataP = other.m_DataP;

			other.m_Buffer = nullptr;
			other.m_BufferMemory = nullptr;
			other.m_Device = nullptr;
			other.m_Type = vkcv::VERTEX; //set to 0
			other.m_Size = 0;
			other.m_DataP = nullptr;

		}// move assignment

		BufferType getType() { return m_Type; };
		size_t getSize() { return m_Size; };
		
		/**
		 * Maps this buffers Memory, fills this buffer with @p data of type T and count @p count
		 * unmaps afterwards.
		 * @p data Pointer to data
		 * @p count Amount of data of type T
		 */
		 // TODO: we will probably need staging-buffer here later (possible add in BufferManager later?)
		void fill(T* data, size_t count) {
			const vk::MemoryRequirements requirements = m_Device.getBufferMemoryRequirements(m_Buffer);
			
			// TODO: check if mapped already
			m_DataP = static_cast<uint8_t*>(m_Device.mapMemory(m_BufferMemory, 0, requirements.size));
			memcpy(m_DataP, data, sizeof(T) * count);
			m_Device.unmapMemory(m_BufferMemory);
		};
		
		T* map() {
			const vk::MemoryRequirements requirements = m_Device.getBufferMemoryRequirements(m_Buffer);
			
			m_DataP = static_cast<uint8_t*>(m_Device.mapMemory(m_BufferMemory, 0, requirements.size));
			// TODO: make sure to unmap before deallocation
			
			return reinterpret_cast<T*>(m_DataP);
		};
		
		void unmap() {
			m_Device.unmapMemory(m_BufferMemory);
			// TODO: mark m_DataP as invalid?
		};
		
		/**
		 * * Create function of #Buffer requires a @p device, a @p physicalDevice, a @p buffer type and a @p size.		 *
		 * @param device Vulkan-Device
		 * @param physicalDevice Vulkan-PhysicalDevice
		 * @param type Enum type of possible vkcv::BufferType
		 * @param Size size of data
		 */
		static Buffer<T> create(vk::Device device, vk::PhysicalDevice physicalDevice, BufferType type, size_t size) {
			vk::Buffer buffer = nullptr;
			

			buffer = outsourcedCreateBuffer(device, type, sizeof(T) * size);
			
			if (!buffer) {
				//TODO: potential issue
			}
			
			//get requirements for allocation
			const vk::MemoryRequirements requirements = device.getBufferMemoryRequirements(buffer);
			
			vk::DeviceMemory memory= outsourcedAllocateMemory(device, physicalDevice, requirements);
			
			if (!memory) {
				//TODO: other potential issue
			}
			
			device.bindBufferMemory(buffer, memory, 0);
			
			return Buffer<T>(buffer, memory, device, type, size);
		}

	private:
		vk::Buffer m_Buffer;
		vk::DeviceMemory m_BufferMemory;
		vk::Device m_Device;
		BufferType m_Type;
		size_t m_Size=0;
		uint8_t* m_DataP;

		/**
		 * * Constructor of #Buffer requires a @p buffer, a @p memory, @p device, @ requirement, a @p buffer type and a @p size.
		 * @param buffer Vulkan-Buffer
		 * @param memory Vulkan-DeviceMemory
		 * @param device Vulkan-Device
		 * @param requirement Vulkan-MemoryRequirements
		 * @param type Enum type of possible vkcv::BufferType
		 * @param Size size of data
		 */
		Buffer<T>(vk::Buffer buffer, vk::DeviceMemory memory, vk::Device device, BufferType type, size_t size) :
				m_Buffer(buffer),
				m_BufferMemory(memory),
				m_Device(device),
				m_Type(type),
				m_Size(size),
				m_DataP(nullptr)
		{}	
	};
}
