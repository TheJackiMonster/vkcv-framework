
#include "vkcv/tone/ACESToneMapping.hpp"

#include "aces.glsl.hxx"

namespace vkcv::tone {
	
	void ACESToneMapping::initToneMapping() {
		buildComputePipeline("aces", ACES_GLSL_SHADER);
	}
	
	ACESToneMapping::ACESToneMapping(Core &core, bool normalize)
	: ToneMapping(core, "ACES Tone Mapping", normalize) {
		initToneMapping();
	}
	
}
