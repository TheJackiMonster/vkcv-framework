#pragma once

#include "Downsampler.hpp"

namespace vkcv {
	
	class ImageManager;
	
	class BlitDownsampler : public Downsampler {
		friend class Core;
	private:
		ImageManager& m_imageManager;
		
		BlitDownsampler(Core& core,
						ImageManager& imageManager);
	
	public:
		void recordDownsampling(const CommandStreamHandle& cmdStream,
								const ImageHandle& image) override;
	};
	
}
