/**
 * @authors Lars Hoerttrich
 * @file vkcv/Image.cpp
 * @brief class for image handles
 */
#include "vkcv/Image.hpp"

#include "ImageManager.hpp"
#include "vkcv/Downsampler.hpp"

namespace vkcv {

	bool isDepthFormat(const vk::Format format) {
		switch (format) {
		case (vk::Format::eD16Unorm):
		case (vk::Format::eD16UnormS8Uint):
		case (vk::Format::eD24UnormS8Uint):
		case (vk::Format::eD32Sfloat):
		case (vk::Format::eD32SfloatS8Uint):
			return true;
		default:
			return false;
		}
	}

	bool isStencilFormat(const vk::Format format) {
		switch (format) {
		case (vk::Format::eS8Uint):
		case (vk::Format::eD16UnormS8Uint):
		case (vk::Format::eD24UnormS8Uint):
		case (vk::Format::eD32SfloatS8Uint):
			return true;
		default:
			return false;
		}
	}

	vk::Format Image::getFormat() const {
		return m_core->getImageFormat(m_handle);
	}

	uint32_t Image::getWidth() const {
		return m_core->getImageWidth(m_handle);
	}

	uint32_t Image::getHeight() const {
		return m_core->getImageHeight(m_handle);
	}

	uint32_t Image::getDepth() const {
		return m_core->getImageDepth(m_handle);
	}

	void Image::switchLayout(vk::ImageLayout newLayout) {
		m_core->switchImageLayout(m_handle, newLayout);
	}

	const vkcv::ImageHandle &Image::getHandle() const {
		return m_handle;
	}

	uint32_t Image::getMipLevels() const {
		return m_core->getImageMipLevels(m_handle);
	}

	void Image::fill(const void* data, size_t size) {
		m_core->fillImage(m_handle, data, size, 0, 0);
	}
	
	void Image::fillLayer(uint32_t layer, const void* data, size_t size) {
		m_core->fillImage(m_handle, data, size, layer, 1);
	}

	void Image::recordMipChainGeneration(const vkcv::CommandStreamHandle &cmdStream,
										 Downsampler &downsampler) {
		downsampler.recordDownsampling(cmdStream, m_handle);
	}

	Image image(Core &core, vk::Format format, uint32_t width, uint32_t height, uint32_t depth,
				bool createMipChain, bool supportStorage, bool supportColorAttachment,
				Multisampling multisampling) {
		ImageConfig config (width, height, depth);
		config.setSupportingStorage(supportStorage);
		config.setSupportingColorAttachment(supportColorAttachment);
		config.setMultisampling(multisampling);
		return image(core, format, config, createMipChain);
	}
	
	Image image(Core &core, vk::Format format, const ImageConfig &config, bool createMipChain) {
		return Image(
				&core,
				core.createImage(
						format,
						config,
						createMipChain
				)
		);
	}

} // namespace vkcv
