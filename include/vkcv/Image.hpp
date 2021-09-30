#pragma once
/**
 * @authors Lars Hoerttrich
 * @file vkcv/Buffer.hpp
 * @brief class for image handles
 */
#include "vulkan/vulkan.hpp"

#include "Handles.hpp"
#include "vkcv/ImageConfig.hpp"

namespace vkcv {
	
	class ImageManager;

	bool isDepthFormat(const vk::Format format);

	class Image {
		friend class Core;
	public:
		
		/**
		 * @return Vulkan format of the image
		 */
		[[nodiscard]]
		vk::Format getFormat() const;
		
		/**
		 * @return Width of the image
		 */
		[[nodiscard]]
		uint32_t getWidth() const;
		
		/**
		 * @return Height of the image
		 */
		[[nodiscard]]
		uint32_t getHeight() const;
		
		/**
		 * @return Depth of the image
		 */
		[[nodiscard]]
		uint32_t getDepth() const;

		/**
		 * @return Handle of the image to be used with the #Core
		 */
		[[nodiscard]]
		const vkcv::ImageHandle& getHandle() const;

		/**
		 * @return Number of mip levels of the image
		 */
		[[nodiscard]]
		uint32_t getMipCount() const;

		/**
		 * @brief Switch the image layout, returns after operation is finished
		 * 
		 * @param newLayout Layout that image is switched to
		 */
		void switchLayout(vk::ImageLayout newLayout);
		
		/**
		 * @brief Fill the image with data
		 * 
		 * @param data Pointer to the source data
		 * @param size Lower limit of the data size to copy in bytes, 
		 * the actual number of copied bytes is min(size, imageDataSize)
		 */
		void fill(const void* data, size_t size = SIZE_MAX);

		/**
		 * @brief Generates the entire mip chain from mip 0, returns after operation is finished
		 */
		void generateMipChainImmediate();

		/**
		 * @brief Record mip chain generation to command stream, mip 0 is used as source
		 * 
		 * @param cmdStream Command stream that the commands are recorded into
		 */
		void recordMipChainGeneration(const vkcv::CommandStreamHandle& cmdStream);
	private:
	    // TODO: const qualifier removed, very hacky!!!
	    //  Else you cannot recreate an image. Pls fix.
		ImageManager*       m_manager;
		ImageHandle   		m_handle;

		Image(ImageManager* manager, const ImageHandle& handle);
		
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
