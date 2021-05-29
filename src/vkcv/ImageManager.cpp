/**
 * @authors Lars Hoerttrich
 * @file vkcv/ImageManager.cpp
 * @brief class creating and managing images
 */
#include "vkcv/ImageManager.hpp"
#include "vkcv/Core.hpp"

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
	
	ImageManager::ImageManager() noexcept :
		m_core(nullptr), m_images()
	{
	}

	ImageManager::~ImageManager() noexcept {
		for (size_t id = 0; id < m_images.size(); id++) {
			destroyImage(id);
		}
	}

	void ImageManager::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
	{
		vk::BufferImageCopy copyRegion(0,width,height); // possibly add offset etc

		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Transfer; //not sure
		m_core->submitCommands(
			submitInfo,
			[buffer, image,copyRegion](const vk::CommandBuffer& commandBuffer) {
				commandBuffer.copyBufferToImage(
					buffer,
					image,vk::ImageLayout::eTransferDstOptimal,
					copyRegion
				);
			},
			[]() {}
			);

	}


	uint64_t ImageManager::createImage(uint32_t width, uint32_t height)
	{
		vk::ImageCreateFlags createFlags;
		vk::ImageUsageFlags imageUsageFlags = (vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
		vk::Format format = vk::Format::eR8G8B8A8Unorm; // als Argument variabel


		const vk::Device& device = m_core->getContext().getDevice();

		vk::ImageCreateInfo imageCreateInfo(
			createFlags,
			vk::ImageType::e2D,
			format, vk::Extent3D(width, height, 1),
			1,
			1,
			vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal,
			imageUsageFlags,
			vk::SharingMode::eExclusive,
			{},
			vk::ImageLayout::eUndefined
		);

		vk::Image image = device.createImage(imageCreateInfo);
		
		const vk::MemoryRequirements requirements = device.getImageMemoryRequirements(image);
		const vk::PhysicalDevice& physicalDevice = m_core->getContext().getPhysicalDevice();

		vk::MemoryPropertyFlags memoryTypeFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		bool mappable = false;

		const uint32_t memoryTypeIndex = searchImageMemoryType(
			physicalDevice.getMemoryProperties(),
			requirements.memoryTypeBits,
			memoryTypeFlags
		);

		vk::DeviceMemory memory = device.allocateMemory(vk::MemoryAllocateInfo(requirements.size, memoryTypeIndex));
		device.bindImageMemory(image, memory, 0);

		const uint64_t id = m_images.size();
		m_images.push_back({ image, memory, nullptr, mappable });
		return id;
	}

	void ImageManager::switchImageLayout(uint64_t id, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
		//alternativly we could use switch case for every variable to set
		vk::AccessFlags sourceAccessMask;
		vk::PipelineStageFlags sourceStage;
		vk::AccessFlags destinationAccessMask;
		vk::PipelineStageFlags destinationStage;
		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
			destinationAccessMask = vk::AccessFlagBits::eTransferWrite;
			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			sourceAccessMask = vk::AccessFlagBits::eTransferWrite;
			destinationAccessMask = vk::AccessFlagBits::eShaderRead;
			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		vk::ImageSubresourceRange imageSubresourceRange(vk::ImageAspectFlagBits::eColor , 0, 1, 0, 1);
		ImageManager::Image image = m_images[id];
		vk::ImageMemoryBarrier imageMemoryBarrier(
			sourceAccessMask,
			destinationAccessMask,
			oldLayout,
			newLayout,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			image.m_handle,
			imageSubresourceRange
			);
		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Present; //not sure
		m_core->submitCommands(
			submitInfo,
			[sourceStage, destinationStage, imageMemoryBarrier](const vk::CommandBuffer& commandBuffer) {
				commandBuffer.pipelineBarrier(
					sourceStage,
					destinationStage,
					{},
					nullptr,
					nullptr,
					imageMemoryBarrier
				);
			},
			[]() {}
		);
	}

	void ImageManager::destroyImage(uint64_t id)
	{
		if (id >= m_images.size()) {
			return;
		}
		auto& image = m_images[id];

		const vk::Device& device = m_core->getContext().getDevice();

		if (image.m_memory) {
			device.freeMemory(image.m_memory);
		}

		if (image.m_handle) {
			device.destroyImage(image.m_handle);
		}
	}


}