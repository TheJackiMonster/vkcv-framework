
#include "vkcv/tone/Uncharted2ToneMapping.hpp"

#include "uncharted2.glsl.hxx"

namespace vkcv::tone {
	
	void Uncharted2ToneMapping::initToneMapping() {
		buildComputePipeline("uncharted2", UNCHARTED2_GLSL_SHADER);
	}
	
	Uncharted2ToneMapping::Uncharted2ToneMapping(Core &core) : ToneMapping(core, "Uncharted2 Tone Mapping") {
		initToneMapping();
	}

}
