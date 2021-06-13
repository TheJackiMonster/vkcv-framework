/**
 * @authors Lars Hoerttrich
 * @file vkcv/Image.cpp
 * @brief class for image handles
 */
#include "vkcv/Image.hpp"
#include "ImageManager.hpp"

namespace vkcv{
	
	bool isDepthFormat(const vk::Format format) {
		switch (format) {
			case(vk::Format::eD16Unorm):        return true;
			case(vk::Format::eD16UnormS8Uint):  return true;
			case(vk::Format::eD24UnormS8Uint):  return true;
			case(vk::Format::eD32Sfloat):       return true;
			case(vk::Format::eD32SfloatS8Uint): return true;
			default:                            return false;
		}
	}

	Image Image::create(ImageManager* manager, vk::Format format, uint32_t width, uint32_t height, uint32_t depth)
	{
		return Image(manager, manager->createImage(width, height, depth, format));
	}
	
	vk::Format Image::getFormat() const {
		return m_manager->getImageFormat(m_handle);
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

	void Image::switchLayout(vk::ImageLayout newLayout)
	{
		m_manager->switchImageLayoutImmediate(m_handle, newLayout);
	}

	vkcv::ImageHandle Image::getHandle() const {
		return m_handle;
	}
	
	void Image::fill(void *data, size_t size) {
		m_manager->fillImage(m_handle, data, size);
	}
	
	Image::Image(ImageManager* manager, const ImageHandle& handle) :
		m_manager(manager),
		m_handle(handle)
	{}

}
