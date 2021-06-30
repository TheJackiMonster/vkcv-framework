#pragma once
/**
 * @authors Lars Hoerttrich
 * @file vkcv/Buffer.hpp
 * @brief class for image handles
 */
#include "vulkan/vulkan.hpp"

#include "Handles.hpp"

namespace vkcv {

    // forward declares
    class ImageManager;

	bool isDepthFormat(const vk::Format format);

	class Image {
		friend class Core;
	public:
		[[nodiscard]]
		vk::Format getFormat() const;
		
		[[nodiscard]]
		uint32_t getWidth() const;
		
		[[nodiscard]]
		uint32_t getHeight() const;
		
		[[nodiscard]]
		uint32_t getDepth() const;

		[[nodiscard]]
		const vkcv::ImageHandle& getHandle() const;

		[[nodiscard]]
		uint32_t getMipCount() const;

		void switchLayout(vk::ImageLayout newLayout);
		
		void fill(const void* data, size_t size = SIZE_MAX);
		void generateMipChainImmediate();
		void recordMipChainGeneration(const vkcv::CommandStreamHandle& cmdStream);
	private:
	    // TODO: const qualifier removed, very hacky!!!
	    //  Else you cannot recreate an image. Pls fix.
		ImageManager*       m_manager;
		ImageHandle   		m_handle;

		Image(ImageManager* manager, const ImageHandle& handle);
		
		static Image create(
			ImageManager* manager,
			vk::Format format,
			uint32_t width,
			uint32_t height,
			uint32_t depth,
			uint32_t mipCount,
			bool supportStorage,
			bool supportColorAttachment);
		
	};
	
}
