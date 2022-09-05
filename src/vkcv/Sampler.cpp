
#include "vkcv/Sampler.hpp"

namespace vkcv {
	
	SamplerHandle samplerLinear(Core &core, bool clampToEdge) {
		return core.createSampler(
				vkcv::SamplerFilterType::LINEAR,
				vkcv::SamplerFilterType::LINEAR,
				vkcv::SamplerMipmapMode::LINEAR,
				clampToEdge?
					vkcv::SamplerAddressMode::CLAMP_TO_EDGE :
					vkcv::SamplerAddressMode::REPEAT
		);
	}
	
	SamplerHandle samplerNearest(Core &core, bool clampToEdge) {
		return core.createSampler(
				vkcv::SamplerFilterType::NEAREST,
				vkcv::SamplerFilterType::NEAREST,
				vkcv::SamplerMipmapMode::NEAREST,
				clampToEdge?
				vkcv::SamplerAddressMode::CLAMP_TO_EDGE :
				vkcv::SamplerAddressMode::REPEAT
		);
	}
	
}
