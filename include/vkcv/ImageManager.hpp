#pragma once
/**
 * @authors Lars Hoerttrich
 * @file vkcv/ImageManager.hpp
 * @brief class creating and managing images
 */
#include <vector>
#include <vulkan/vulkan.hpp>

namespace vkcv {
	class Core;
	class BufferManager;
	class ImageManager
	{
		friend class Core;
	private:
		struct Image
		{
			vk::Image m_handle;
			vk::DeviceMemory m_memory;
			size_t m_size;
		};
		
		Core* m_core;
		vk::Buffer m_stagingBuffer;
		vk::DeviceMemory m_stagingMemory;

		std::vector<Image> m_images;
		void init(BufferManager* bufferManager);
		ImageManager() noexcept;
	public:
		~ImageManager() noexcept;
		ImageManager(ImageManager&& other) = delete;
		ImageManager(const ImageManager& other) = delete;

		ImageManager& operator=(ImageManager&& other) = delete;
		ImageManager& operator=(const ImageManager& other) = delete;
		
		void switchImageLayout(uint64_t id, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
		void fillImage(uint64_t id, void* data, size_t size);

		uint64_t createImage(uint32_t width, uint32_t height);

		/**
		 * Destroys and deallocates image represented by a given
		 * buffer handle id.
		 *
		 * @param id Buffer handle id
		 */
		void destroyImage(uint64_t id);
	};
}