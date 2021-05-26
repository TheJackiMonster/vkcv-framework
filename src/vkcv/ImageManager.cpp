/**
 * @authors Lars Hoerttrich
 * @file vkcv/ImageManager.cpp
 * @brief class creating and managing images
 */
#include "vkcv/ImageManager.hpp"
#include "vkcv/Core.hpp"

namespace vkcv {
	
	ImageManager::ImageManager() noexcept :
		m_core(nullptr), m_images()
	{
	}

	ImageManager::~ImageManager() noexcept {
		for (size_t id = 0; id < m_images.size(); id++) {
			destroyImage(id);
		}
	}

	uint64_t ImageManager::createImage()
	{
		
		return uint64_t();
	}

	void ImageManager::destroyImage(uint64_t id)
	{
		if (id >= m_images.size()) {
			return;
		}
		auto& image = m_images[id];

		const vk::Device& device = m_core->getContext().getDevice();

		if (image.m_memory) {
			device.freeMemory(image.m_memory);
		}

		if (image.m_handle) {
			device.destroyImage(image.m_handle);
		}
	}


}