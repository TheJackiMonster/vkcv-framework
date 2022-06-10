
#include "vkcv/BlitDownsampler.hpp"

#include "vkcv/Core.hpp"
#include "ImageManager.hpp"

namespace vkcv {
	
	BlitDownsampler::BlitDownsampler(Core &core,
									 ImageManager& imageManager) :
		Downsampler(core),
		m_imageManager(imageManager) {}
	
	void BlitDownsampler::recordDownsampling(const CommandStreamHandle &cmdStream,
											 const ImageHandle &image) const {
		m_imageManager.recordImageMipChainGenerationToCmdStream(cmdStream, image);
		m_core.prepareImageForSampling(cmdStream, image);
	}
	
}
