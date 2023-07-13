#pragma once
/**
 * @authors Lars Hoerttrich
 * @file vkcv/ImageManager.hpp
 * @brief class creating and managing images
 */

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "BufferManager.hpp"
#include "HandleManager.hpp"

#include "vkcv/Container.hpp"
#include "vkcv/ImageConfig.hpp"

namespace vkcv {

	/**
	 * @brief Determine whether an image format is valid
	 * for depth buffers.
	 *
	 * @param[in] format Image format
	 * @return True, if the format is usable for depth buffers, otherwise false.
	 */
	bool isDepthImageFormat(vk::Format format);

	struct ImageLayer {
		Vector<vk::ImageLayout> m_layouts;
	};

	struct ImageEntry {
		vk::Image m_handle;
		vma::Allocation m_allocation;

		Vector<vk::ImageView> m_viewPerMip;
		Vector<vk::ImageView> m_arrayViewPerMip;

		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_depth;

		vk::Format m_format;
		Vector<ImageLayer> m_layers;
		bool m_storage;
	};

	/**
	 * @brief Class to manage the creation, destruction, allocation
	 * and filling of images.
	 */
	class ImageManager : HandleManager<ImageEntry, ImageHandle> {
		friend class Core;

	private:
		BufferManager* m_bufferManager;

		Vector<ImageEntry> m_swapchainImages;
		int m_currentSwapchainInputImage;

		using HandleManager<ImageEntry, ImageHandle>::init;
		bool init(Core &core, BufferManager &bufferManager);

		[[nodiscard]] uint64_t getIdFrom(const ImageHandle &handle) const override;

		[[nodiscard]] ImageHandle createById(uint64_t id,
											 const HandleDestroyFunction &destroy) override;

		/**
		 * Destroys and deallocates image represented by a given
		 * image handle id.
		 *
		 * @param id Image handle id
		 */
		void destroyById(uint64_t id) override;

		[[nodiscard]] const BufferManager &getBufferManager() const;

		[[nodiscard]] BufferManager &getBufferManager();

		void recordImageMipGenerationToCmdBuffer(vk::CommandBuffer cmdBuffer,
												 const ImageHandle &handle);

	protected:
		[[nodiscard]] virtual const ImageEntry &
		operator[](const ImageHandle &handle) const override;

		[[nodiscard]] virtual ImageEntry &operator[](const ImageHandle &handle) override;

	public:
		ImageManager() noexcept;

		~ImageManager() noexcept override;

		[[nodiscard]] ImageHandle createImage(vk::Format format,
											  uint32_t mipCount,
											  const ImageConfig& config);

		[[nodiscard]] vk::Image getVulkanImage(const ImageHandle &handle) const;

		[[nodiscard]] vk::DeviceMemory getVulkanDeviceMemory(const ImageHandle &handle) const;

		[[nodiscard]] vk::ImageView getVulkanImageView(const ImageHandle &handle,
													   size_t mipLevel = 0,
													   bool arrayView = false) const;

		void switchImageLayoutImmediate(const ImageHandle &handle, vk::ImageLayout newLayout);

		void recordImageLayoutTransition(const ImageHandle &handle, uint32_t mipLevelCount,
										 uint32_t mipLevelOffset, vk::ImageLayout newLayout,
										 vk::CommandBuffer cmdBuffer);

		void recordImageMemoryBarrier(const ImageHandle &handle, vk::CommandBuffer cmdBuffer);

		void fillImage(const ImageHandle &handle,
					   const void* data,
					   size_t size,
					   uint32_t firstLayer,
					   uint32_t layerCount);

		void recordImageMipChainGenerationToCmdStream(const vkcv::CommandStreamHandle &cmdStream,
													  const ImageHandle &handle);

		void recordMSAAResolve(vk::CommandBuffer cmdBuffer, const ImageHandle &src,
							   const ImageHandle &dst);

		[[nodiscard]] uint32_t getImageWidth(const ImageHandle &handle) const;

		[[nodiscard]] uint32_t getImageHeight(const ImageHandle &handle) const;

		[[nodiscard]] uint32_t getImageDepth(const ImageHandle &handle) const;

		[[nodiscard]] vk::Format getImageFormat(const ImageHandle &handle) const;

		[[nodiscard]] bool isImageSupportingStorage(const ImageHandle &handle) const;

		[[nodiscard]] uint32_t getImageMipCount(const ImageHandle &handle) const;

		[[nodiscard]] uint32_t getImageArrayLayers(const ImageHandle &handle) const;

		void setCurrentSwapchainImageIndex(int index);

		void setSwapchainImages(const Vector<vk::Image> &images,
								const Vector<vk::ImageView> &views, uint32_t width,
								uint32_t height, vk::Format format);

		// if manual vulkan work, e.g. ImGui integration, changes an image layout this function must
		// be used to update the internal image state
		void updateImageLayoutManual(const vkcv::ImageHandle &handle, vk::ImageLayout layout);
	};
} // namespace vkcv