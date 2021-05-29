/**
 * @authors Lars Hoerttrich
 * @file vkcv/Image.cpp
 * @brief class for image handles
 */
#include "vkcv/Image.hpp"

namespace vkcv{
	Image Image::create(ImageManager* manager, uint32_t width, uint32_t height)
	{
		return Image(manager, manager->createImage(width, height));
	}

	void Image::switchImageLayout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
	{
		m_manager->switchImageLayout(m_handle_id, oldLayout, newLayout);
	}
	Image::Image(ImageManager* manager, uint64_t id) :
		m_manager(manager),
		m_handle_id(id)
	{
	}

}
