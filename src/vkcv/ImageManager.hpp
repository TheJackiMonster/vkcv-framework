#pragma once
/**
 * @authors Lars Hoerttrich
 * @file vkcv/ImageManager.hpp
 * @brief class creating and managing images
 */
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

#include "vkcv/BufferManager.hpp"
#include "vkcv/Handles.hpp"
#include "vkcv/ImageConfig.hpp"

namespace vkcv {
	
	bool isDepthImageFormat(vk::Format format);

	class ImageManager
	{
		friend class Core;
	public:
		struct Image
		{
			vk::Image                   m_handle;
			vma::Allocation             m_allocation;
			std::vector<vk::ImageView>  m_viewPerMip;
			uint32_t                    m_width;
			uint32_t                    m_height;
			uint32_t                    m_depth;
			vk::Format                  m_format;
			uint32_t                    m_layers;
			vk::ImageLayout             m_layout;
		private:
			friend ImageManager;
		};
	private:
		
		Core* m_core;
		BufferManager& m_bufferManager;
		
		std::vector<Image> m_images;
		std::vector<Image> m_swapchainImages;
		int m_currentSwapchainInputImage;
		
		explicit ImageManager(BufferManager& bufferManager) noexcept;
		
		/**
		 * Destroys and deallocates image represented by a given
		 * image handle id.
		 *
		 * @param id Image handle id
		 */
		void destroyImageById(uint64_t id);

		void recordImageMipGenerationToCmdBuffer(vk::CommandBuffer cmdBuffer, const ImageHandle& handle);

	public:
		~ImageManager() noexcept;
		ImageManager(ImageManager&& other) = delete;
		ImageManager(const ImageManager& other) = delete;

		ImageManager& operator=(ImageManager&& other) = delete;
		ImageManager& operator=(const ImageManager& other) = delete;
		
		ImageHandle createImage(
			uint32_t        width, 
			uint32_t        height, 
			uint32_t        depth, 
			vk::Format      format, 
			uint32_t        mipCount,
			bool            supportStorage, 
			bool            supportColorAttachment,
			Multisampling   msaa);
		
		[[nodiscard]]
		ImageHandle createSwapchainImage() const;
		
		[[nodiscard]]
		vk::Image getVulkanImage(const ImageHandle& handle) const;
		
		[[nodiscard]]
		vk::DeviceMemory getVulkanDeviceMemory(const ImageHandle& handle) const;
		
		[[nodiscard]]
		vk::ImageView getVulkanImageView(const ImageHandle& handle, size_t mipLevel = 0) const;

		void switchImageLayoutImmediate(const ImageHandle& handle, vk::ImageLayout newLayout);
		void recordImageLayoutTransition(
			const ImageHandle& handle, 
			vk::ImageLayout newLayout, 
			vk::CommandBuffer cmdBuffer);

		void recordImageMemoryBarrier(
			const ImageHandle& handle,
			vk::CommandBuffer cmdBuffer);

		void fillImage(const ImageHandle& handle, const void* data, size_t size);
		void generateImageMipChainImmediate(const ImageHandle& handle);
		void recordImageMipChainGenerationToCmdStream(const vkcv::CommandStreamHandle& cmdStream, const ImageHandle& handle);
		void recordMSAAResolve(vk::CommandBuffer cmdBuffer, ImageHandle src, ImageHandle dst);

		[[nodiscard]]
		uint32_t getImageWidth(const ImageHandle& handle) const;
		
		[[nodiscard]]
		uint32_t getImageHeight(const ImageHandle& handle) const;
		
		[[nodiscard]]
		uint32_t getImageDepth(const ImageHandle& handle) const;
		
		[[nodiscard]]
		vk::Format getImageFormat(const ImageHandle& handle) const;

		[[nodiscard]]
		uint32_t getImageMipCount(const ImageHandle& handle) const;

		void setCurrentSwapchainImageIndex(int index);
		
		void setSwapchainImages(const std::vector<vk::Image>& images, const std::vector<vk::ImageView>& views,
								uint32_t width, uint32_t height, vk::Format format);

		// if manual vulkan work, e.g. ImGui integration, changes an image layout this function must be used
		// to update the internal image state
		void updateImageLayoutManual(const vkcv::ImageHandle& handle, const vk::ImageLayout layout);

	};
}