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
		vk::Image           handle,
		vk::DeviceMemory    memory,
		vk::ImageView       view,
		uint32_t            width,
		uint32_t            height,
		uint32_t            depth,
		vk::Format          format,
		uint32_t            layers,
		uint32_t            levels)
		:
		m_handle(handle),
		m_memory(memory),
		m_view(view),
		m_width(width),
		m_height(height),
		m_depth(depth),
		m_format(format),
		m_layers(layers),
		m_levels(levels)
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

	ImageHandle ImageManager::createImage(uint32_t width, uint32_t height, uint32_t depth, vk::Format format)
	{
		const vk::PhysicalDevice& physicalDevice = m_core->getContext().getPhysicalDevice();
		
		const vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);
		
		vk::ImageCreateFlags createFlags;
		vk::ImageUsageFlags imageUsageFlags = (
				vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst
		);
		
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
		
		const vk::ImageFormatProperties imageFormatProperties = physicalDevice.getImageFormatProperties(
				format, imageType, imageTiling, imageUsageFlags
		);
		
		const uint32_t mipLevels = std::min<uint32_t>(1, imageFormatProperties.maxMipLevels);
		const uint32_t arrayLayers = std::min<uint32_t>(1, imageFormatProperties.maxArrayLayers);
		
		const vk::ImageCreateInfo imageCreateInfo(
			createFlags,
			imageType,
			format,
			vk::Extent3D(width, height, depth),
			mipLevels,
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
		
		const vk::ImageViewCreateInfo imageViewCreateInfo (
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
						0,
						mipLevels,
						0,
						arrayLayers
				)
		);
		
		vk::ImageView view = device.createImageView(imageViewCreateInfo);
		
		const uint64_t id = m_images.size();
		m_images.push_back(Image(image, memory, view, width, height, depth, format, arrayLayers, mipLevels));
		return ImageHandle(id, [&](uint64_t id) { destroyImageById(id); });
	}
	
	ImageHandle ImageManager::createSwapchainImage() {
		return ImageHandle::createSwapchainImageHandle();
	}
	
	vk::Image ImageManager::getVulkanImage(const ImageHandle &handle) const {
		const uint64_t id = handle.getId();
		
		if (id >= m_images.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid handle");
			return nullptr;
		}
		
		auto& image = m_images[id];
		
		return image.m_handle;
	}
	
	vk::DeviceMemory ImageManager::getVulkanDeviceMemory(const ImageHandle &handle) const {
		const uint64_t id = handle.getId();
		
		if (id >= m_images.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid handle");
			return nullptr;
		}
		
		auto& image = m_images[id];
		
		return image.m_memory;
	}
	
	vk::ImageView ImageManager::getVulkanImageView(const ImageHandle &handle) const {
		const uint64_t id = handle.getId();
		
		if (id >= m_images.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid handle");
			return nullptr;
		}
		
		auto& image = m_images[id];
		
		return image.m_view;
	}
	
	void ImageManager::switchImageLayoutImmediate(const ImageHandle& handle, vk::ImageLayout newLayout) {
		const uint64_t id = handle.getId();
		
		if (id >= m_images.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid handle");
			return;
		}
		
		auto& image = m_images[id];
		const auto transitionBarrier = createImageLayoutTransitionBarrier(image, newLayout);
		
		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Graphics;
		
		m_core->recordAndSubmitCommands(
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

		if (id >= m_images.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid handle");
			return;
		}

		auto& image = m_images[id];
		const auto transitionBarrier = createImageLayoutTransitionBarrier(image, newLayout);
		recordImageBarrier(cmdBuffer, transitionBarrier);
		image.m_layout = newLayout;
	}
	
	void ImageManager::fillImage(const ImageHandle& handle, void* data, size_t size)
	{
		const uint64_t id = handle.getId();
		
		if (id >= m_images.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid handle");
			return;
		}
		
		auto& image = m_images[id];
		
		switchImageLayoutImmediate(
				handle,
				vk::ImageLayout::eTransferDstOptimal);
		
		uint32_t channels = 4; // TODO: check image.m_format
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
		
		m_core->recordAndSubmitCommands(
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
	
	uint32_t ImageManager::getImageWidth(const ImageHandle &handle) const {
		const uint64_t id = handle.getId();
		
		if (id >= m_images.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid handle");
			return 0;
		}
		
		auto& image = m_images[id];
		
		return image.m_width;
	}
	
	uint32_t ImageManager::getImageHeight(const ImageHandle &handle) const {
		const uint64_t id = handle.getId();
		
		if (id >= m_images.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid handle");
			return 0;
		}
		
		auto& image = m_images[id];
		
		return image.m_height;
	}
	
	uint32_t ImageManager::getImageDepth(const ImageHandle &handle) const {
		const uint64_t id = handle.getId();
		
		if (id >= m_images.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid handle");
			return 0;
		}
		
		auto& image = m_images[id];
		
		return image.m_depth;
	}
	
	void ImageManager::destroyImageById(uint64_t id)
	{
		if (id >= m_images.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid handle");
			return;
		}
		
		auto& image = m_images[id];

		const vk::Device& device = m_core->getContext().getDevice();
		
		if (image.m_view) {
			device.destroyImageView(image.m_view);
			image.m_view = nullptr;
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

		if (id >= m_images.size()) {
			vkcv_log(vkcv::LogLevel::ERROR, "Invalid handle");
			return vk::Format::eUndefined;
		}

		return m_images[id].m_format;
	}

}