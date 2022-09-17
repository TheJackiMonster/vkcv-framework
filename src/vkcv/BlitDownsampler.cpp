
#include "vkcv/BlitDownsampler.hpp"

#include "ImageManager.hpp"
#include "vkcv/Core.hpp"

namespace vkcv {

	BlitDownsampler::BlitDownsampler(Core &core, ImageManager &imageManager) :
		Downsampler(core), m_imageManager(imageManager) {}

	void BlitDownsampler::recordDownsampling(const CommandStreamHandle &cmdStream,
											 const ImageHandle &image) {
		m_imageManager.recordImageMipChainGenerationToCmdStream(cmdStream, image);
		m_core.prepareImageForSampling(cmdStream, image);
	}

} // namespace vkcv
