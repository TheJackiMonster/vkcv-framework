
#include "vkcv/upscaling/BilinearUpscaling.hpp"

namespace vkcv::upscaling {
	
	BilinearUpscaling::BilinearUpscaling(Core &core) : Upscaling(core) {}
	
	void BilinearUpscaling::recordUpscaling(const CommandStreamHandle &cmdStream, const ImageHandle &input,
											const ImageHandle &output) {
		m_core.recordBeginDebugLabel(cmdStream, "vkcv::upscaling::BilinearUpscaling", {
			0.0f, 0.0f, 1.0f, 1.0f
		});
		
		m_core.recordBlitImage(cmdStream, input, output, SamplerFilterType::LINEAR);
		
		m_core.recordEndDebugLabel(cmdStream);
	}

}
