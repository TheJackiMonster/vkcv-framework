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
		vkcv::ImageHandle getHandle() const;
		
		void switchLayout(vk::ImageLayout newLayout);
		
		void fill(void* data, size_t size = SIZE_MAX);
		void generateMipChainImmediate();
	private:
		ImageManager* const m_manager;
		const ImageHandle   m_handle;

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
