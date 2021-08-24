
#include "vkcv/upscaling/BilinearUpscaling.hpp"

namespace vkcv::upscaling {
	
	BilinearUpscaling::BilinearUpscaling(Core &core) : Upscaling(core) {}
	
	void BilinearUpscaling::recordUpscaling(const CommandStreamHandle &cmdStream, const ImageHandle &input,
											const ImageHandle &output) {
		m_core.recordBlitImage(cmdStream, input, output, SamplerFilterType::LINEAR);
	}

}
