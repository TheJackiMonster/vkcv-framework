/**
 * @authors Lars Hoerttrich
 * @file vkcv/Image.cpp
 * @brief class for image handles
 */
#include "vkcv/Image.hpp"
#include "vkcv/ImageManager.hpp"

namespace vkcv{
	Image Image::create(ImageManager* manager, uint32_t width, uint32_t height)
	{
		return Image(manager, manager->createImage(width, height));
	}

	void Image::switchImageLayout(vk::ImageLayout newLayout)
	{
		m_manager->switchImageLayout(m_handle_id, m_layout, newLayout);
		m_layout = newLayout;
	}
	Image::Image(ImageManager* manager, uint64_t id) :
		m_manager(manager),
		m_handle_id(id),
		m_layout(vk::ImageLayout::eUndefined)
	{
	}

}
