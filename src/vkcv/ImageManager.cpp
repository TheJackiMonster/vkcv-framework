/**
 * @authors Lars Hoerttrich
 * @file vkcv/ImageManager.cpp
 * @brief class creating and managing images
 */
#include "ImageManager.hpp"
#include "vkcv/Core.hpp"
#include "vkcv/Logger.hpp"

#include <algorithm>

namespace vkcv {

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
		
		for (const auto& swapchainImage : m_swapchainImages) {
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
		uint32_t        width, 
		uint32_t        height, 
		uint32_t        depth, 
		vk::Format      format, 
		uint32_t        mipCount,
		bool            supportStorage, 
		bool            supportColorAttachment,
		Multisampling   msaa)
	{
		const vk::PhysicalDevice& physicalDevice = m_core->getContext().getPhysicalDevice();
		
		const vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);
		
		vk::ImageCreateFlags createFlags;
		vk::ImageUsageFlags imageUsageFlags = (
				vk::ImageUsageFlagBits::eSampled |
				vk::ImageUsageFlagBits::eTransferDst |
				vk::ImageUsageFlagBits::eTransferSrc
		);
		
		vk::ImageTiling imageTiling = vk::ImageTiling::eOptimal;
		
		if (supportStorage) {
			imageUsageFlags |= vk::ImageUsageFlagBits::eStorage;
			
			if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eStorageImage)) {
				imageTiling = vk::ImageTiling::eLinear;
				
				if (!(formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eStorageImage))
					return ImageHandle();
			}
		}
		
		if (supportColorAttachment) {
			imageUsageFlags |= vk::ImageUsageFlagBits::eColorAttachment;
		}
		
		const bool isDepthFormat = isDepthImageFormat(format);
		
		if (isDepthFormat) {
			imageUsageFlags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
		}

		const vma::Allocator& allocator = m_core->getContext().getAllocator();

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
		
		if (vk::ImageType::e3D == imageType) {
			createFlags |= vk::ImageCreateFlagBits::e2DArrayCompatible;
		}
		
		if (!formatProperties.optimalTilingFeatures) {
			if (!formatProperties.linearTilingFeatures)
				return ImageHandle();
			
			imageTiling = vk::ImageTiling::eLinear;
		}
		
		const vk::ImageFormatProperties imageFormatProperties = physicalDevice.getImageFormatProperties(
				format,
				imageType,
				imageTiling,
				imageUsageFlags
		);
		
		const uint32_t arrayLayers = std::min<uint32_t>(1, imageFormatProperties.maxArrayLayers);

		vk::SampleCountFlagBits sampleCountFlag = msaaToVkSampleCountFlag(msaa);

		const vk::ImageCreateInfo imageCreateInfo (
			createFlags,
			imageType,
			format,
			vk::Extent3D(width, height, depth),
			mipCount,
			arrayLayers,
			sampleCountFlag,
			imageTiling,
			imageUsageFlags,
			vk::SharingMode::eExclusive,
			{},
			vk::ImageLayout::eUndefined
		);
		
		auto imageAllocation = allocator.createImage(
				imageCreateInfo,
				vma::AllocationCreateInfo(
						vma::AllocationCreateFlags(),
						vma::MemoryUsage::eGpuOnly,
						vk::MemoryPropertyFlagBits::eDeviceLocal,
						vk::MemoryPropertyFlagBits::eDeviceLocal,
						0,
						vma::Pool(),
						nullptr
				)
		);

		vk::Image image = imageAllocation.first;
		vma::Allocation allocation = imageAllocation.second;

		vk::ImageAspectFlags aspectFlags;
		
		if (isDepthFormat) {
			aspectFlags = vk::ImageAspectFlagBits::eDepth;
		} else {
			aspectFlags = vk::ImageAspectFlagBits::eColor;
		}
		
		const vk::Device& device = m_core->getContext().getDevice();
		
		std::vector<vk::ImageView> views;
		std::vector<vk::ImageView> arrayViews;
		
		for (uint32_t mip = 0; mip < mipCount; mip++) {
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
		
		for (uint32_t mip = 0; mip < mipCount; mip++) {
			const vk::ImageViewCreateInfo imageViewCreateInfo(
					{},
					image,
					vk::ImageViewType::e2DArray,
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
							1,
							0,
							arrayLayers
					)
			);
			
			arrayViews.push_back(device.createImageView(imageViewCreateInfo));
		}
		
		const uint64_t id = m_images.size();
		m_images.push_back({
			image,
			allocation,
			
			views,
			arrayViews,
			
			width,
			height,
			depth,
			
			format,
			arrayLayers,
			vk::ImageLayout::eUndefined,
			supportStorage
		});
		
		return ImageHandle(id, [&](uint64_t id) { destroyImageById(id); });
	}
	
	ImageHandle ImageManager::createSwapchainImage() const {
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
		
		const vma::Allocator& allocator = m_core->getContext().getAllocator();
		
		auto info = allocator.getAllocationInfo(
				image.m_allocation
		);
		
		return info.deviceMemory;
	}
	
	vk::ImageView ImageManager::getVulkanImageView(const ImageHandle &handle,
												   size_t mipLevel,
												   bool arrayView) const {
		
		if (handle.isSwapchainImage()) {
			return m_swapchainImages[m_currentSwapchainInputImage].m_viewPerMip[0];
		}

		const uint64_t id = handle.getId();
		if (id >= m_images.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return nullptr;
		}
		
		const auto& image = m_images[id];
		const auto& views = arrayView? image.m_arrayViewPerMip : image.m_viewPerMip;
		
		if (mipLevel >= views.size()) {
			vkcv_log(LogLevel::ERROR, "Image does not have requested mipLevel");
			return nullptr;
		}
		
		return views[mipLevel];
	}
	
	static vk::ImageMemoryBarrier createImageLayoutTransitionBarrier(const ImageManager::Image &image,
																	 uint32_t mipLevelCount,
																	 uint32_t mipLevelOffset,
																	 vk::ImageLayout newLayout) {
		vk::ImageAspectFlags aspectFlags;
		if (isDepthFormat(image.m_format)) {
			aspectFlags = vk::ImageAspectFlagBits::eDepth;
		} else {
			aspectFlags = vk::ImageAspectFlagBits::eColor;
		}
		
		const uint32_t mipLevelsMax = image.m_viewPerMip.size();
		
		if (mipLevelOffset > mipLevelsMax)
			mipLevelOffset = mipLevelsMax;
		
		if ((!mipLevelCount) || (mipLevelOffset + mipLevelCount > mipLevelsMax))
			mipLevelCount = mipLevelsMax - mipLevelOffset;
		
		vk::ImageSubresourceRange imageSubresourceRange(
				aspectFlags,
				mipLevelOffset,
				mipLevelCount,
				0,
				image.m_layers
		);
		
		// TODO: precise AccessFlagBits, will require a lot of context
		return vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eMemoryWrite,
				vk::AccessFlagBits::eMemoryRead,
				image.m_layout,
				newLayout,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				image.m_handle,
				imageSubresourceRange
		);
	}
	
	void ImageManager::switchImageLayoutImmediate(const ImageHandle& handle, vk::ImageLayout newLayout) {
		uint64_t id = handle.getId();
		
		const bool isSwapchainImage = handle.isSwapchainImage();

		if (id >= m_images.size() && !isSwapchainImage) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return;
		}
		
		auto& image = isSwapchainImage ? m_swapchainImages[m_currentSwapchainInputImage] : m_images[id];
		const auto transitionBarrier = createImageLayoutTransitionBarrier(image, 0, 0, newLayout);
		
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
			nullptr
		);
		
		image.m_layout = newLayout;
	}

	void ImageManager::recordImageLayoutTransition(const ImageHandle& handle,
												   uint32_t mipLevelCount,
												   uint32_t mipLevelOffset,
												   vk::ImageLayout newLayout,
												   vk::CommandBuffer cmdBuffer) {
		const uint64_t id = handle.getId();
		const bool isSwapchainImage = handle.isSwapchainImage();

		if (id >= m_images.size() && !isSwapchainImage) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return;
		}

		auto& image = isSwapchainImage ? m_swapchainImages[m_currentSwapchainInputImage] : m_images[id];
		const auto transitionBarrier = createImageLayoutTransitionBarrier(
				image,
				mipLevelCount,
				mipLevelOffset,
				newLayout
		);
		
		cmdBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlagBits::eAllCommands,
				{},
				nullptr,
				nullptr,
				transitionBarrier
		);
		
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
		const auto transitionBarrier = createImageLayoutTransitionBarrier(image, 0, 0, image.m_layout);
		
		cmdBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlagBits::eAllCommands,
				{},
				nullptr,
				nullptr,
				transitionBarrier
		);
	}
	
	constexpr uint32_t getBytesPerPixel(vk::Format format) {
		switch (format) {
			case vk::Format::eR8Unorm:
				return 1;
			case vk::Format::eR8G8B8A8Srgb:
				return 4;
			case vk::Format::eR8G8B8A8Unorm:
				return 4;
			case vk::Format::eR16G16B16A16Sfloat:
				return 8;
			case vk::Format::eR32G32B32A32Sfloat:
				return 16;
			default:
				std::cerr << "Unknown image format" << std::endl;
				return 4;
		}
	}
	
	void ImageManager::fillImage(const ImageHandle& handle, const void* data, size_t size)
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
		
		const size_t image_size = (
				image.m_width * image.m_height * image.m_depth *
				getBytesPerPixel(image.m_format)
		);
		
		const size_t max_size = std::min(size, image_size);
		
		BufferHandle bufferHandle = m_bufferManager.createBuffer(
				BufferType::STAGING,
				max_size,
				BufferMemoryType::DEVICE_LOCAL,
				false,
				false
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

	void ImageManager::recordImageMipGenerationToCmdBuffer(vk::CommandBuffer cmdBuffer, const ImageHandle& handle) {

		const auto id = handle.getId();
		if (id >= m_images.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid image handle");
			return;
		}

		auto& image = m_images[id];
		recordImageLayoutTransition(handle, 0, 0, vk::ImageLayout::eGeneral, cmdBuffer);

		vk::ImageAspectFlags aspectMask = isDepthImageFormat(image.m_format) ?
			vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;

		uint32_t srcWidth = image.m_width;
		uint32_t srcHeight = image.m_height;
		uint32_t srcDepth = image.m_depth;

		auto half = [](uint32_t in) {
			return std::max<uint32_t>(in / 2, 1);
		};

		uint32_t dstWidth = half(srcWidth);
		uint32_t dstHeight = half(srcHeight);
		uint32_t dstDepth = half(srcDepth);

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

			srcWidth = dstWidth;
			srcHeight = dstHeight;
			srcDepth = dstDepth;

			dstWidth = half(dstWidth);
			dstHeight = half(dstHeight);
			dstDepth = half(dstDepth);

			recordImageMemoryBarrier(handle, cmdBuffer);
		}
	}

	void ImageManager::recordImageMipChainGenerationToCmdStream(
		const vkcv::CommandStreamHandle& cmdStream,
		const ImageHandle& handle) {

		const auto record = [this, handle](const vk::CommandBuffer cmdBuffer) {
			recordImageMipGenerationToCmdBuffer(cmdBuffer, handle);
		};
		m_core->recordCommandsToStream(cmdStream, record, nullptr);
	}

	void ImageManager::recordMSAAResolve(vk::CommandBuffer cmdBuffer, ImageHandle src, ImageHandle dst) {

		const uint64_t srcId = src.getId();
		const uint64_t dstId = dst.getId();

		const bool isSrcSwapchainImage = src.isSwapchainImage();
		const bool isDstSwapchainImage = dst.isSwapchainImage();

		const bool isSrcHandleInvalid = srcId >= m_images.size() && !isSrcSwapchainImage;
		const bool isDstHandleInvalid = dstId >= m_images.size() && !isDstSwapchainImage;

		if (isSrcHandleInvalid || isDstHandleInvalid) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return;
		}

		auto& srcImage = isSrcSwapchainImage ? m_swapchainImages[m_currentSwapchainInputImage] : m_images[srcId];
		auto& dstImage = isDstSwapchainImage ? m_swapchainImages[m_currentSwapchainInputImage] : m_images[dstId];

		vk::ImageResolve region(
			vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
			vk::Offset3D(0, 0, 0),
			vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
			vk::Offset3D(0, 0, 0), 
			vk::Extent3D(dstImage.m_width, dstImage.m_height, dstImage.m_depth));

		recordImageLayoutTransition(src, 0, 0, vk::ImageLayout::eTransferSrcOptimal, cmdBuffer);
		recordImageLayoutTransition(dst, 0, 0, vk::ImageLayout::eTransferDstOptimal, cmdBuffer);

		cmdBuffer.resolveImage(
			srcImage.m_handle,
			srcImage.m_layout,
			dstImage.m_handle,
			dstImage.m_layout,
			region);
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
		
		for (auto& view : image.m_arrayViewPerMip) {
			if (view) {
				device.destroyImageView(view);
				view = nullptr;
			}
		}
		
		const vma::Allocator& allocator = m_core->getContext().getAllocator();

		if (image.m_handle) {
			allocator.destroyImage(image.m_handle, image.m_allocation);
			
			image.m_handle = nullptr;
			image.m_allocation = nullptr;
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
	
	bool ImageManager::isImageSupportingStorage(const ImageHandle& handle) const {
		const uint64_t id = handle.getId();
		const bool isSwapchainFormat = handle.isSwapchainImage();
		
		if (isSwapchainFormat) {
			return false;
		}
		
		if (id >= m_images.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return false;
		}
		
		return m_images[id].m_storage;
	}

	uint32_t ImageManager::getImageMipCount(const ImageHandle& handle) const {
		const uint64_t id = handle.getId();

		if (handle.isSwapchainImage()) {
			return 1;
		}

		if (id >= m_images.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return 0;
		}

		return m_images[id].m_viewPerMip.size();
	}
	
	uint32_t ImageManager::getImageArrayLayers(const ImageHandle& handle) const {
		const uint64_t id = handle.getId();
		
		if (handle.isSwapchainImage()) {
			return m_swapchainImages[0].m_layers;
		}
		
		if (id >= m_images.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return 0;
		}
		
		return m_images[id].m_layers;
	}

	void ImageManager::setCurrentSwapchainImageIndex(int index) {
		m_currentSwapchainInputImage = index;
	}

	void ImageManager::setSwapchainImages(const std::vector<vk::Image>& images, const std::vector<vk::ImageView>& views,
		uint32_t width, uint32_t height, vk::Format format) {

		// destroy old views
		for (const auto& image : m_swapchainImages) {
			for (const auto& view : image.m_viewPerMip) {
				m_core->getContext().getDevice().destroyImageView(view);
			}
		}

		assert(images.size() == views.size());
		m_swapchainImages.clear();
		for (size_t i = 0; i < images.size(); i++) {
			m_swapchainImages.push_back({
				images[i],
				nullptr,
				{ views[i] },
				{},
				width,
				height,
				1,
				format,
				1,
				vk::ImageLayout::eUndefined,
				false
			});
		}
	}

	void ImageManager::updateImageLayoutManual(const vkcv::ImageHandle& handle, const vk::ImageLayout layout) {
		const uint64_t id = handle.getId();

		if (handle.isSwapchainImage()) {
			m_swapchainImages[m_currentSwapchainInputImage].m_layout = layout;
		}
		else {
			if (id >= m_images.size()) {
				vkcv_log(LogLevel::ERROR, "Invalid handle");
				return;
			}
			m_swapchainImages[id].m_layout = layout;
		}
		
	}

}