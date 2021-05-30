#pragma once

namespace vkcv {

	enum class SamplerFilterType {
		NEAREST = 1,
		LINEAR = 2
	};
	
	enum class SamplerMipmapMode {
		NEAREST = 1,
		LINEAR = 2
	};
	
	enum class SamplerAddressMode {
		REPEAT = 1,
		MIRRORED_REPEAT = 2,
		CLAMP_TO_EDGE = 3,
		MIRROR_CLAMP_TO_EDGE = 4
	};

}
