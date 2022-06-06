#pragma once
/**
 * @authors Alexander Gauggel, Tobias Frisch, Lars Hoerttrich, Artur Wasmut
 * @file vkcv/Image.hpp
 * @brief Class for image handling.
 */
 
#include <vulkan/vulkan.hpp>

#include "Handles.hpp"
#include "ImageConfig.hpp"

namespace vkcv {
	
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
     * @brief Class for image handling and filling data.
     */
	class Image {
		friend class Core;
	public:
		
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
		uint32_t getMipCount() const;

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
		 * @brief Generates the entire mip chain from mip level zero,
		 * returns after operation is finished
		 */
		void generateMipChainImmediate();

		/**
		 * @brief Records mip chain generation to command stream,
		 * mip level zero is used as source
		 * 
		 * @param[out] cmdStream Command stream that the commands are recorded into
		 */
		void recordMipChainGeneration(const vkcv::CommandStreamHandle& cmdStream);
		
	private:
	    // TODO: const qualifier removed, very hacky!!!
	    //  Else you cannot recreate an image. Pls fix.
		ImageManager* m_manager;
		ImageHandle m_handle;

		Image(ImageManager* manager, const ImageHandle& handle);
		
		/**
		 * @brief Creates an image with given parameters like width, height,
		 * depth, amount of mip levels and others.
		 *
		 * @param[in,out] manager Image manager
		 * @param[in] format Vulkan image format
		 * @param[in] width Width of the image
		 * @param[in] height Height of the image
		 * @param[in] depth Depth of the image
		 * @param[in] mipCount Amount of mip levels
		 * @param[in] supportStorage Support of storage
		 * @param[in] supportColorAttachment Support of color attachment
		 * @param[in] msaa MSAA mode
		 * @return New created image
		 */
		static Image create(
			ImageManager*   manager,
			vk::Format      format,
			uint32_t        width,
			uint32_t        height,
			uint32_t        depth,
			uint32_t        mipCount,
			bool            supportStorage,
			bool            supportColorAttachment,
			Multisampling   msaa);

	};
	
}
