
#include "vkcv/tone/ReinhardToneMapping.hpp"

#include "reinhard.glsl.hxx"

namespace vkcv::tone {
	
	void ReinhardToneMapping::initToneMapping() {
		buildComputePipeline("reinhard", REINHARD_GLSL_SHADER);
	}
	
	ReinhardToneMapping::ReinhardToneMapping(Core &core, bool normalize)
	: ToneMapping(core, "Reinhard Tone Mapping", normalize) {
		initToneMapping();
	}

}
