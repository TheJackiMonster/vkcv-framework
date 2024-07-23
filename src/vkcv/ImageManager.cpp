/**
 * @authors Lars Hoerttrich, Tobias Frisch
 * @file vkcv/ImageManager.cpp
 * @brief class creating and managing images
 */
#include "ImageManager.hpp"
#include "vkcv/Core.hpp"
#include "vkcv/Image.hpp"
#include "vkcv/Logger.hpp"
#include "vkcv/Multisampling.hpp"
#include "vkcv/TypeGuard.hpp"

#include <algorithm>
#include <cstdint>
#include <sys/types.h>

namespace vkcv {
	
	bool ImageManager::init(Core &core, BufferManager &bufferManager) {
		if (!HandleManager<ImageEntry, ImageHandle>::init(core)) {
			return false;
		}
		
		m_bufferManager = &bufferManager;
		m_swapchainImages.clear();
		return true;
	}
	
	uint64_t ImageManager::getIdFrom(const ImageHandle &handle) const {
		return handle.getId();
	}
	
	ImageHandle ImageManager::createById(uint64_t id, const HandleDestroyFunction &destroy) {
		return ImageHandle(id, destroy);
	}
	
	void ImageManager::destroyById(uint64_t id) {
		auto &image = getById(id);
		
		const vk::Device &device = getCore().getContext().getDevice();
		
		for (auto &view : image.m_viewPerMip) {
			if (view) {
				device.destroyImageView(view);
				view = nullptr;
			}
		}
		
		for (auto &view : image.m_arrayViewPerMip) {
			if (view) {
				device.destroyImageView(view);
				view = nullptr;
			}
		}
		
		const vma::Allocator &allocator = getCore().getContext().getAllocator();
		
		if (image.m_handle) {
			allocator.destroyImage(image.m_handle, image.m_allocation);
			
			image.m_handle = nullptr;
			image.m_allocation = nullptr;
		}
	}
	
	const BufferManager &ImageManager::getBufferManager() const {
		return *m_bufferManager;
	}
	
	BufferManager &ImageManager::getBufferManager() {
		return *m_bufferManager;
	}
	
	void ImageManager::recordImageMipGenerationToCmdBuffer(vk::CommandBuffer cmdBuffer,
														   const ImageHandle &handle) {
		auto &image = (*this) [handle];

		vk::ImageAspectFlags aspectFlags;
		if (isDepthFormat(image.m_format)) {
			aspectFlags = vk::ImageAspectFlagBits::eDepth;
		} else {
			aspectFlags = vk::ImageAspectFlagBits::eColor;
		}
		
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
			const vk::ImageBlit region(
					vk::ImageSubresourceLayers(aspectFlags, srcMip, 0, image.m_layers.size()),
					{ vk::Offset3D(0, 0, 0), vk::Offset3D(srcWidth, srcHeight, srcDepth) },
					vk::ImageSubresourceLayers(aspectFlags, dstMip, 0, image.m_layers.size()),
					{ vk::Offset3D(0, 0, 0), vk::Offset3D(dstWidth, dstHeight, dstDepth) }
			);

			recordImageLayoutTransition(handle, 1, srcMip, vk::ImageLayout::eTransferSrcOptimal, cmdBuffer);
			recordImageLayoutTransition(handle, 1, dstMip, vk::ImageLayout::eTransferDstOptimal, cmdBuffer);
			
			cmdBuffer.blitImage(image.m_handle, image.m_layers[0].m_layouts[srcMip], image.m_handle,
								image.m_layers[0].m_layouts[dstMip], region, vk::Filter::eLinear);
			
			srcWidth = dstWidth;
			srcHeight = dstHeight;
			srcDepth = dstDepth;
			
			dstWidth = half(dstWidth);
			dstHeight = half(dstHeight);
			dstDepth = half(dstDepth);

			recordImageMemoryBarrier(handle, cmdBuffer);
		}
	}
	
	const ImageEntry &ImageManager::operator[](const ImageHandle &handle) const {
		if (handle.isSwapchainImage()) {
			return m_swapchainImages [m_currentSwapchainInputImage];
		}
		
		return HandleManager<ImageEntry, ImageHandle>::operator[](handle);
	}
	
	ImageEntry &ImageManager::operator[](const ImageHandle &handle) {
		if (handle.isSwapchainImage()) {
			return m_swapchainImages [m_currentSwapchainInputImage];
		}
		
		return HandleManager<ImageEntry, ImageHandle>::operator[](handle);
	}
	
	ImageManager::ImageManager() noexcept :
			HandleManager<ImageEntry, ImageHandle>(), m_bufferManager(nullptr), m_swapchainImages(),
			m_currentSwapchainInputImage(0) {}
	
	ImageManager::~ImageManager() noexcept {
		clear();
		
		for (const auto &swapchainImage : m_swapchainImages) {
			for (const auto view : swapchainImage.m_viewPerMip) {
				getCore().getContext().getDevice().destroy(view);
			}
		}
	}
	
	bool isDepthImageFormat(vk::Format format) {
		if ((format == vk::Format::eD16Unorm) || (format == vk::Format::eD16UnormS8Uint)
			|| (format == vk::Format::eD24UnormS8Uint) || (format == vk::Format::eD32Sfloat)
			|| (format == vk::Format::eD32SfloatS8Uint)) {
			return true;
		} else {
			return false;
		}
	}
	
	ImageHandle ImageManager::createImage(vk::Format format,
										  uint32_t mipCount,
										  const ImageConfig& config) {
		const vk::PhysicalDevice &physicalDevice = getCore().getContext().getPhysicalDevice();
		const vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);
		
		vk::ImageCreateFlags createFlags;
		vk::ImageUsageFlags imageUsageFlags = (
				vk::ImageUsageFlagBits::eSampled |
				vk::ImageUsageFlagBits::eTransferDst |
				vk::ImageUsageFlagBits::eTransferSrc
		);
		
		vk::ImageTiling imageTiling = vk::ImageTiling::eOptimal;
		
		if (config.isSupportingStorage()) {
			imageUsageFlags |= vk::ImageUsageFlagBits::eStorage;
			
			if (!(formatProperties.optimalTilingFeatures
				  & vk::FormatFeatureFlagBits::eStorageImage)) {
				imageTiling = vk::ImageTiling::eLinear;
				
				if (!(formatProperties.linearTilingFeatures
					  & vk::FormatFeatureFlagBits::eStorageImage))
					return {};
			}
		}
		
		if (config.isSupportingColorAttachment()) {
			imageUsageFlags |= vk::ImageUsageFlagBits::eColorAttachment;
		}
		
		const bool isDepthFormat = isDepthImageFormat(format);
		
		if (isDepthFormat) {
			imageUsageFlags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
		}
		
		const vma::Allocator &allocator = getCore().getContext().getAllocator();
		uint32_t requiredArrayLayers = 1;
		
		vk::ImageType imageType = vk::ImageType::e3D;
		vk::ImageViewType imageViewType = vk::ImageViewType::e3D;
		
		if (config.getDepth() <= 1) {
			if (config.getHeight() <= 1) {
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
		
		if (config.isCubeMapImage()) {
			requiredArrayLayers = 6;
			
			imageViewType = vk::ImageViewType::eCube;
			createFlags |= vk::ImageCreateFlagBits::eCubeCompatible;
		} else
		if (vk::ImageType::e3D == imageType) {
			createFlags |= vk::ImageCreateFlagBits::e2DArrayCompatible;
		}
		
		if (!formatProperties.optimalTilingFeatures) {
			if (!formatProperties.linearTilingFeatures)
				return {};
			
			imageTiling = vk::ImageTiling::eLinear;
		}
		
		const vk::ImageFormatProperties imageFormatProperties = (
				physicalDevice.getImageFormatProperties(
						format,
						imageType,
						imageTiling,
						imageUsageFlags
				)
		);
		
		const uint32_t arrayLayers = std::min<uint32_t>(
				requiredArrayLayers,
				imageFormatProperties.maxArrayLayers
		);
		
		const vk::ImageCreateInfo imageCreateInfo(
				createFlags,
				imageType,
				format,
				vk::Extent3D(
						config.getWidth(),
						config.getHeight(),
						config.getDepth()
				),
				mipCount,
				arrayLayers,
				msaaToSampleCountFlagBits(
						config.getMultisampling()
				),
				imageTiling,
				imageUsageFlags,
				vk::SharingMode::eExclusive,
				{},
				vk::ImageLayout::eUndefined
		);

		vma::AllocationCreateFlags allocationCreateFlags;
		if (getBufferManager().useResizableBar()) {
			allocationCreateFlags = vma::AllocationCreateFlagBits::eHostAccessAllowTransferInstead
									| vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;
		}
		
		const auto imageAllocation = allocator.createImage(
				imageCreateInfo,
				vma::AllocationCreateInfo(
						allocationCreateFlags,
						vma::MemoryUsage::eAutoPreferDevice,
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

		const vk::MemoryPropertyFlags finalMemoryFlags = allocator.getAllocationMemoryProperties(
				allocation
		);

		bool accessible = false;
		if (vk::MemoryPropertyFlagBits::eHostVisible & finalMemoryFlags) {
			const auto& featureManager = getCore().getContext().getFeatureManager();

			accessible = (
				featureManager.isExtensionActive(VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME) &&
				featureManager.checkFeatures<vk::PhysicalDeviceHostImageCopyFeaturesEXT>(
					vk::StructureType::ePhysicalDeviceHostImageCopyFeaturesEXT,
					[](const vk::PhysicalDeviceHostImageCopyFeaturesEXT& features) {
						return features.hostImageCopy;
					}
				)
			);
		}
		
		const vk::Device &device = getCore().getContext().getDevice();
		
		Vector<vk::ImageView> views;
		Vector<vk::ImageView> arrayViews;
		
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
		
		Vector<ImageLayer> layers;
		layers.resize(arrayLayers);

		for (auto& layer : layers) {
			layer.m_layouts.resize(mipCount, vk::ImageLayout::eUndefined);
		}
		
		return add({
			image,
			allocation,
			views,
			arrayViews,
			config.getWidth(),
			config.getHeight(),
			config.getDepth(),
			format,
			layers,
			config.isSupportingStorage(),
			accessible
		});
	}
	
	vk::Image ImageManager::getVulkanImage(const ImageHandle &handle) const {
		auto &image = (*this) [handle];
		return image.m_handle;
	}
	
	vk::DeviceMemory ImageManager::getVulkanDeviceMemory(const ImageHandle &handle) const {
		if (handle.isSwapchainImage()) {
			vkcv_log(LogLevel::ERROR, "Swapchain image has no memory");
			return nullptr;
		}
		
		auto &image = (*this) [handle];
		const vma::Allocator &allocator = getCore().getContext().getAllocator();
		
		auto info = allocator.getAllocationInfo(image.m_allocation);
		
		return info.deviceMemory;
	}
	
	vk::ImageView ImageManager::getVulkanImageView(const ImageHandle &handle, size_t mipLevel,
												   bool arrayView) const {
		if (handle.isSwapchainImage()) {
			return m_swapchainImages [m_currentSwapchainInputImage].m_viewPerMip [0];
		}
		
		const auto &image = (*this) [handle];
		const auto &views = arrayView ? image.m_arrayViewPerMip : image.m_viewPerMip;
		
		if (mipLevel >= views.size()) {
			vkcv_log(LogLevel::ERROR, "Image does not have requested mipLevel");
			return nullptr;
		}
		
		return views [mipLevel];
	}
	
	static Vector<vk::ImageMemoryBarrier> createImageLayoutTransitionBarriers(const ImageEntry &image,
																			  uint32_t mipLevelCount,
																			  uint32_t mipLevelOffset,
																			  vk::ImageLayout newLayout,
																			  bool keepOldLayout) {
		vk::ImageAspectFlags aspectFlags;
		if (isDepthFormat(image.m_format)) {
			aspectFlags = vk::ImageAspectFlagBits::eDepth;
		} else {
			aspectFlags = vk::ImageAspectFlagBits::eColor;
		}
		
		const uint32_t mipLevelsMax = image.m_viewPerMip.size();
		
		if (mipLevelOffset > mipLevelsMax) {
			mipLevelOffset = mipLevelsMax;
		}
		
		if ((!mipLevelCount) || (mipLevelOffset + mipLevelCount > mipLevelsMax)) {
			mipLevelCount = mipLevelsMax - mipLevelOffset;
		}
		
		if (mipLevelCount <= 0) {
			return {};
		}

		Vector<vk::ImageMemoryBarrier> barriers;
		vk::ImageLayout layout = vk::ImageLayout::eUndefined;
		uint32_t levels = 0;

		for (uint32_t i = 1; i <= mipLevelCount; i++) {
			if ((levels > 0) && (layout != image.m_layers[0].m_layouts[mipLevelOffset + mipLevelCount - i])) {
				const vk::ImageSubresourceRange imageSubresourceRange(
						aspectFlags,
						mipLevelOffset + mipLevelCount - (i - 1),
						levels,
						0,
						static_cast<uint32_t>(image.m_layers.size())
				);

				// TODO: precise AccessFlagBits, will require a lot of context
				barriers.emplace_back(
						vk::AccessFlagBits::eMemoryWrite,
						vk::AccessFlagBits::eMemoryRead,
						layout,
						keepOldLayout? layout : newLayout,
						VK_QUEUE_FAMILY_IGNORED,
						VK_QUEUE_FAMILY_IGNORED,
						image.m_handle,
						imageSubresourceRange
				);

				levels = 0;
			}

			layout = image.m_layers[0].m_layouts[mipLevelOffset + mipLevelCount - i];
			levels++;
		}
		
		if (levels > 0) {
			const vk::ImageSubresourceRange imageSubresourceRange(
					aspectFlags,
					mipLevelOffset,
					levels,
					0,
					static_cast<uint32_t>(image.m_layers.size())
			);

			// TODO: precise AccessFlagBits, will require a lot of context
			barriers.emplace_back(
					vk::AccessFlagBits::eMemoryWrite,
					vk::AccessFlagBits::eMemoryRead,
					layout,
					keepOldLayout? layout : newLayout,
					VK_QUEUE_FAMILY_IGNORED,
					VK_QUEUE_FAMILY_IGNORED,
					image.m_handle,
					imageSubresourceRange
			);
		}
		
		return barriers;
	}
	
	void ImageManager::switchImageLayoutImmediate(const ImageHandle &handle,
                                                vk::ImageLayout newLayout) {
		auto &image = (*this) [handle];
		const auto transitionBarriers = createImageLayoutTransitionBarriers(image, 0, 0, newLayout, false);
		
		auto &core = getCore();

		if (image.m_accessible) {
			const auto &dynamicDispatch = getCore().getContext().getDispatchLoaderDynamic();

			for (const auto& barrier : transitionBarriers) {
				const vk::HostImageLayoutTransitionInfoEXT transitionInfo(
						image.m_handle,
						barrier.oldLayout,
						barrier.newLayout,
						barrier.subresourceRange
				);

				const auto result = core.getContext().getDevice().transitionImageLayoutEXT(
						1, 
						&transitionInfo,
						dynamicDispatch
				);

				if (vk::Result::eSuccess != result) {
					vkcv_log(LogLevel::WARNING, "Transition to new layout failed");
					break;
				}
			}
		} else {
			auto stream = core.createCommandStream(QueueType::Graphics);
		
			core.recordCommandsToStream(
					stream,
					[transitionBarriers](const vk::CommandBuffer &commandBuffer) {
						// TODO: precise PipelineStageFlagBits, will require a lot of context
						for (const auto& barrier : transitionBarriers) {
							commandBuffer.pipelineBarrier(
									vk::PipelineStageFlagBits::eTopOfPipe,
									vk::PipelineStageFlagBits::eBottomOfPipe,
									{},
									nullptr,
									nullptr,
									barrier
							);
						}
					},
					nullptr
			);
			
			core.submitCommandStream(stream, false);
		}
		
		for (const auto& barrier : transitionBarriers) {
			for (uint32_t i = 0; i < barrier.subresourceRange.layerCount; i++) {
				for (uint32_t j = 0; j < barrier.subresourceRange.levelCount; j++) {
					image.m_layers[barrier.subresourceRange.baseArrayLayer + i].m_layouts[barrier.subresourceRange.baseMipLevel + j] = newLayout;
				}
			}
		}
	}
	
	void ImageManager::recordImageLayoutTransition(const ImageHandle &handle,
												   uint32_t mipLevelCount, uint32_t mipLevelOffset,
												   vk::ImageLayout newLayout,
												   vk::CommandBuffer cmdBuffer) {
		auto &image = (*this) [handle];
		
		const auto transitionBarriers = createImageLayoutTransitionBarriers(
				image,
				mipLevelCount,
				mipLevelOffset,
				newLayout,
				false
		);
		
		for (const auto& barrier : transitionBarriers) {
			cmdBuffer.pipelineBarrier(
					vk::PipelineStageFlagBits::eAllCommands,
					vk::PipelineStageFlagBits::eAllCommands,
					{},
					nullptr,
					nullptr,
					barrier
			);

			for (uint32_t i = 0; i < barrier.subresourceRange.layerCount; i++) {
				for (uint32_t j = 0; j < barrier.subresourceRange.levelCount; j++) {
					image.m_layers[barrier.subresourceRange.baseArrayLayer + i].m_layouts[barrier.subresourceRange.baseMipLevel + j] = newLayout;
				}
			}
		}
	}
	
	void ImageManager::recordImageMemoryBarrier(const ImageHandle &handle,
												vk::CommandBuffer cmdBuffer) {
		auto &image = (*this) [handle];

		const auto transitionBarriers = createImageLayoutTransitionBarriers(
				image,
				0,
				0,
				vk::ImageLayout::eUndefined,
				true
		);
		
		for (const auto& barrier : transitionBarriers) {
			cmdBuffer.pipelineBarrier(
					vk::PipelineStageFlagBits::eAllCommands,
					vk::PipelineStageFlagBits::eAllCommands,
					{},
					nullptr,
					nullptr,
					barrier
			);
		}
	}
	
	constexpr uint32_t getBytesPerPixel(vk::Format format) {
		switch (format) {
			case vk::Format::eR8Unorm:
				return 1;
			case vk::Format::eR16Unorm:
				return 2;
			case vk::Format::eR32Uint:
			case vk::Format::eR8G8B8A8Srgb:
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

	static void fillImageViaCommandBuffer(Core& core, 
										  ImageManager& imageManager, 
										  BufferManager& bufferManager, 
										  const ImageEntry& image,
										  const ImageHandle& handle,
										  const void* data, 
										  size_t size,
										  uint32_t baseArrayLayer,
										  uint32_t arrayLayerCount) {
		imageManager.switchImageLayoutImmediate(handle, vk::ImageLayout::eTransferDstOptimal);
		
		BufferHandle bufferHandle = bufferManager.createBuffer(
				TypeGuard(1), BufferType::STAGING, BufferMemoryType::DEVICE_LOCAL, size, false
		);
		
		bufferManager.fillBuffer(bufferHandle, data, size, 0);
		
		vk::Buffer stagingBuffer = bufferManager.getBuffer(bufferHandle);
		
		auto stream = core.createCommandStream(QueueType::Transfer);
		
		core.recordCommandsToStream(
				stream,
				[&image, &stagingBuffer, &baseArrayLayer, &arrayLayerCount]
						(const vk::CommandBuffer &commandBuffer) {
					vk::ImageAspectFlags aspectFlags;
					
					if (isDepthImageFormat(image.m_format)) {
						aspectFlags = vk::ImageAspectFlagBits::eDepth;
					} else {
						aspectFlags = vk::ImageAspectFlagBits::eColor;
					}

					const vk::BufferImageCopy2 region2(
							0,
							0,
							0,
							vk::ImageSubresourceLayers(aspectFlags, 0, baseArrayLayer, arrayLayerCount),
							vk::Offset3D(0, 0, 0),
							vk::Extent3D(image.m_width, image.m_height, image.m_depth)
					);
					
					const vk::CopyBufferToImageInfo2 copyInfo(
							stagingBuffer,
							image.m_handle,
							vk::ImageLayout::eTransferDstOptimal,
							1,
							&region2
					);

					commandBuffer.copyBufferToImage2(&copyInfo);
				},
				[&imageManager, &handle]() {
					imageManager.switchImageLayoutImmediate(handle, vk::ImageLayout::eShaderReadOnlyOptimal);
				}
		);
		
		core.submitCommandStream(stream, false);
	}

	static void fillImageFromHost(Core& core, 
								  ImageManager& imageManager, 
								  const ImageEntry& image,
								  const ImageHandle& handle,
								  const void* data, 
								  uint32_t baseArrayLayer, 
								  uint32_t arrayLayerCount) {
		imageManager.switchImageLayoutImmediate(handle, vk::ImageLayout::eTransferDstOptimal);

		const auto &dynamicDispatch = core.getContext().getDispatchLoaderDynamic();
		
		vk::ImageAspectFlags aspectFlags;

		if (isDepthImageFormat(image.m_format)) {
			aspectFlags = vk::ImageAspectFlagBits::eDepth;
		} else {
			aspectFlags = vk::ImageAspectFlagBits::eColor;
		}

		const vk::MemoryToImageCopyEXT region(
				data,
				0,
				0,
				vk::ImageSubresourceLayers(aspectFlags, 0, baseArrayLayer, arrayLayerCount),
				vk::Offset3D(0, 0, 0),
				vk::Extent3D(image.m_width, image.m_height, image.m_depth)
		);

		const vk::CopyMemoryToImageInfoEXT copyInfo(
				vk::HostImageCopyFlagsEXT(),
				image.m_handle,
				vk::ImageLayout::eTransferDstOptimal,
				1,
				&region
		);

		core.getContext().getDevice().copyMemoryToImageEXT(copyInfo, dynamicDispatch);

		imageManager.switchImageLayoutImmediate(handle, vk::ImageLayout::eShaderReadOnlyOptimal);
	}
	
	void ImageManager::fillImage(const ImageHandle &handle,
								 const void* data,
								 size_t size,
								 uint32_t firstLayer,
								 uint32_t layerCount) {
		if (handle.isSwapchainImage()) {
			vkcv_log(LogLevel::ERROR, "Swapchain image cannot be filled");
			return;
		}
		
		auto &image = (*this) [handle];
		
		const auto imageLayerCount = static_cast<uint32_t>(image.m_layers.size());
		const uint32_t baseArrayLayer = std::min<uint32_t>(firstLayer, imageLayerCount);
		
		if (baseArrayLayer >= image.m_layers.size()) {
			return;
		}
		
		uint32_t arrayLayerCount;
		
		if (layerCount > 0) {
			arrayLayerCount = std::min<uint32_t>(layerCount, imageLayerCount - baseArrayLayer);
		} else {
			arrayLayerCount = imageLayerCount - baseArrayLayer;
		}
		
		const size_t image_size = (
				image.m_width * image.m_height * image.m_depth * getBytesPerPixel(image.m_format)
		);
		
		const size_t max_size = std::min(size, image_size);

		auto& core = getCore();

		if (image.m_accessible) {
			fillImageFromHost(
				core,
				(*this), 
				image, 
				handle, 
				data, 
				baseArrayLayer, 
				arrayLayerCount
			);
		} else {
			fillImageViaCommandBuffer(
				core, 
				(*this), 
				getBufferManager(), 
				image, 
				handle, 
				data, 
				max_size, 
				baseArrayLayer, 
				arrayLayerCount
			);
		}
	}
	
	void ImageManager::recordImageMipChainGenerationToCmdStream(
			const vkcv::CommandStreamHandle &cmdStream, const ImageHandle &handle) {
		const auto record = [this, handle](const vk::CommandBuffer cmdBuffer) {
			recordImageMipGenerationToCmdBuffer(cmdBuffer, handle);
		};
		
		getCore().recordCommandsToStream(cmdStream, record, nullptr);
	}
	
	void ImageManager::recordMSAAResolve(vk::CommandBuffer cmdBuffer, const ImageHandle &src,
										 const ImageHandle &dst) {
		auto &srcImage = (*this) [src];
		auto &dstImage = (*this) [dst];
		
		const auto srcLayerCount = static_cast<uint32_t>(srcImage.m_layers.size());
		const auto dstLayerCount = static_cast<uint32_t>(dstImage.m_layers.size());
		
		vk::ImageResolve region(
				vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, srcLayerCount),
				vk::Offset3D(0, 0, 0),
				vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, dstLayerCount),
				vk::Offset3D(0, 0, 0),
				vk::Extent3D(dstImage.m_width, dstImage.m_height, dstImage.m_depth)
		);
		
		recordImageLayoutTransition(src, 0, 0, vk::ImageLayout::eTransferSrcOptimal, cmdBuffer);
		recordImageLayoutTransition(dst, 0, 0, vk::ImageLayout::eTransferDstOptimal, cmdBuffer);
		
		cmdBuffer.resolveImage(
				srcImage.m_handle,
				srcImage.m_layers[0].m_layouts[0],
				dstImage.m_handle,
				dstImage.m_layers[0].m_layouts[0],
				region
		);
	}
	
	uint32_t ImageManager::getImageWidth(const ImageHandle &handle) const {
		auto &image = (*this) [handle];
		return image.m_width;
	}
	
	uint32_t ImageManager::getImageHeight(const ImageHandle &handle) const {
		auto &image = (*this) [handle];
		return image.m_height;
	}
	
	uint32_t ImageManager::getImageDepth(const ImageHandle &handle) const {
		auto &image = (*this) [handle];
		return image.m_depth;
	}
	
	vk::Format ImageManager::getImageFormat(const ImageHandle &handle) const {
		auto &image = (*this) [handle];
		return image.m_format;
	}
	
	bool ImageManager::isImageSupportingStorage(const ImageHandle &handle) const {
		if (handle.isSwapchainImage()) {
			return false;
		}
		
		auto &image = (*this) [handle];
		return image.m_storage;
	}
	
	uint32_t ImageManager::getImageMipCount(const ImageHandle &handle) const {
		if (handle.isSwapchainImage()) {
			return 1;
		}
		
		auto &image = (*this) [handle];
		return image.m_viewPerMip.size();
	}
	
	uint32_t ImageManager::getImageArrayLayers(const ImageHandle &handle) const {
		auto &image = (*this) [handle];
		return static_cast<uint32_t>(image.m_layers.size());
	}
	
	void ImageManager::setCurrentSwapchainImageIndex(int index) {
		m_currentSwapchainInputImage = index;
	}
	
	void ImageManager::setSwapchainImages(const Vector<vk::Image> &images,
										  const Vector<vk::ImageView> &views, uint32_t width,
										  uint32_t height, vk::Format format) {
		
		// destroy old views
		for (const auto &image : m_swapchainImages) {
			for (const auto &view : image.m_viewPerMip) {
				getCore().getContext().getDevice().destroyImageView(view);
			}
		}
		
		assert(images.size() == views.size());
		m_swapchainImages.clear();
		for (size_t i = 0; i < images.size(); i++) {
			const ImageLayer layer = { { vk::ImageLayout::eUndefined } };
			m_swapchainImages.push_back({
					images [i],
					nullptr,
					{ views [i] },
					{},
					width,
					height,
					1,
					format,
					{ layer },
					false,
					false
			});
		}
	}
	
	void ImageManager::updateImageLayoutManual(const vkcv::ImageHandle &handle,
											   vk::ImageLayout layout) {
		auto &image = (*this) [handle];
		for (auto& layer : image.m_layers) {
			for (auto& level : layer.m_layouts) {
				level = layout;
			}
		}
	}
	
} // namespace vkcv