#pragma once
/**
 * @authors Alexander Gauggel, Tobias Frisch, Lars Hoerttrich, Artur Wasmut
 * @file vkcv/Image.hpp
 * @brief Class for image handling.
 */
 
#include <vulkan/vulkan.hpp>

#include "BufferTypes.hpp"
#include "Core.hpp"
#include "Handles.hpp"
#include "Multisampling.hpp"

namespace vkcv {
	
	class Downsampler;
	class ImageManager;

	/**
	 * @brief Returns whether an image format is usable as depth buffer.
	 *
	 * @param format Vulkan image format
	 * @return True, if the format is valid to use as depth buffer,
	 * otherwise false.
	 */
	bool isDepthFormat(const vk::Format format);
	
	/**
	 * @brief Returns whether an image format is usable as stencil buffer.
	 *
	 * @param format Vulkan image format
	 * @return True, if the format is valid to use as stencil buffer,
	 * otherwise false.
	 */
	bool isStencilFormat(const vk::Format format);

    /**
     * @brief Class for image handling and filling data.
     */
	class Image {
		friend class Core;
	public:
		Image() : m_core(nullptr), m_handle() {};
		
		Image(Core* core, const ImageHandle& handle) : m_core(core), m_handle(handle) {}
		
		Image(const Image& other) = default;
		Image(Image&& other) = default;
		
		~Image() = default;
		
		Image& operator=(const Image& other) = default;
		Image& operator=(Image&& other) = default;
		
		/**
		 * @brief Returns the format of the image.
		 *
		 * @return Vulkan image format
		 */
		[[nodiscard]]
		vk::Format getFormat() const;
		
		/**
		 * @brief Returns the width of the image.
		 *
		 * @return Width of the image
		 */
		[[nodiscard]]
		uint32_t getWidth() const;
		
		/**
		 * @brief Returns the height of the image.
		 *
		 * @return Height of the image
		 */
		[[nodiscard]]
		uint32_t getHeight() const;
		
		/**
		 * @brief Returns the depth of the image.
		 *
		 * @return Depth of the image
		 */
		[[nodiscard]]
		uint32_t getDepth() const;

		/**
		 * @brief Returns the image handle of the image.
		 *
		 * @return Handle of the image
		 */
		[[nodiscard]]
		const vkcv::ImageHandle& getHandle() const;

		/**
		 * @brief Returns the amount of mip levels of the image.
		 *
		 * @return Number of mip levels
		 */
		[[nodiscard]]
		uint32_t getMipLevels() const;

		/**
		 * @brief Switches the image layout,
		 * returns after operation is finished.
		 * 
		 * @param[in] newLayout Layout that image is switched to
		 */
		void switchLayout(vk::ImageLayout newLayout);
		
		/**
		 * @brief Fills the image with data of a given size in bytes.
		 * 
		 * @param[in] data Pointer to the source data
		 * @param[in] size Lower limit of the data size to copy in bytes,
		 * the actual number of copied bytes is min(size, imageDataSize)
		 */
		void fill(const void* data, size_t size = SIZE_MAX);

		/**
		 * @brief Records mip chain generation to command stream,
		 * mip level zero is used as source
		 * 
		 * @param[out] cmdStream Command stream that the commands are recorded into
		 * @param[in,out] downsampler Downsampler to generate mip levels with
		 */
		void recordMipChainGeneration(const vkcv::CommandStreamHandle& cmdStream,
									  Downsampler &downsampler);
		
	private:
		Core* m_core;
		ImageHandle m_handle;

	};
	
	Image image(Core &core,
				vk::Format format,
				uint32_t width,
				uint32_t height,
				uint32_t depth=1,
				bool createMipChain=false,
				bool supportStorage=false,
				bool supportColorAttachment=false,
				Multisampling multisampling=Multisampling::None);
	
}
