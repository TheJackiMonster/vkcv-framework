#pragma once
/**
 * @authors Lars Hoerttrich
 * @file vkcv/Buffer.hpp
 * @brief class for image handles
 */
#include "vulkan/vulkan.hpp"

namespace vkcv {
	
	class ImageManager;
	class Image {
	public:
		static Image create(ImageManager* manager, uint32_t width, uint32_t height);
		
		[[nodiscard]]
		uint32_t getWidth() const;
		
		[[nodiscard]]
		uint32_t getHeight() const;
		
		[[nodiscard]]
		vk::ImageLayout getLayout() const;
		
		void switchLayout(vk::ImageLayout newLayout);
	private:
		ImageManager* const m_manager;
		const uint64_t m_handle_id;
		const uint32_t m_width;
		const uint32_t m_height;
		vk::ImageLayout m_layout;

		Image(ImageManager* manager, uint64_t id, uint32_t width, uint32_t height);
	};
	
}
