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


		uint64_t createImage();

		/**
		 * Destroys and deallocates image represented by a given
		 * buffer handle id.
		 *
		 * @param id Buffer handle id
		 */
		void destroyImage(uint64_t id);
	};
}