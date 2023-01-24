
#include "vkcv/tone/UnrealToneMapping.hpp"

#include "unreal.glsl.hxx"

namespace vkcv::tone {
	
	void UnrealToneMapping::initToneMapping() {
		buildComputePipeline("unreal", UNREAL_GLSL_SHADER);
	}
	
	UnrealToneMapping::UnrealToneMapping(Core &core, bool normalize)
	: ToneMapping(core, "Unreal Tone Mapping", normalize) {
		initToneMapping();
	}

}
