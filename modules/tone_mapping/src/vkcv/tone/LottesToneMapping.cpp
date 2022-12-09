
#include "vkcv/tone/LottesToneMapping.hpp"

#include "lottes.glsl.hxx"

namespace vkcv::tone {
	
	void LottesToneMapping::initToneMapping() {
		buildComputePipeline("lottes", LOTTES_GLSL_SHADER);
	}
	
	LottesToneMapping::LottesToneMapping(Core &core) : ToneMapping(core, "Lottes Tone Mapping") {
		initToneMapping();
	}

}
