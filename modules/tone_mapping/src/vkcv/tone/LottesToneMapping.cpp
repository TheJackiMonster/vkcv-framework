
#include "vkcv/tone/LottesToneMapping.hpp"

#include "lottes.glsl.hxx"

namespace vkcv::tone {
	
	void LottesToneMapping::initToneMapping() {
		buildComputePipeline("lottes", LOTTES_GLSL_SHADER);
	}
	
	LottesToneMapping::LottesToneMapping(Core &core, bool normalize)
	: ToneMapping(core, "Lottes Tone Mapping", normalize) {
		initToneMapping();
	}

}
