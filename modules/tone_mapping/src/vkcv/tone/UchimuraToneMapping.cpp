
#include "vkcv/tone/UchimuraToneMapping.hpp"

#include "uchimura.glsl.hxx"

namespace vkcv::tone {
	
	void UchimuraToneMapping::initToneMapping() {
		buildComputePipeline("uchimura", UCHIMURA_GLSL_SHADER);
	}
	
	UchimuraToneMapping::UchimuraToneMapping(Core &core) : ToneMapping(core, "Uchimura Tone Mapping") {
		initToneMapping();
	}

}
