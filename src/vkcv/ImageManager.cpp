/**
 * @authors Lars Hoerttrich
 * @file vkcv/ImageManager.cpp
 * @brief class creating and managing images
 */
#include "ImageManager.hpp"
#include "vkcv/Core.hpp"
#include "ImageLayoutTransitions.hpp"
#include "vkcv/Logger.hpp"

#include <algorithm>

namespace vkcv {

	ImageManager::Image::Image(
		vk::Image                   handle,
		vk::DeviceMemory            memory,
		std::vector<vk::ImageView>  views,
		uint32_t                    width,
		uint32_t                    height,
		uint32_t                    depth,
		vk::Format                  format,
		uint32_t                    layers)
		:
		m_handle(handle),
		m_memory(memory),
        m_viewPerMip(views),
		m_width(width),
		m_height(height),
		m_depth(depth),
		m_format(format),
		m_layers(layers)
	{}

	/**
	 * @brief searches memory type index for image allocation, combines requirements of image and application
	 * @param physicalMemoryProperties Memory Properties of physical device
	 * @param typeBits Bit field for suitable memory types
	 * @param requirements Property flags that are required
	 * @return memory type index for image
	 */
	uint32_t searchImageMemoryType(const vk::PhysicalDeviceMemoryProperties& physicalMemoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirements) {
		const uint32_t memoryCount = physicalMemoryProperties.memoryTypeCount;
		for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex) {
			const uint32_t memoryTypeBits = (1 << memoryIndex);
			const bool isRequiredMemoryType = typeBits & memoryTypeBits;

			const vk::MemoryPropertyFlags properties =
				physicalMemoryProperties.memoryTypes[memoryIndex].propertyFlags;
			const bool hasRequiredProperties =
				(properties & requirements) == requirements;

			if (isRequiredMemoryType && hasRequiredProperties)
				return static_cast<int32_t>(memoryIndex);
		}

		// failed to find memory type
		return -1;
	}

	ImageManager::ImageManager(BufferManager& bufferManager) noexcept :
		m_core(nullptr), m_bufferManager(bufferManager), m_images()
	{
	}

	ImageManager::~ImageManager() noexcept {
		for (uint64_t id = 0; id < m_images.size(); id++) {
			destroyImageById(id);
		}
		for (const auto swapchainImage : m_swapchainImages) {
			for (const auto view : swapchainImage.m_viewPerMip) {
				m_core->getContext().getDevice().destroy(view);
			}
		}
	}
	
	bool isDepthImageFormat(vk::Format format) {
		if ((format == vk::Format::eD16Unorm) || (format == vk::Format::eD16UnormS8Uint) ||
			(format == vk::Format::eD24UnormS8Uint) || (format == vk::Format::eD32Sfloat) ||
			(format == vk::Format::eD32SfloatS8Uint)) {
			return true;
		} else {
			return false;
		}
	}

	ImageHandle ImageManager::createImage(
		uint32_t    width, 
		uint32_t    height, 
		uint32_t    depth, 
		vk::Format  format, 
		uint32_t    mipCount,
		bool        supportStorage, 
		bool        supportColorAttachment)
	{
		const vk::PhysicalDevice& physicalDevice = m_core->getContext().getPhysicalDevice();
		
		const vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);
		
		vk::ImageCreateFlags createFlags;
		vk::ImageUsageFlags imageUsageFlags = (
				vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc
		);
		if (supportStorage) {
			imageUsageFlags |= vk::ImageUsageFlagBits::eStorage;
		}
		if (supportColorAttachment) {
			imageUsageFlags |= vk::ImageUsageFlagBits::eColorAttachment;
		}
		
		const bool isDepthFormat = isDepthImageFormat(format);
		
		if (isDepthFormat) {
			imageUsageFlags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
		}

		const vk::Device& device = m_core->getContext().getDevice();

		vk::ImageType imageType = vk::ImageType::e3D;
		vk::ImageViewType imageViewType = vk::ImageViewType::e3D;
		
		if (depth <= 1) {
			if (height <= 1) {
				imageType = vk::ImageType::e1D;
				imageViewType = vk::ImageViewType::e1D;
			} else {
				imageType = vk::ImageType::e2D;
				imageViewType = vk::ImageViewType::e2D;
			}
		}
		
		if (isDepthFormat) {
			imageType = vk::ImageType::e2D;
			imageViewType = vk::ImageViewType::e2D;
		}
		
		vk::ImageTiling imageTiling = vk::ImageTiling::eOptimal;
		
		if (!formatProperties.optimalTilingFeatures) {
			if (!formatProperties.linearTilingFeatures)
				return ImageHandle();
			
			imageTiling = vk::ImageTiling::eLinear;
		}
		
		const vk::ImageFormatProperties imageFormatProperties = 
			physicalDevice.getImageFormatProperties(format, imageType, imageTiling, imageUsageFlags);
		
		const uint32_t arrayLayers = std::min<uint32_t>(1, imageFormatProperties.maxArrayLayers);
		
		const vk::ImageCreateInfo imageCreateInfo(
			createFlags,
			imageType,
			format,
			vk::Extent3D(width, height, depth),
			mipCount,
			arrayLayers,
			vk::SampleCountFlagBits::e1,
			imageTiling,
			imageUsageFlags,
			vk::SharingMode::eExclusive,
			{},
			vk::ImageLayout::eUndefined
		);

		vk::Image image = device.createImage(imageCreateInfo);
		
		const vk::MemoryRequirements requirements = device.getImageMemoryRequirements(image);

		vk::MemoryPropertyFlags memoryTypeFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

		const uint32_t memoryTypeIndex = searchImageMemoryType(
			physicalDevice.getMemoryProperties(),
			requirements.memoryTypeBits,
			memoryTypeFlags
		);

		vk::DeviceMemory memory = device.allocateMemory(vk::MemoryAllocateInfo(requirements.size, memoryTypeIndex));
		device.bindImageMemory(image, memory, 0);

		vk::ImageAspectFlags aspectFlags;
		
		if (isDepthFormat) {
			aspectFlags = vk::ImageAspectFlagBits::eDepth;
		} else {
			aspectFlags = vk::ImageAspectFlagBits::eColor;
		}
		
		std::vector<vk::ImageView> views;
		for (int mip = 0; mip < mipCount; mip++) {
			const vk::ImageViewCreateInfo imageViewCreateInfo(
				{},
				image,
				imageViewType,
				format,
				vk::ComponentMapping(
					vk::ComponentSwizzle::eIdentity,
					vk::ComponentSwizzle::eIdentity,
					vk::ComponentSwizzle::eIdentity,
					vk::ComponentSwizzle::eIdentity
				),
				vk::ImageSubresourceRange(
					aspectFlags,
					mip,
					mipCount - mip,
					0,
					arrayLayers
				)
			);

			views.push_back(device.createImageView(imageViewCreateInfo));
		}
		
		const uint64_t id = m_images.size();
		m_images.push_back(Image(image, memory, views, width, height, depth, format, arrayLayers));
		return ImageHandle(id, [&](uint64_t id) { destroyImageById(id); });
	}
	
	ImageHandle ImageManager::createSwapchainImage() {
		return ImageHandle::createSwapchainImageHandle();
	}
	
	vk::Image ImageManager::getVulkanImage(const ImageHandle &handle) const {

		if (handle.isSwapchainImage()) {
			m_swapchainImages[m_currentSwapchainInputImage].m_handle;
		}

		const uint64_t id = handle.getId();
		if (id >= m_images.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return nullptr;
		}
		
		auto& image = m_images[id];
		
		return image.m_handle;
	}
	
	vk::DeviceMemory ImageManager::getVulkanDeviceMemory(const ImageHandle &handle) const {

		if (handle.isSwapchainImage()) {
			vkcv_log(LogLevel::ERROR, "Swapchain image has no memory");
			return nullptr;
		}

		const uint64_t id = handle.getId();
		if (id >= m_images.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return nullptr;
		}
		
		auto& image = m_images[id];
		
		return image.m_memory;
	}
	
	vk::ImageView ImageManager::getVulkanImageView(const ImageHandle &handle, const size_t mipLevel) const {
		
		if (handle.isSwapchainImage()) {
			return m_swapchainImages[m_currentSwapchainInputImage].m_viewPerMip[0];
		}

		const uint64_t id = handle.getId();
		if (id >= m_images.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return nullptr;
		}
		
		const auto& image = m_images[id];

		if (mipLevel >= m_images.size()) {
			vkcv_log(LogLevel::ERROR, "Image does not have requested mipLevel");
			return nullptr;
		}

		return image.m_viewPerMip[mipLevel];
	}
	
	void ImageManager::switchImageLayoutImmediate(const ImageHandle& handle, vk::ImageLayout newLayout) {
		uint64_t id = handle.getId();
		
		const bool isSwapchainImage = handle.isSwapchainImage();

		if (id >= m_images.size() && !isSwapchainImage) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return;
		}
		
		auto& image = isSwapchainImage ? m_swapchainImages[m_currentSwapchainInputImage] : m_images[id];
		const auto transitionBarrier = createImageLayoutTransitionBarrier(image, newLayout);
		
		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Graphics;
		
		m_core->recordAndSubmitCommandsImmediate(
			submitInfo,
			[transitionBarrier](const vk::CommandBuffer& commandBuffer) {
			// TODO: precise PipelineStageFlagBits, will require a lot of context
			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eBottomOfPipe,
				{},
				nullptr,
				nullptr,
				transitionBarrier
				);
			},
			nullptr);
		image.m_layout = newLayout;
	}

	void ImageManager::recordImageLayoutTransition(
		const ImageHandle& handle, 
		vk::ImageLayout newLayout, 
		vk::CommandBuffer cmdBuffer) {

		const uint64_t id = handle.getId();
		const bool isSwapchainImage = handle.isSwapchainImage();

		if (id >= m_images.size() && !isSwapchainImage) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return;
		}

		auto& image = isSwapchainImage ? m_swapchainImages[m_currentSwapchainInputImage] : m_images[id];
		const auto transitionBarrier = createImageLayoutTransitionBarrier(image, newLayout);
		recordImageBarrier(cmdBuffer, transitionBarrier);
		image.m_layout = newLayout;
	}

	void ImageManager::recordImageMemoryBarrier(
		const ImageHandle& handle,
		vk::CommandBuffer cmdBuffer) {

		const uint64_t id = handle.getId();
		const bool isSwapchainImage = handle.isSwapchainImage();

		if (id >= m_images.size() && !isSwapchainImage) {
			std::cerr << "Error: ImageManager::recordImageMemoryBarrier invalid handle" << std::endl;
			return;
		}

		auto& image = isSwapchainImage ? m_swapchainImages[m_currentSwapchainInputImage] : m_images[id];
		const auto transitionBarrier = createImageLayoutTransitionBarrier(image, image.m_layout);
		recordImageBarrier(cmdBuffer, transitionBarrier);
	}
	
	constexpr uint32_t getChannelsByFormat(vk::Format format) {
		switch (format) {
			case vk::Format::eR8Unorm:
				return 1;
			case vk::Format::eR8G8B8A8Srgb:
				return 4;
			default:
				std::cerr << "Check format instead of guessing, please!" << std::endl;
				return 4;
		}
	}
	
	void ImageManager::fillImage(const ImageHandle& handle, void* data, size_t size)
	{
		const uint64_t id = handle.getId();
		
		if (handle.isSwapchainImage()) {
			vkcv_log(LogLevel::ERROR, "Swapchain image cannot be filled");
			return;
		}

		if (id >= m_images.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return;
		}
		
		auto& image = m_images[id];
		
		switchImageLayoutImmediate(
				handle,
				vk::ImageLayout::eTransferDstOptimal);
		
		uint32_t channels = getChannelsByFormat(image.m_format);
		const size_t image_size = (
				image.m_width * image.m_height * image.m_depth * channels
		);
		
		const size_t max_size = std::min(size, image_size);
		
		BufferHandle bufferHandle = m_bufferManager.createBuffer(
				BufferType::STAGING, max_size, BufferMemoryType::HOST_VISIBLE
		);
		
		m_bufferManager.fillBuffer(bufferHandle, data, max_size, 0);
		
		vk::Buffer stagingBuffer = m_bufferManager.getBuffer(bufferHandle);
		
		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Transfer;
		
		m_core->recordAndSubmitCommandsImmediate(
				submitInfo,
				[&image, &stagingBuffer](const vk::CommandBuffer& commandBuffer) {
					vk::ImageAspectFlags aspectFlags;
					
					if (isDepthImageFormat(image.m_format)) {
						aspectFlags = vk::ImageAspectFlagBits::eDepth;
					} else {
						aspectFlags = vk::ImageAspectFlagBits::eColor;
					}
					
					const vk::BufferImageCopy region (
							0,
							0,
							0,
							vk::ImageSubresourceLayers(
									aspectFlags,
									0,
									0,
									image.m_layers
							),
							vk::Offset3D(0, 0, 0),
							vk::Extent3D(image.m_width, image.m_height, image.m_depth)
					);
					
					commandBuffer.copyBufferToImage(
							stagingBuffer,
							image.m_handle,
							vk::ImageLayout::eTransferDstOptimal,
							1,
							&region
					);
				},
				[&]() {
					switchImageLayoutImmediate(
							handle,
							vk::ImageLayout::eShaderReadOnlyOptimal
					);
				}
		);
	}

	void ImageManager::generateImageMipChainImmediate(const ImageHandle& handle) {

		const auto& device = m_core->getContext().getDevice();

		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Transfer;

		if (handle.isSwapchainImage()) {
			vkcv_log(vkcv::LogLevel::ERROR, "You cannot generate a mip chain for the swapchain, what are you smoking?");
			return;
		}

		const auto id = handle.getId();
		if (id >= m_images.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid image handle");
			return;
		}
		auto& image = m_images[id];
		switchImageLayoutImmediate(handle, vk::ImageLayout::eGeneral);

		const auto record = [&image, this, handle](const vk::CommandBuffer cmdBuffer) {

			vk::ImageAspectFlags aspectMask = isDepthImageFormat(image.m_format) ?
				vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;

			uint32_t srcWidth   = image.m_width;
			uint32_t srcHeight  = image.m_height;
			uint32_t srcDepth   = image.m_depth;

			auto half = [](uint32_t in) {
				return std::max(in / 2, (uint32_t)1);
			};

			uint32_t dstWidth   = half(image.m_width);
			uint32_t dstHeight  = half(image.m_height);
			uint32_t dstDepth   = half(image.m_depth);

			for (uint32_t srcMip = 0; srcMip < image.m_viewPerMip.size() - 1; srcMip++) {
				uint32_t dstMip = srcMip + 1;
				vk::ImageBlit region(
					vk::ImageSubresourceLayers(aspectMask, srcMip, 0, 1),
					{ vk::Offset3D(0, 0, 0), vk::Offset3D(srcWidth, srcHeight, srcDepth) },
					vk::ImageSubresourceLayers(aspectMask, dstMip, 0, 1),
					{ vk::Offset3D(0, 0, 0), vk::Offset3D(dstWidth, dstHeight, dstDepth) });

				cmdBuffer.blitImage(
					image.m_handle,
					vk::ImageLayout::eGeneral,
					image.m_handle,
					vk::ImageLayout::eGeneral,
					region,
					vk::Filter::eLinear);

				srcWidth    = dstWidth;
				srcHeight   = dstHeight;
				srcDepth    = dstDepth;

				dstWidth    = half(dstWidth);
				dstHeight   = half(dstHeight);
				dstDepth    = half(dstDepth);
			}
		};

		m_core->recordAndSubmitCommandsImmediate(submitInfo, record, nullptr);
	}

	uint32_t ImageManager::getImageWidth(const ImageHandle &handle) const {
		const uint64_t id = handle.getId();
		const bool isSwapchainImage = handle.isSwapchainImage();

		if (id >= m_images.size() && !isSwapchainImage) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return 0;
		}
		
		auto& image = isSwapchainImage ? m_swapchainImages[m_currentSwapchainInputImage] : m_images[id];
		
		return image.m_width;
	}
	
	uint32_t ImageManager::getImageHeight(const ImageHandle &handle) const {
		const uint64_t id = handle.getId();
		const bool isSwapchainImage = handle.isSwapchainImage();
		
		if (id >= m_images.size() && !isSwapchainImage) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return 0;
		}
		
		auto& image = isSwapchainImage ? m_swapchainImages[m_currentSwapchainInputImage] : m_images[id];
		
		return image.m_height;
	}
	
	uint32_t ImageManager::getImageDepth(const ImageHandle &handle) const {
		const uint64_t id = handle.getId();
		const bool isSwapchainImage = handle.isSwapchainImage();

		if (id >= m_images.size() && !isSwapchainImage) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return 0;
		}
		
		auto& image = isSwapchainImage ? m_swapchainImages[m_currentSwapchainInputImage] : m_images[id];
		
		return image.m_depth;
	}
	
	void ImageManager::destroyImageById(uint64_t id)
	{
		if (id >= m_images.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return;
		}
		
		auto& image = m_images[id];

		const vk::Device& device = m_core->getContext().getDevice();
		
		for (auto& view : image.m_viewPerMip) {
			if (view) {
				device.destroyImageView(view);
				view = nullptr;
			}
		}

		if (image.m_memory) {
			device.freeMemory(image.m_memory);
			image.m_memory = nullptr;
		}

		if (image.m_handle) {
			device.destroyImage(image.m_handle);
			image.m_handle = nullptr;
		}
	}

	vk::Format ImageManager::getImageFormat(const ImageHandle& handle) const {

		const uint64_t id = handle.getId();
		const bool isSwapchainFormat = handle.isSwapchainImage();

		if (id >= m_images.size() && !isSwapchainFormat) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return vk::Format::eUndefined;
		}

		return isSwapchainFormat ? m_swapchainImages[m_currentSwapchainInputImage].m_format : m_images[id].m_format;
	}

	void ImageManager::setCurrentSwapchainImageIndex(int index) {
		m_currentSwapchainInputImage = index;
	}

	void ImageManager::setSwapchainImages(const std::vector<vk::Image>& images, std::vector<vk::ImageView> views, 
		uint32_t width, uint32_t height, vk::Format format) {

		// destroy old views
		for (auto image : m_swapchainImages) {
			for (const auto& view : image.m_viewPerMip) {
				m_core->getContext().getDevice().destroyImageView(view);
			}
		}

		assert(images.size() == views.size());
		m_swapchainImages.clear();
		for (int i = 0; i < images.size(); i++) {
			m_swapchainImages.push_back(Image(images[i], nullptr, { views[i] }, width, height, 1, format, 1));
		}
	}

}