#pragma once
/**
 * @authors Lars Hoerttrich
 * @file vkcv/Buffer.hpp
 * @brief class for image handles
 */
#include "ImageManager.hpp"

namespace vkcv {
	class Image {
	public:
		static Image create(ImageManager* manager, uint32_t width, uint32_t height);
		void switchImageLayout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	private:
		ImageManager* const m_manager;
		const uint64_t m_handle_id;

		Image(ImageManager* manager, uint64_t id);
	};
}
