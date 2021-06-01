/**
 * @authors Lars Hoerttrich
 * @file vkcv/Image.cpp
 * @brief class for image handles
 */
#include "vkcv/Image.hpp"
#include "ImageManager.hpp"

namespace vkcv{
	
	Image Image::create(ImageManager* manager, vk::Format format, uint32_t width, uint32_t height, uint32_t depth)
	{
		return Image(manager, manager->createImage(width, height, depth, format), format);
	}
	
	vk::Format Image::getFormat() const {
		return m_format;
	}
	
	uint32_t Image::getWidth() const {
		return m_manager->getImageWidth(m_handle);
	}
	
	uint32_t Image::getHeight() const {
		return m_manager->getImageHeight(m_handle);
	}
	
	uint32_t Image::getDepth() const {
		return m_manager->getImageDepth(m_handle);
	}
	
	vk::ImageLayout Image::getLayout() const {
		return m_layout;
	}

	void Image::switchLayout(vk::ImageLayout newLayout)
	{
		m_manager->switchImageLayout(m_handle, m_layout, newLayout);
		m_layout = newLayout;
	}

	vkcv::ImageHandle Image::getHandle() const {
		return m_handle;
	}
	
	void Image::fill(void *data, size_t size) {
		m_manager->fillImage(m_handle, data, size);
	}
	
	Image::Image(ImageManager* manager, const ImageHandle& handle, vk::Format format) :
		m_manager(manager),
		m_handle(handle),
		m_format(format),
		m_layout(vk::ImageLayout::eUndefined)
	{
	}

}
