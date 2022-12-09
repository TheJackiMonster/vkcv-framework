
#include "vkcv/tone/ACESToneMapping.hpp"

#include "aces.glsl.hxx"

namespace vkcv::tone {
	
	void ACESToneMapping::initToneMapping() {
		buildComputePipeline("aces", ACES_GLSL_SHADER);
	}
	
	ACESToneMapping::ACESToneMapping(Core &core) : ToneMapping(core, "ACES Tone Mapping") {
		initToneMapping();
	}
	
}
