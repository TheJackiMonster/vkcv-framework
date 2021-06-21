#pragma once

namespace vkcv {

	enum class ShaderStage : uint32_t
	{
		VERTEX = 0x00000001,
		TESS_CONTROL = 0x00000002,
		TESS_EVAL = 0x00000004,
		GEOMETRY = 0x00000008,
		FRAGMENT = 0x00000016,
		COMPUTE = 0x00000032
	};
}
