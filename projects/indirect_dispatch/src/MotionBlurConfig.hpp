#pragma once
#include "vulkan/vulkan.hpp"

namespace MotionBlurConfig {
	const vk::Format    motionVectorTileFormat  = vk::Format::eR16G16Sfloat;
	const vk::Format    outputColorFormat       = vk::Format::eB10G11R11UfloatPack32;
	const uint32_t      maxMotionTileSize       = 24;	// must match "motionTileSize" in motionBlurConfig.inc
	const uint32_t      maxWidth                = 3840;
	const uint32_t      maxHeight               = 2160;
}