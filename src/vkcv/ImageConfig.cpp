/**
 * @authors Tobias Frisch
 * @file vkcv/Image.cpp
 * @brief Structure for image configuration
 */
#include "vkcv/ImageConfig.hpp"

namespace vkcv {
	
	ImageConfig::ImageConfig(uint32_t width,
							 uint32_t height,
							 uint32_t depth)
	: m_width(width),
	  m_height(height),
	  m_depth(depth),
	  
	  m_supportStorage(false),
	  m_supportColorAttachment(false),
	  m_cubeMapImage(false),
	  
	  m_msaa(Multisampling::None)
	{}
	
	uint32_t ImageConfig::getWidth() const {
		return m_width;
	}
	
	void ImageConfig::setWidth(uint32_t width) {
		m_width = width;
	}
	
	uint32_t ImageConfig::getHeight() const {
		return m_height;
	}
	
	void ImageConfig::setHeight(uint32_t height) {
		m_height = height;
	}
	
	uint32_t ImageConfig::getDepth() const {
		return m_depth;
	}
	
	void ImageConfig::setDepth(uint32_t depth) {
		m_depth = depth;
	}
	
	bool ImageConfig::isSupportingStorage() const {
		return m_supportStorage;
	}
	
	void ImageConfig::setSupportingStorage(bool supportStorage) {
		m_supportStorage = supportStorage;
	}
	
	bool ImageConfig::isSupportingColorAttachment() const {
		return m_supportColorAttachment;
	}
	
	void ImageConfig::setSupportingColorAttachment(bool supportColorAttachment) {
		m_supportColorAttachment = supportColorAttachment;
	}
	
	bool ImageConfig::isCubeMapImage() const {
		return m_cubeMapImage;
	}
	
	void ImageConfig::setCubeMapImage(bool cubeMapImage) {
		m_cubeMapImage = cubeMapImage;
	}
	
	Multisampling ImageConfig::getMultisampling() const {
		return m_msaa;
	}
	
	void ImageConfig::setMultisampling(Multisampling msaa) {
		m_msaa = msaa;
	}
	
}