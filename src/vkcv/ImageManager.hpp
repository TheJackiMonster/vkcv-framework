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
	public:
		struct Image
		{
			vk::Image           m_handle;
			vk::DeviceMemory    m_memory;
			vk::ImageView       m_view;
			uint32_t            m_width = 0;
			uint32_t            m_height = 0;
			uint32_t            m_depth = 0;
			vk::Format          m_format;
			uint32_t            m_layers = 1;
			uint32_t            m_levels = 1;
			vk::ImageLayout     m_layout = vk::ImageLayout::eUndefined;
		private:
			// struct is public so utility functions can access members, but only ImageManager can create Image
			friend ImageManager;
			Image(
				vk::Image           handle,
				vk::DeviceMemory    memory,
				vk::ImageView       view,
				uint32_t            width,
				uint32_t            height,
				uint32_t            depth,
				vk::Format          format,
				uint32_t            layers,
				uint32_t            levels);
		};
	private:
		
		Core* m_core;
		BufferManager& m_bufferManager;
		
		std::vector<Image> m_images;
		
		ImageManager(BufferManager& bufferManager) noexcept;
		
		/**
		 * Destroys and deallocates image represented by a given
		 * image handle id.
		 *
		 * @param id Image handle id
		 */
		void destroyImageById(uint64_t id);
		
	public:
		~ImageManager() noexcept;
		ImageManager(ImageManager&& other) = delete;
		ImageManager(const ImageManager& other) = delete;

		ImageManager& operator=(ImageManager&& other) = delete;
		ImageManager& operator=(const ImageManager& other) = delete;
		
		ImageHandle createImage(uint32_t width, uint32_t height, uint32_t depth, vk::Format format, bool supportStorage, bool supportColorAttachment);
		
		ImageHandle createSwapchainImage();
		
		[[nodiscard]]
		vk::Image getVulkanImage(const ImageHandle& handle) const;
		
		[[nodiscard]]
		vk::DeviceMemory getVulkanDeviceMemory(const ImageHandle& handle) const;
		
		[[nodiscard]]
		vk::ImageView getVulkanImageView(const ImageHandle& handle) const;

		void switchImageLayoutImmediate(const ImageHandle& handle, vk::ImageLayout newLayout);
		void recordImageLayoutTransition(
			const ImageHandle& handle, 
			vk::ImageLayout newLayout, 
			vk::CommandBuffer cmdBuffer);

		void fillImage(const ImageHandle& handle, void* data, size_t size);
		
		[[nodiscard]]
		uint32_t getImageWidth(const ImageHandle& handle) const;
		
		[[nodiscard]]
		uint32_t getImageHeight(const ImageHandle& handle) const;
		
		[[nodiscard]]
		uint32_t getImageDepth(const ImageHandle& handle) const;
		
		[[nodiscard]]
		vk::Format getImageFormat(const ImageHandle& handle) const;
	};
}