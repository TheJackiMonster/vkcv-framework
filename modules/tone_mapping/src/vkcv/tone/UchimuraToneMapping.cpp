
#include "vkcv/tone/UchimuraToneMapping.hpp"

#include "uchimura.glsl.hxx"

namespace vkcv::tone {
	
	void UchimuraToneMapping::initToneMapping() {
		buildComputePipeline("uchimura", UCHIMURA_GLSL_SHADER);
	}
	
	UchimuraToneMapping::UchimuraToneMapping(Core &core, bool normalize)
	: ToneMapping(core, "Uchimura Tone Mapping", normalize) {
		initToneMapping();
	}

}
