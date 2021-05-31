#pragma once
/**
 * @authors Lars Hoerttrich
 * @file vkcv/Buffer.hpp
 * @brief class for image handles
 */
#include "vulkan/vulkan.hpp"

#include "Handles.hpp"

namespace vkcv {
	
	class ImageManager;
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
		vk::ImageLayout getLayout() const;

		[[nodiscard]]
		vkcv::ImageHandle getHandle() const;
		
		void switchLayout(vk::ImageLayout newLayout);
		
		void fill(void* data, size_t size = SIZE_MAX);
	private:
		ImageManager* const m_manager;
		const ImageHandle m_handle;
		const vk::Format m_format;
		const uint32_t m_width;
		const uint32_t m_height;
		const uint32_t m_depth;
		vk::ImageLayout m_layout;

		Image(ImageManager* manager, const ImageHandle& handle, vk::Format format, uint32_t width, uint32_t height, uint32_t depth);
		
		static Image create(ImageManager* manager, vk::Format format, uint32_t width, uint32_t height, uint32_t depth);
		
	};
	
}
