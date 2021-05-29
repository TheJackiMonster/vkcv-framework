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
	class ImageManager 
	{
		friend class Core;
	private:
		struct Image
		{
			vk::Image m_handle;
			vk::DeviceMemory m_memory;
			void* m_mapped = nullptr;
			bool m_mappable;
		};
		Core* m_core;
		std::vector<Image> m_images;
		ImageManager() noexcept;
	public:
		~ImageManager() noexcept;
		ImageManager(ImageManager&& other) = delete;
		ImageManager(const ImageManager& other) = delete;

		ImageManager& operator=(ImageManager&& other) = delete;
		ImageManager& operator=(const ImageManager& other) = delete;

		void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
		void switchImageLayout(uint64_t id, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

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