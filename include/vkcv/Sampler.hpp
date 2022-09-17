#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/Sampler.hpp
 * @brief Support functions for basic sampler creation.
 */

#include "Core.hpp"
#include "Handles.hpp"
#include "SamplerTypes.hpp"

namespace vkcv {

	[[nodiscard]] SamplerHandle samplerLinear(Core &core, bool clampToEdge = false);

	[[nodiscard]] SamplerHandle samplerNearest(Core &core, bool clampToEdge = false);

} // namespace vkcv
