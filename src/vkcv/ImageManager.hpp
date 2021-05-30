#pragma once
/**
 * @authors Lars Hoerttrich
 * @file vkcv/ImageManager.hpp
 * @brief class creating and managing images
 */
#include <vector>
#include <vulkan/vulkan.hpp>

#include "vkcv/BufferManager.hpp"
#include "vkcv/Handles.hpp"

namespace vkcv {
	
	class ImageManager
	{
		friend class Core;
	private:
		struct Image
		{
			vk::Image m_handle;
			vk::DeviceMemory m_memory;
			vk::ImageView m_view;
			uint32_t m_width = 0;
			uint32_t m_height = 0;
			uint32_t m_depth = 0;
			vk::Format m_format;
		};
		
		Core* m_core;
		BufferManager& m_bufferManager;
		
		std::vector<Image> m_images;
		
		ImageManager(BufferManager& bufferManager) noexcept;
		
	public:
		~ImageManager() noexcept;
		ImageManager(ImageManager&& other) = delete;
		ImageManager(const ImageManager& other) = delete;

		ImageManager& operator=(ImageManager&& other) = delete;
		ImageManager& operator=(const ImageManager& other) = delete;
		
		ImageHandle createImage(uint32_t width, uint32_t height, uint32_t depth, vk::Format format);
		
		[[nodiscard]]
		vk::Image getVulkanImage(const ImageHandle& handle) const;
		
		[[nodiscard]]
		vk::DeviceMemory getVulkanDeviceMemory(const ImageHandle& handle) const;
		
		[[nodiscard]]
		vk::ImageView getVulkanImageView(const ImageHandle& handle) const;
		
		void switchImageLayout(const ImageHandle& handle, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
		void fillImage(const ImageHandle& handle, void* data, size_t size);

		/**
		 * Destroys and deallocates image represented by a given
		 * buffer handle.
		 *
		 * @param handle Image handle
		 */
		void destroyImage(const ImageHandle& handle);
		
	};
}