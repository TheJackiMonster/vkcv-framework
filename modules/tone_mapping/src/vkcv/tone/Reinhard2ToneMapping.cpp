
#include "vkcv/tone/Reinhard2ToneMapping.hpp"

#include "reinhard2.glsl.hxx"

namespace vkcv::tone {
	
	void Reinhard2ToneMapping::initToneMapping() {
		buildComputePipeline("reinhard2", REINHARD2_GLSL_SHADER);
	}
	
	Reinhard2ToneMapping::Reinhard2ToneMapping(Core &core) : ToneMapping(core, "Reinhard2 Tone Mapping") {
		initToneMapping();
	}

}
