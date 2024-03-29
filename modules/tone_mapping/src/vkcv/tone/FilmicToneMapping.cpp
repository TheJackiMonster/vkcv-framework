
#include "vkcv/tone/FilmicToneMapping.hpp"

#include "filmic.glsl.hxx"

namespace vkcv::tone {
	
	void FilmicToneMapping::initToneMapping() {
		buildComputePipeline("filmic", FILMIC_GLSL_SHADER);
	}
	
	FilmicToneMapping::FilmicToneMapping(Core &core, bool normalize)
	: ToneMapping(core, "Filmic Tone Mapping", normalize) {
		initToneMapping();
	}

}
