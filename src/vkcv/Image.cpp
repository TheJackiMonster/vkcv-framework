/**
 * @authors Lars Hoerttrich
 * @file vkcv/Image.cpp
 * @brief class for image handles
 */
#include "vkcv/Image.hpp"
namespace vkcv{
	Image Image::create(ImageManager* manager)
	{
		return Image(manager, manager->createImage());
	}

	Image::Image(ImageManager* manager, uint64_t id) :
		m_manager(manager),
		m_handle_id(id)
	{
	}

}
