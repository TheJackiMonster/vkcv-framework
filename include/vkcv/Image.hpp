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
		static Image create(ImageManager* manager);
	private:
		ImageManager* const m_manager;
		const uint64_t m_handle_id;

		Image(ImageManager* manager, uint64_t id);
	};
}
