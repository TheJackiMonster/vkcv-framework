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
	
	void ImageManager::init(BufferManager* bufferManager)
	{
		if (!m_core) {
			return;
		}
		uint64_t stagingID = bufferManager->createBuffer(BufferType::STAGING, 1024 * 1024, BufferMemoryType::HOST_VISIBLE);
		m_stagingBuffer = bufferManager->m_buffers[stagingID].m_handle;
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

		const uint32_t memoryTypeIndex = searchImageMemoryType(
			physicalDevice.getMemoryProperties(),
			requirements.memoryTypeBits,
			memoryTypeFlags
		);

		vk::DeviceMemory memory = device.allocateMemory(vk::MemoryAllocateInfo(requirements.size, memoryTypeIndex));
		device.bindImageMemory(image, memory, 0);

		const uint64_t id = m_images.size();
		m_images.push_back({ image, memory});
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
			nullptr
		);
	}

	struct ImageStagingStepInfo {
		void* data;
		size_t size;
		uint32_t width;
		uint32_t height;
		size_t offset;

		vk::Image image;
		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingMemory;

		size_t stagingLimit;
		size_t stagingPosition;
	};

	void copyStagingToImage(Core* core, ImageStagingStepInfo info)
	{
		/*
		* Alte implementation
		vk::BufferImageCopy copyRegion(0, width, height);

		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Transfer; //not sure
		core->submitCommands(
			submitInfo,
			[buffer, image, copyRegion](const vk::CommandBuffer& commandBuffer) {
				commandBuffer.copyBufferToImage(
					buffer,
					image, vk::ImageLayout::eTransferDstOptimal,
					copyRegion
				);
			},
			[]() {}
			);
		*/

		const size_t remaining = info.size - info.stagingPosition;
		const size_t mapped_size = std::min(remaining, info.stagingLimit);

		const vk::Device& device = core->getContext().getDevice();

		void* mapped = device.mapMemory(info.stagingMemory, 0, mapped_size);
		memcpy(mapped, reinterpret_cast<char*>(info.data) + info.stagingPosition, mapped_size);
		device.unmapMemory(info.stagingMemory);

		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Transfer;

		core->submitCommands(
			submitInfo,
			[&info, &mapped_size](const vk::CommandBuffer& commandBuffer) {
				/*
				const vk::BufferImageCopy region(
					info.offset, //bufferOffset
					info.size, //bufferRowlength
					0, //bufferImageHeight
					vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor,0,0,1),//soubresource layer
					vk::Offset2D(info.offset,0), //imageoffset
					vk::Extent3D(info.width,info.height,1) //extend3d
				);
				
				commandBuffer.copyBufferToImage(
					info.stagingBuffer,
					info.image,
					vk::ImageLayout::eTransferDstOptimal,
					region);
				*/
			},
			[&core, &info, &mapped_size, &remaining]() {
				if (mapped_size < remaining) {
					info.stagingPosition += mapped_size;

					copyStagingToImage(
						core,
						info
					);
				}
			}
			);
	}
	void ImageManager::fillImage(uint64_t id, void* data, size_t size)
	{
		uint64_t width =0, height = 0; // TODO
		//Image wird geladen	
		size_t sizeImage = width * height * 3; //TODO
		switchImageLayout(id, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		//const size_t max_size = std::min(size, image.m_size - offset);
		ImageStagingStepInfo info;
		info.data = data;
		info.size = size;//TODO
		info.offset = 0;

		info.image = m_images[id].m_handle;
		info.stagingBuffer = m_stagingBuffer;
		info.stagingMemory = m_stagingMemory;

		const vk::MemoryRequirements stagingRequirements = m_core->getContext().getDevice().getBufferMemoryRequirements(m_stagingBuffer);
		info.stagingLimit = stagingRequirements.size;
		info.stagingPosition = 0;

		copyStagingToImage(m_core, info);
		switchImageLayout(id, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
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